#include "LVGLTask.hpp"

#include "UIOutputTask.hpp" // DisplayMessage struct
#include "Test/Config.hpp"
#if LVGL_TEST_ANIMATION
#include "Test/UI/TestAnimation.hpp"
#endif

/**
 * @brief 清理一段内存的 D-Cache，保证 DMA 访问一致性。
 */
void LVGLTask::clean_dcache_for_range(const void* ptr, uint32_t len) {
  if (ptr == nullptr || len == 0)
    return;
  uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
  uintptr_t start = addr & ~static_cast<uintptr_t>(0x1F);
  uintptr_t end = (addr + len + 31) & ~static_cast<uintptr_t>(0x1F);
  SCB_CleanDCache_by_Addr(reinterpret_cast<uint32_t*>(start), static_cast<int32_t>(end - start));
}

/**
 * @brief 将外部创建的同步原语注入任务，必须在 Start 前调用。
 */
void LVGLTask::InjectPrimitives(
    QueueHandle_t& display_queue,
    SemaphoreHandle_t& buf1_sem,
    SemaphoreHandle_t& buf2_sem) {
  display_queue_ = display_queue;
  buf1_sem_ = buf1_sem;
  buf2_sem_ = buf2_sem;
}

/**
 * @brief 创建并配置 LVGL display 对象。
 * @return 新创建的 display 指针。
 */
lv_display_t* LVGLTask::CreateDisplayDriver() {
  lv_display_t* disp = lv_display_create(LCD_WIDTH, LCD_HEIGHT);

  // 设置双缓冲（传入像素数：宽*行数）
  lv_display_set_buffers(disp, reinterpret_cast<uint8_t*>(buf1_), reinterpret_cast<uint8_t*>(buf2_),
                         static_cast<uint32_t>(LCD_WIDTH) * ROW_HEIGHT * 2, LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(disp, lvgl_flush_cb);
  return disp;
}

/**
 * @brief LVGL 主任务：初始化 LVGL 与显示驱动，并驱动 UI 刷新循环。
 */
void LVGLTask::Run() {
  // 等待必要的同步原语注入
  while (display_queue_ == nullptr || buf1_sem_ == nullptr || buf2_sem_ == nullptr) {
    DelayMs(10);
  }

  lv_init();
  lv_tick_set_cb(GetTickMs);

  // 初始化显示驱动
  lv_display_t* disp = CreateDisplayDriver();
  (void)disp;

#if LVGL_TEST_ANIMATION
  CreateTestAnimation();
#endif

  while (true) {
    lv_timer_handler();
    DelayMs(8); // 更高刷新率（根据需要可调）
  }
}

/**
 * @brief 从 LVGL 提供的像素缓冲区指针判断对应 semaphore，并尝试非阻塞获取。
 */
static inline bool try_take_buf_sem_from_pxbuf(uint8_t* px_buf, SemaphoreHandle_t& buf1_sem, SemaphoreHandle_t& buf2_sem) {
  // px_buf passed by LVGL is uint8_t*, our buffers are uint16_t[]
  if (px_buf == reinterpret_cast<uint8_t*>(LVGLTask::GetBuf1())) {
    // try take buf1 semaphore non-blocking
    return (xSemaphoreTake(buf1_sem, 0) == pdTRUE);
  } else if (px_buf == reinterpret_cast<uint8_t*>(LVGLTask::GetBuf2())) {
    return (xSemaphoreTake(buf2_sem, 0) == pdTRUE);
  } else {
    // unknown buffer pointer: fail safe - do not block LVGL
    return false;
  }
}

/**
 * @brief 根据缓冲区指针释放对应 semaphore。
 */
static inline void give_buf_sem_from_pxbuf(uint8_t* px_buf, SemaphoreHandle_t& buf1_sem, SemaphoreHandle_t& buf2_sem) {
  if (px_buf == reinterpret_cast<uint8_t*>(LVGLTask::GetBuf1())) {
    xSemaphoreGive(buf1_sem);
  } else if (px_buf == reinterpret_cast<uint8_t*>(LVGLTask::GetBuf2())) {
    xSemaphoreGive(buf2_sem);
  }
}

/**
 * @brief LVGL flush 回调：发送区域给 DMA 任务并在 DMA 完成后通知 LVGL。
 * @param disp 当前 display 句柄。
 * @param area 本次刷新的像素区域。
 * @param px_buf 缓冲区指针（与 buf1_ / buf2_ 之一相同）。
 */
void LVGLTask::lvgl_flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_buf) {
  // Defensive: if not initialized, let LVGL continue (no blocking)
  if (display_queue_ == nullptr || buf1_sem_ == nullptr || buf2_sem_ == nullptr) {
    lv_display_flush_ready(disp);
    return;
  }

  // Try to take the semaphore corresponding to this buffer *non-blocking*
  bool got_sem = try_take_buf_sem_from_pxbuf(px_buf, buf1_sem_, buf2_sem_);
  if (!got_sem) {
    // buffer busy -> return immediately and let LVGL retry later
    lv_display_flush_ready(disp);
    return;
  }

  // Ensure cache consistency for this buffer region
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);
  clean_dcache_for_range(px_buf, w * h * 2u);

  // Prepare display message
  DisplayMessage msg;
  msg.disp = disp;
  msg.area = *area;
  msg.px_buf = px_buf;

  // Send to DMA task (short timeout). If queue send fails, release sem and notify LVGL.
  if (xQueueSend(display_queue_, &msg, pdMS_TO_TICKS(10)) != pdTRUE) {
    // Failed to enqueue -> release the buffer semaphore and inform LVGL
    give_buf_sem_from_pxbuf(px_buf, buf1_sem_, buf2_sem_);
    lv_display_flush_ready(disp);
    return;
  }

  // IMPORTANT: do NOT call lv_display_flush_ready here.
  // The UIOutputTask will call lv_display_flush_ready after DMA + (optional) BTE done.
}

/**
 * @brief 获取毫秒级 Tick，提供给 LVGL。
 */
uint32_t LVGLTask::GetTickMs() {
  return static_cast<uint32_t>(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

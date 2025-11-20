#pragma once

#include <cstdint>

#include "FreeRTOS.h"
#include "Lib/TaskBase.hpp"
#include "lvgl/lvgl.h"
#include "main.h"
#include "queue.h"
#include "semphr.h"

// 前向声明
struct DisplayMessage;

/**
 * @class LVGLTask
 * @brief LVGL 渲染任务，负责 UI 渲染和动画处理。
 */
class LVGLTask : public RAM_D2Task<LVGLTask, 2048> {
public:
  /**
   * @brief 任务入口，在 FreeRTOS 任务上下文中运行 LVGL 主循环。
   */
  void Run();

  /**
   * @brief 注入同步原语句柄。
   * @param display_queue LVGL flush 回调向 DMA 任务发送消息的队列。
   * @param buf1_sem 与 buf1_ 关联的二值信号量。
   * @param buf2_sem 与 buf2_ 关联的二值信号量。
   */
  void InjectPrimitives(QueueHandle_t& display_queue,
                        SemaphoreHandle_t& buf1_sem,
                        SemaphoreHandle_t& buf2_sem);

  /**
   * @brief 获取第一个绘制缓冲区指针（供 UIOutputTask 比对释放）。
   */
  static inline uint16_t* GetBuf1() { return buf1_; }
  /**
   * @brief 获取第二个绘制缓冲区指针（供 UIOutputTask 比对释放）。
   */
  static inline uint16_t* GetBuf2() { return buf2_; }

private:
  static constexpr uint16_t LCD_WIDTH = 800;
  static constexpr uint16_t LCD_HEIGHT = 480;
  static constexpr uint16_t ROW_HEIGHT = 120; // 1/4 屏
  static constexpr uint32_t CANVAS_BASE = 800 * 480 * 8;

  // 双缓冲区（1/4 屏幕大小），16-bit 像素
  __attribute__((section(".axi.data.testPic2"), aligned(32))) static inline uint16_t buf1_[LCD_WIDTH * ROW_HEIGHT];
  __attribute__((section(".axi.data.testPic2"), aligned(32))) static inline uint16_t buf2_[LCD_WIDTH * ROW_HEIGHT];

  // 同步原语指针
  static inline QueueHandle_t display_queue_ = nullptr;
  static inline SemaphoreHandle_t buf1_sem_ = nullptr;
  static inline SemaphoreHandle_t buf2_sem_ = nullptr;

  /**
   * @brief 创建并配置 LVGL display 对象。
   * @return 新创建的 display 指针。
   */
  lv_display_t* CreateDisplayDriver();

  /// LVGL flush 回调：将区域交由 DMA 任务处理。
  static void lvgl_flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_buf);

  /**
   * @brief 清理缓存，保证 DMA 访问一致性。
   * @param ptr 数据起始地址。
   * @param len 字节长度。
   */
  static void clean_dcache_for_range(const void* ptr, uint32_t len);

  /**
   * @brief 为 LVGL 提供 millisecond tick。
   */
  static uint32_t GetTickMs();

  static void DelayMs(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
  }
};

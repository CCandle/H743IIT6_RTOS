#pragma once

#include <cstdint>

#include "FreeRTOS.h"
#include "LT768_Lib.h"
#include "OS/TaskBase.hpp"
#include "lvgl/lvgl.h"
#include "main.h"
#include "queue.h"
#include "semphr.h"
#include "spi.h"
#include "LT768Driver.hpp"

/// 显示消息结构
struct DisplayMessage {
  lv_display_t* disp; ///< LVGL display 句柄。
  lv_area_t area;     ///< 本次刷新区域。
  uint8_t* px_buf;    ///< 像素缓冲区指针。
};

/**
 * @class UIOutputTask
 * @brief DMA传输任务，负责LCD初始化和图像数据传输
 */
class UIOutputTask : public RAM_D2Task<UIOutputTask, 1024> {
public:
  /**
   * @brief 任务入口，负责显示初始化与消息处理循环。
   */
  void Run();

  /**
   * @brief 注入同步原语：队列与 buffer 信号量。
   * @param display_queue 来自 LVGL flush 的消息队列。
   * @param buf1_sem 与 LVGL buf1 对应的二值信号量。
   * @param buf2_sem 与 LVGL buf2 对应的二值信号量。
   */
  void inject(QueueHandle_t& display_queue,
              SemaphoreHandle_t& buf1_sem,
              SemaphoreHandle_t& buf2_sem);

private:
  static constexpr uint16_t LCD_WIDTH = 800;
  static constexpr uint16_t LCD_HEIGHT = 480;
  static constexpr uint32_t CANVAS_BASE = 800 * 480 * 8;

  static inline LT768Driver lcd_driver_{};

  // 同步原语指针
  static inline QueueHandle_t display_queue_ = nullptr;
  static inline SemaphoreHandle_t buf1_sem_ = nullptr;
  static inline SemaphoreHandle_t buf2_sem_ = nullptr;
  static inline SemaphoreHandle_t dma_done_sem_ = nullptr;

  /**
   * @brief 完成显示控制器初始化。
   */
  void InitializeDisplay();

  /**
   * @brief 处理来自 LVGL 的显示更新消息。
   * @param msg 包含区域与缓冲区指针的消息。
   */
  void ProcessDisplayMessage(const DisplayMessage& msg);

  /**
   * @brief 使用 SPI DMA 将一块像素区域写入外部帧缓冲。
   * @param canvas_base 显存基地址。
   * @param x 目标区域 X 起点。
   * @param y 目标区域 Y 起点。
   * @param width 区域宽度（像素）。
   * @param height 区域高度（像素）。
   * @param bitmap_data 指向像素数据的指针。
   */
  void DrawBitmapDMA(uint32_t canvas_base, uint16_t x, uint16_t y,
                     uint16_t width, uint16_t height, const uint8_t* bitmap_data);

  static void DelayMs(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
  }

  /// 清理缓存，保证 DMA 读取最新数据。
  static void clean_dcache_for_range(const void* ptr, uint32_t len);

  // friend ISR
  friend void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi);
};

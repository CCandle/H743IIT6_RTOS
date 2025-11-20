#include "Tasks/Tasks.hpp"
#include "IPC/IPC.hpp"

namespace Tasks {
void TaskStartFailHandle() {
  for (;;) {
  }
}

void init() {
  lvgl_task.InjectPrimitives(IPC::display_queue, IPC::buf1_sem, IPC::buf2_sem);
  UI_output_task.InjectPrimitives(IPC::display_queue, IPC::buf1_sem, IPC::buf2_sem);
}

void startTasks() {
  // if (!udp_task.Start("UDPTask", tskIDLE_PRIORITY + 1)) {
  //   TaskStartFailHandle();
  // }
  // if (!lcd_task.Start("LCDTask", configMAX_PRIORITIES - 1)) {
  //   TaskStartFailHandle();
  // }
  if (!UI_output_task.Start("Screen_DMA_Task", configMAX_PRIORITIES - 2)) {
    TaskStartFailHandle();
  }
  if (!lvgl_task.Start("LVGL_Task", configMAX_PRIORITIES - 3)) {
    TaskStartFailHandle();
  }
}

} // namespace Tasks

#include "Tasks/Tasks.hpp"
#include "IPC/IPC.hpp"

namespace Tasks {
void TaskStartFailHandle() {
  for (;;) {
  }
}

void init() {
  lvgl_task.inject(IPC::display_queue, IPC::buf1_sem, IPC::buf2_sem, IPC::lvgl_ready_sem, IPC::lvgl_mutex);
  UI_output_task.inject(IPC::display_queue, IPC::buf1_sem, IPC::buf2_sem);
  touch_input_task.inject(IPC::lvgl_ready_sem, IPC::lvgl_mutex);
  key_input_task.inject(IPC::lvgl_ready_sem, IPC::lvgl_mutex);
}

void startTasks() {
  // if (!udp_task.Start("UDPTask", tskIDLE_PRIORITY + 1)) {
  //   TaskStartFailHandle();
  // }
  // if (!lcd_task.Start("LCDTask", configMAX_PRIORITIES - 1)) {
  //   TaskStartFailHandle();
  // }
  if (!touch_input_task.Start("Touch_Input_Task", configMAX_PRIORITIES - 1)) {
    TaskStartFailHandle();
  }
  if (!key_input_task.Start("Key_Input_Task", configMAX_PRIORITIES - 2)) {
    TaskStartFailHandle();
  }
  if (!UI_output_task.Start("UI_Output_Task", configMAX_PRIORITIES - 3)) {
    TaskStartFailHandle();
  }
  if (!lvgl_task.Start("LVGL_Task", configMAX_PRIORITIES - 4)) {
    TaskStartFailHandle();
  }
}

} // namespace Tasks

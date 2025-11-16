#include "Tasks/Tasks.hpp"

namespace Tasks {
void TaskStartFailHandle() {
  for (;;) {
  }
}

void init() {
}

void startTasks() {
  if(!udp_task.Start("UDPTask", tskIDLE_PRIORITY + 1)) {
    TaskStartFailHandle();
  }
}

} // namespace Tasks

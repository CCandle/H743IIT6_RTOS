#include "IPC/LogIPC.hpp"

namespace IPC::Log {

SnapshotStore snapshot_store{};

void init() {
  snapshot_store.init();
}

} // namespace IPC::Log

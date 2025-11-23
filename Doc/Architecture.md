# 系统架构概览

本项目保持 CubeMX 生成的硬件与启动代码目录不变，在其之上梳理出清晰的分层与任务职责：

- **硬件与 HAL 层**：`Core/`、`Drivers/`、`Middlewares/` 由 CubeMX 维护，提供 HAL、FreeRTOS、启动脚本等基础能力。
- **设备抽象层**：`DriversExt/` 与 `LCD_Drivers/` 封装外设驱动（例如 LT768 LCD 与触摸板），为上层提供接口而不暴露 HAL 细节。
- **系统服务层**：`OS/` 与 `App/IPC/` 定义任务基类、同步原语工厂与跨任务通信队列，解耦业务逻辑与 RTOS API。
- **业务逻辑层**：`App/Tasks/`、`App/Data/`、`App/UI/` 等实现主回路控制、数据记录、UI 刷新与输入处理。

## 目录与模块

- **`App/Tasks/`**：核心业务任务，包括主功率环 `MainCirTask`、统计与快照 `LoggerTask`、状态仲裁 `SystemTask`，以及 UI 子系统的 `LVGLTask`、`UIOutputTask`、`TouchInputTask`、`KeyInputTask`、`UDPTask`。
- **`App/IPC/`**：集中初始化并暴露跨任务同步对象（队列、二值/计数信号量、互斥量），同时定义控制命令与日志快照接口。
- **`App/Data/`**：业务数据结构与环形缓冲，`DB::MainCirBuffer` 提供高频采样帧的无锁覆盖式缓存。
- **`App/UI/`**：LVGL 组件与屏幕页面（例如 `MainScreen`），与渲染/输入任务通过 LVGL 回调协作。
- **`DriversExt/`, `LCD_Drivers/`**：LCD、触摸与通信外设的独立驱动封装。
- **`Doc/`**：设计文档与开发规范；本文件用于总体架构与数据流说明，`Naming.md`、`RefactorRules.md` 记录命名与重构规则。

### 现代化分层（C++23 目标 + CMake 拆分）

- **app_platform**：公共编译选项、接口头文件以及 `stm32cubemx`/`OS`/`DriversExt`/`LCD_Drivers`/`lvgl` 等平台依赖，确保所有子模块共享一致的裸机配置（禁用异常与 RTTI）。
- **app_utils**：`App/Utils/` 下的基础设施（例如串口日志、`printf` 适配），为其他模块提供可复用的工具函数。
- **app_config / app_data / app_logic**：配置、数据模型与算法逻辑以接口库暴露，确保任务/驱动仅依赖抽象定义即可复用。
- **app_ipc**：消息队列与控制命令服务，作为任务间的“服务层”，聚合数据/逻辑并向任务提供稳定 API。
- **app_drivers**：面向板级的业务驱动封装（按键、触摸、GT1151Q 等），与 HAL 保持隔离。
- **app_tasks**：RTOS 任务实现，链接所需的 IPC/逻辑/数据/驱动与工具库。
- **app_entry**：`App/init.cpp` 中的引导入口，负责内存/驱动/IPC/任务初始化顺序。
- **app_test_support**：测试配置头与桩，保持与主业务同样的接口与分层。

## 运行时任务与职责

- **MainCirTask** (`App/Tasks/MainCirTask.cpp`)
  - 通过 ADC + DMA 采样主电路关键量（总线电压/电流、电容电压），并根据开环/闭环模式驱动控制器输出 PWM。
  - 处理 DMA 同步超时与 ISR 报错，遇到故障时锁存并关闭 PWM，持续向日志环形缓冲写入带故障标记的帧。
- **LoggerTask** (`App/Tasks/LoggerTask.cpp`)
  - 从 `DB::MainCirBuffer` 拉取采样帧，按窗口累积均值与统计信息，更新 `SnapshotStore` 以供 UI/控制查询。
- **SystemTask** (`App/Tasks/SystemTask.cpp`)
  - 接收 `IPC::Control::control_queue` 的控制命令（Start/Stop/Toggle/ResetFault），仲裁运行状态并将设置下发至主回路。
  - 监听最新快照中的故障标志，锁存故障码并阻止运行，直到收到复位命令。
- **LVGLTask** (`App/Tasks/Screen/LVGLTask.cpp`)
  - 初始化 LVGL 显示对象与双缓冲，创建主界面，周期性运行 `lv_timer_handler`。
  - 通过队列向 `UIOutputTask` 发送渲染区域，使用信号量保护双缓冲与 LVGL 互斥访问。
- **UIOutputTask** (`App/Tasks/Screen/UIOutputTask.cpp`)
  - 收取 LVGL 渲染消息后，经 SPI DMA 将区域写入离屏缓冲，再通过 BTE 拷贝至 LCD 显存，完成后通知 LVGL 并归还缓冲信号量。
- **TouchInputTask** (`App/Tasks/Screen/TouchInputTask.cpp`)
  - 等待 LVGL 准备就绪，注册输入设备回调；在触摸中断后读取坐标，通过互斥访问 LVGL 输入接口。
- **KeyInputTask** (`App/Tasks/Screen/KeyInputTask.hpp/.cpp`)
  - 监听按键硬件事件（经 EXTI 或轮询），在获得 LVGL 就绪后把输入映射为 LVGL 键盘/编码器事件。
- **UDPTask** (`App/Tasks/UDPTask.hpp/.cpp`)
  - 预留的网络接口任务，可基于 LWIP 处理远程指令或遥测数据（目前默认未启动）。

## 跨任务通信

- **显示链路**：
  1. `LVGLTask::lvgl_flush_cb` 在渲染时清理缓存、占用对应缓冲信号量，将 `DisplayMessage` 入队至 `IPC::display_queue`。
  2. `UIOutputTask` 从队列取消息，DMA 写入后调用 `lv_display_flush_ready`，再释放缓冲信号量。
  3. `lvgl_ready_sem` 与 `lvgl_mutex` 用于 Touch/Key 任务等待 LVGL 初始化完成并序列化 LVGL 调用。
- **控制与快照**：
  - `SystemTask` 从 `control_queue` 拉取命令，调整运行模式；`LoggerTask` 将采样窗口归并成 `SnapshotStore`，供 `SystemTask` 检测故障并更新状态。
- **数据缓冲**：
  - `DB::MainCirBuffer` 为多生产者/消费者安全的覆盖式 ring buffer，确保高频采样不会阻塞。

## 数据流（文字版）

1. **采样与控制链**：ADC DMA → `MainCirTask::execute` → 控制器/调制 → PWM/保护 → 带状态的数据帧写入 `DB::MainCirBuffer`。
2. **日志与状态链**：`LoggerTask` 从环形缓冲读取 → 统计窗口 → `SnapshotStore` → `SystemTask` 读取故障/指标 → 控制队列命令触发运行/停机/复位。
3. **UI 渲染链**：LVGL 绘制 → `LVGLTask::lvgl_flush_cb` → `IPC::display_queue` → `UIOutputTask` DMA + BTE → `lv_display_flush_ready` → 缓冲信号量释放。
4. **输入链**：触摸/按键中断 → 各输入任务获取事件 → 持有 `lvgl_mutex` 调用 LVGL 输入回调 → UI 组件响应并可能发起控制命令。

## 设计原则与重构建议

- **C++23 现代化**：在 `CMakeLists.txt` 设定 C++23 标准，鼓励使用 `std::span`、`std::expected`、`enum class` 等安全抽象，同时避免在导入 HAL/FreeRTOS 时引入异常或 RTTI 以保持裸机可预测性。
- **分层边界清晰**：CubeMX 生成的硬件层保持只做驱动初始化；业务逻辑通过 `App/IPC/` 握手，不直接依赖 HAL 细节；UI 与控制逻辑经队列/信号量隔离。
- **内存与 DMA 安全**：双缓冲与 D-Cache 清理函数封装在任务内部，保证 DMA 与 CPU 访问一致性；环形缓冲采取覆盖式策略防止实时采样阻塞。
- **可测性与扩展性**：将输入/显示驱动留在独立目录，便于替换面板或通信接口；`UDPTask` 留作远程接口扩展入口；`Doc/RefactorRules.md`、本文件为未来重构提供基线。

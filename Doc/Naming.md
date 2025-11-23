# 项目命名规范

保持统一、可读、可搜索的命名风格，所有新代码按本规范执行。

- **文件与目录**：`PascalCase` 的类名文件（如 `LVGLTask.cpp`），工具/配置可用 `snake_case`（如 `ipc_config.ini`）。
- **类/结构体/枚举**：`PascalCase`，如 `DMATask`, `DisplayMessage`。
- **函数/方法**：`PascalCase`，动词在前，强调行为，如 `Run`, `InitializeDisplay`, `CreateSimpleAnimation`。
- **局部变量/形参**：`lower_snake_case`，如 `display_queue`, `buf1_sem`。
- **成员变量**：`lower_snake_case_` 结尾下划线标识成员，如 `display_queue_`；静态成员同样保持结尾下划线。
- **常量/宏**：全大写加下划线，如 `LCD_WIDTH`, `ROW_HEIGHT`；`constexpr` 常量同样使用此格式。
- **命名空间**：`PascalCase` 或全大写缩写均可，但需与已有命名保持一致（例如 `IPC`, `Tasks`）。
- **回调/ISR**：以用途或外部接口命名，遵循 SDK 要求，如 `HAL_SPI_TxCpltCallback`；自定义回调可用 `PascalCase`，如 `lvgl_flush_cb`。
- **布尔变量/函数**：名称包含意义性动词或形容词，如 `got_sem`, `IsReady()`, `HasData()`。

额外约定：
- 保持缩写一致：LCD/LVGL/LT768 等硬件缩写全大写；FreeRTOS 句柄后缀 `*_sem`, `*_queue`。
- 避免魔法数，使用已命名的常量或枚举；若不可避免，添加简短注释说明来源。
- 新增 Doxygen 注释时使用 `@brief`、`@param`、`@return` 说明接口，类与关键函数需写明职责与线程/ISR 上下文。

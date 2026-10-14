# 变更记录

本文件记录工程的版本演进。早期版本未做正式版本号管理，时间点依据文件时间戳推定。

格式参考 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.1.0/)。

---

## [Unreleased]

### 修复（2026-10-14）
- `App/svc_hmi/svc_hmi.c`：`export_data()` 中 `file_name`(15 B) / `save_data`(25 B) 缓冲区小于实际拷贝长度（18 B / 28 B）→ 分别放大到 24 B / 32 B，并改用 `memset` + `memcpy` 定长拷贝；长度统一由 `svc_export.h` 的 `EXPORT_FILE_NAME_MAX` / `EXPORT_SAVE_DATA_MAX` 宏定义（`svc_export.c` 的数组定义同样引用该宏，避免声明与定义不一致）
- `App/svc_export/svc_export.c`：`dir_path[25]` / `file_path[30]` 在长文件名下会溢出 → 放大到 64 B / 96 B，`sprintf` 改为 `snprintf`
- `App/svc_hmi/svc_hmi.c`：`HAL_UART_RxCpltCallback()` 收帧索引缺少上限检查（最坏越界约 4 KB）→ 增加边界判断，超长帧丢弃并复位状态机
- `App/svc_export/svc_export.c`：`Save_Data()` 中 `strchr()` 返回值未判空（序号格式异常时 HardFault）→ 增加判空与格式校验；`group` / `slot` 补充字符串终止符
- `Core/Src/main.c`：TIM2 中断内单次执行 1000 次 SPI 读取（约 12 ms ≫ 1 ms 中断周期）→ 恢复为"单次读取 + 均值/峰值双策略"

### 变更（2026-10-14）
- 源码编码统一为 **UTF-8 with BOM**（原为 GBK 与 UTF-8 混用，共 17 个含中文的文件）
- 自研模块目录统一更名为 **`App/`**，同步更新 Keil 工程的分组名、文件路径与包含路径
- `App/` 下的模块改用**层次前缀**命名，目录名 / 文件名 / 标识符前缀三者统一：
  `hy_ad7190`→`drv_ad7190`、`hy_flash`→`drv_w25q64`、`hy_fram`→`drv_fram`、`hy_tmc5240`→`drv_tmc5240`、
  `hy_pci`→`bsp_board`、`hy_kalman`→`alg_kalman`、`hy_hmi`→`svc_hmi`、`hy_file`→`svc_export`
  （命名规则见 README 3.2 节）
- 新增 **MIT 许可证**（`LICENSE`），并补充第三方组件许可说明

### 待办
见 `docs/待办与已知问题.md` 的 P1 / P2 列表。

---

## [2026-04-07]

面向**标定精度与运行稳定性**的一次改进版本。

### 新增
- **IWDG 硬件看门狗**：`Core/Src/iwdg.c`，LSI/64、重装 1000（约 2 s 溢出），主循环每 500 ms 喂狗
- **串口屏上电复位**：`svc_hmi.c` 新增 `hmi_res()`，上电向屏发送 `x01` 与 `rest` 指令，避免屏状态残留
- **标定常数**：`main.c` 中新增 `X0 / X100 / X200 / X500 / X1000 / X2000` 实测原始读数常量

### 变更
- 力值换算改为按实测标定的线性关系：`x1 = round(x2 / 109.635f) / 10`（灵敏度约 1096 计数/g）
- AD7190 采样率由 100 Hz 提升至 **1000 Hz**（`sampling_rate()` 锁定 `rate = 3`）
- 去皮采样点数由 20 次提升至 **200 次**，提高零点稳定性
- 卡尔曼滤波调用被注释，改用多点平均降噪

### 已知问题
- TIM2 中断内引入 1000 次 ADC 读取循环，单次中断耗时远超 1 ms 周期

---

## [2025-11-19]

**功能定版版本**——数据存储与导出链路完成。

### 新增
- **FRAM 组/槽数据模型**：`drv_fram.c` 实现"300 点/槽 → 20 槽/组 → 100 组"三级结构
- **CSV 导出流程**：`svc_export.c` 实现 `Create_File()` / `Save_Data()` / `Clear_File()`，导出主表与各槽原始波形
- **双脚踏开关**：`svc_hmi.c` 中 `svc_hmi_pedal_scan()`，左右脚踏模拟"记录"/"清零"
- **串口屏完整协议**：`0x0A` / `0x0B` / `0x0C` 命令族与曲线批量重绘 `addt()`
- 自动量程分挡（7 挡）与曲线坐标映射

---

## [2025-09 ~ 2025-10]

**框架集成阶段**。

### 新增
- 集成 **FatFs R0.12c** 与 **USB Host MSC**（`FATFS/`、`USB_HOST/`、`Middlewares/ST`）
- 集成 **RT-Thread** 内核（`Middlewares/Third_Party/RealThread_RTOS_RT-Thread`、`RT-Thread/rtconfig.h`）
- 建立 5 线程应用框架与信号量同步机制
- AD7190 驱动初版（`drv_ad7190.c`）

### 变更
- TIM2 采样节拍中断、HAL 时基（TIM1）配置定型

---

## [2025-01 ~ 2025-02]

**硬件接口与底层驱动奠基**。

### 新增
- `bsp_board.c`：GPIO 输出与 LED 指示
- `drv_tmc5240.c`：TMC5240 步进电机驱动（32 位寄存器读写、初始化参数表）——**当前固件未启用**
- `alg_kalman.c`：一维卡尔曼滤波
- STM32CubeMX 工程建立，引脚与外设分配定型（`mc01a.ioc`）

---

## 版本标记

| Tag | 对应版本 | 说明 |
|---|---|---|
| `baseline-20260407` | 2026-04-07 | 本仓库的基线版本 |

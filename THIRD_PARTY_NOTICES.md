# 第三方组件许可说明 / Third-Party Notices

本项目（GSM01A-firmware）采用 MIT 许可证，详见 [`LICENSE`](LICENSE)。

仓库的 `Drivers/` 与 `Middlewares/` 目录包含以下第三方开源组件，其版权与许可归原作者所有。
各组件的许可条款均与本项目的 MIT 许可兼容，使用或再分发时请一并保留其原始版权声明与许可文本。

| 组件 | 位置 | 许可 |
|---|---|---|
| STM32Cube HAL Driver | `Drivers/STM32F4xx_HAL_Driver/` | BSD-3-Clause（Copyright STMicroelectronics） |
| CMSIS | `Drivers/CMSIS/` | Apache-2.0（Copyright ARM Limited） |
| STM32 USB Host Library | `Middlewares/ST/STM32_USB_Host_Library/` | BSD-3-Clause（Copyright STMicroelectronics，另见该目录下的 `LICENSE.txt`） |
| FatFs | `Middlewares/Third_Party/FatFs/` | ChaN 的 FatFs 许可（允许自由使用，需保留版权声明） |
| RT-Thread | `Middlewares/Third_Party/RealThread_RTOS_RT-Thread/` | Apache-2.0（Copyright RT-Thread Development Team） |

---

## 说明

- 上述目录内的源文件均保留了其原有的版权头与许可声明，未作修改（本项目仅对
  `Core/`、`App/`、`FATFS/`、`USB_HOST/` 等自研部分进行调整）。
- 若需了解各组件许可的完整条款，请查阅其源码文件头或对应目录下的许可文本。

#pragma once

/* peripheral 层 GPIO 公共头：直接转发 interface 的 GPIO 驱动。
 * 业务模块 include 本文件即可使用 EM_GPIO_WritePin / TogglePin / ReadPin。
 * 平台相关的 EM_GPIO_ID <-> BSP_GPIO_ID 映射在 interface/euuen_c_board_bsp/gpio.c。 */

#include <euuen_mod/interface/gpio.h>

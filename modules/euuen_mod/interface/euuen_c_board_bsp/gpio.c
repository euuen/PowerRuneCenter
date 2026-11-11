#include <c_board_bsp/gpio.h>
#include <euuen_mod/interface/gpio.h>
#include <stddef.h>
#include <stdint.h>

/* ---- 平台相关：EM_GPIO_ID -> BSP_GPIO_ID 转换表（换板改这里） ---- */

const uint8_t EM_GPIO_ConvertTable[EM_GPIO_NUM] = {
    (uint8_t)BSP_GPIO_NUM, /* EM_GPIO_NONE -> 无效哨兵 */
    (uint8_t)BSP_GPIO_CS1_ACCEL,
    (uint8_t)BSP_GPIO_CS1_GYRO,
    (uint8_t)BSP_GPIO_INT_ACC,
    (uint8_t)BSP_GPIO_INT_GYRO,
    (uint8_t)BSP_GPIO_MAG_RST,
    (uint8_t)BSP_GPIO_KEY,
};

/* ---- 驱动状态：每个 EM_GPIO_ID 对应一个 BSP_GPIO 实例，首次使用时绑定 ---- */

static BSP_GPIO s_gpio[EM_GPIO_NUM];
static uint8_t s_gpio_inited[EM_GPIO_NUM];

static BSP_GPIO *EM_GPIO_Get(EM_GPIO_ID id) {
  if (id <= EM_GPIO_NONE || id >= EM_GPIO_NUM) {
    return NULL;
  }
  if (!s_gpio_inited[id]) {
    if (BSP_GPIO_Init(&s_gpio[id], (BSP_GPIO_ID)EM_GPIO_ConvertTable[id]) !=
        BSP_OK) {
      return NULL;
    }
    s_gpio_inited[id] = 1;
  }
  return &s_gpio[id];
}

/* ---- 驱动 API ---- */

void EM_GPIO_WritePin(EM_GPIO_ID id, EM_GPIO_State state) {
  BSP_GPIO *bsp_gpio = EM_GPIO_Get(id);
  if (bsp_gpio == NULL) {
    return;
  }
  BSP_GPIO_WritePin(bsp_gpio, (state == EM_GPIO_SET) ? GPIO_PIN_SET
                                                     : GPIO_PIN_RESET);
}

void EM_GPIO_TogglePin(EM_GPIO_ID id) {
  BSP_GPIO *bsp_gpio = EM_GPIO_Get(id);
  if (bsp_gpio == NULL) {
    return;
  }
  BSP_GPIO_TogglePin(bsp_gpio);
}

EM_GPIO_State EM_GPIO_ReadPin(EM_GPIO_ID id) {
  BSP_GPIO *bsp_gpio = EM_GPIO_Get(id);
  if (bsp_gpio == NULL) {
    return EM_GPIO_RESET;
  }
  return (BSP_GPIO_ReadPin(bsp_gpio) == GPIO_PIN_SET) ? EM_GPIO_SET
                                                      : EM_GPIO_RESET;
}

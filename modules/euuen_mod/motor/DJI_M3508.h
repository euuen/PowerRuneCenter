#pragma once
#include <c_board_bsp/can.h>
#include <stdint.h>

#define M3508_SPEED_SMOOTH_COEF  0.85f
#define M3508_CURRENT_SMOOTH_COEF 0.9f
#define M3508_ECD_ANGLE_COEF     0.043945f  // 360/8192
#define M3508_RPM_2_ANGLE_PER_SEC 6.0f      // RPM -> °/s

typedef struct {
    uint16_t ecd;               // 编码器值 0-8191
    float angle_single_round;  // 单圈角度
    float speed_aps;           // 角速度 °/s
    int16_t real_current;      // 实际电流 mA
    uint8_t temperature;       // 温度 ℃
    float total_angle;         // 总角度
    int32_t total_round;       // 总圈数
} M3508_Measure_s;

typedef struct {
    BSP_CAN *can;
    M3508_Measure_s measure;
    uint8_t motor_id;          // 1-4
    uint16_t ecd_last;
    uint32_t feed_cnt;
} DJI_M3508;

void DJI_M3508_Init(DJI_M3508 *motor, BSP_CAN *can, uint8_t id);
void DJI_M3508_SetCurrent(DJI_M3508 *motor, int16_t current);
void DJI_M3508_Stop(DJI_M3508 *motor);
void DJI_M3508_RxCallback(uint32_t std_id, uint8_t *data, uint8_t len);

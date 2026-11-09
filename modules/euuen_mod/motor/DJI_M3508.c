#include "DJI_M3508.h"
#include <c_board_bsp/can.h>
#include <string.h>

#define M3508_MAX_MOTORS 4

static DJI_M3508 *s_motors[M3508_MAX_MOTORS] = {NULL};

void DJI_M3508_Init(DJI_M3508 *motor, BSP_CAN *can, uint8_t id) {
    memset(motor, 0, sizeof(DJI_M3508));
    motor->can = can;
    motor->motor_id = id;
    if (id >= 1 && id <= M3508_MAX_MOTORS)
        s_motors[id - 1] = motor;
}

void DJI_M3508_SetCurrent(DJI_M3508 *motor, int16_t current) {
    uint8_t data[8] = {0};
    uint8_t idx = motor->motor_id - 1;
    data[2 * idx]     = (uint8_t)(current >> 8);
    data[2 * idx + 1] = (uint8_t)(current & 0xFF);
    BSP_CAN_SendStdFrame(motor->can, 0x200, data, 8, 1.0f);
}

void DJI_M3508_Stop(DJI_M3508 *motor) {
    DJI_M3508_SetCurrent(motor, 0);
}

void DJI_M3508_RxCallback(uint32_t std_id, uint8_t *data, uint8_t len) {
    if (len < 7) return;
    uint8_t id = (uint8_t)(std_id - 0x200);
    if (id < 1 || id > M3508_MAX_MOTORS) return;
    DJI_M3508 *motor = s_motors[id - 1];
    if (motor == NULL) return;

    M3508_Measure_s *m = &motor->measure;
    motor->ecd_last = m->ecd;
    m->ecd = ((uint16_t)data[0] << 8) | data[1];
    m->angle_single_round = M3508_ECD_ANGLE_COEF * (float)m->ecd;
    m->speed_aps = (1.0f - M3508_SPEED_SMOOTH_COEF) * m->speed_aps
                 + M3508_SPEED_SMOOTH_COEF * M3508_RPM_2_ANGLE_PER_SEC * (float)((int16_t)(data[2] << 8 | data[3]));
    m->real_current = (1.0f - M3508_CURRENT_SMOOTH_COEF) * m->real_current
                    + M3508_CURRENT_SMOOTH_COEF * (float)((int16_t)(data[4] << 8 | data[5]));
    m->temperature = data[6];

    if ((int16_t)(m->ecd - motor->ecd_last) > 4096)
        m->total_round--;
    else if ((int16_t)(m->ecd - motor->ecd_last) < -4096)
        m->total_round++;
    m->total_angle = m->total_round * 360 + m->angle_single_round;
}

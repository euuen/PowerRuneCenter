#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* 六轴 Mahony AHRS
 *
 * 输入：陀螺仪角速度（°/s）+ 加速度计比力（m/s^2）
 * 输出：姿态四元数 q = [w, x, y, z]，单位四元数
 *
 * 说明：Mahony 的航向（yaw）由陀螺仪积分得到；没有磁力计时 yaw 会缓慢漂移，
 * 加速度计只负责修正俯仰/横滚（重力方向）。 */

typedef struct Mahony_t Mahony_t;

struct Mahony_t {
  float q0, q1, q2, q3;              /* 四元数 w, x, y, z */
  float integral_x, integral_y, integral_z; /* 误差积分（rad/s） */
  float kp;                          /* 比例增益 */
  float ki;                          /* 积分增益 */
};

/* 创建实例：kp/ki 为 Mahony 增益，常用 kp=1.0，ki=0.0~0.1 */
Mahony_t *Mahony_Init(float kp, float ki);

/* 复位姿态到单位四元数，并清空误差积分 */
void Mahony_Reset(Mahony_t *self);

/* 更新一次姿态
 * gyro_dps   : 三轴角速度，单位 °/s
 * accel_mps2 : 三轴加速度，单位 m/s^2
 * dt_s       : 本次更新周期，单位 s
 */
void Mahony_Update(Mahony_t *self, const float gyro_dps[3],
                   const float accel_mps2[3], float dt_s);

/* 读取四元数 w, x, y, z（任一输出指针可为 NULL） */
void Mahony_GetQuaternion(const Mahony_t *self, float *w, float *x, float *y,
                          float *z);

#ifdef __cplusplus
}
#endif

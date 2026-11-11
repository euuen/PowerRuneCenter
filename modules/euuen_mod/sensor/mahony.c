#include "mahony.h"
#include <euuen_mod/euuen_mod.h>
#include <math.h>
#include <stddef.h>
#include <string.h>

#define MAHONY_DEG_TO_RAD 0.017453292519943295f
#define MAHONY_EPSILON 1e-6f

Mahony_t *Mahony_Init(float kp, float ki) {
  Mahony_t *self =
      (Mahony_t *)MemAllocator_Alloc(&ram_allocator, sizeof(Mahony_t));
  if (self == NULL) {
    LOG_ERROR("[Mahony_Init] instance alloc failed");
    return NULL;
  }

  memset(self, 0, sizeof(Mahony_t));
  self->kp = (kp > 0.0f) ? kp : 0.0f;
  self->ki = (ki > 0.0f) ? ki : 0.0f;
  Mahony_Reset(self);
  return self;
}

void Mahony_Reset(Mahony_t *self) {
  if (self == NULL) {
    return;
  }
  self->q0 = 1.0f;
  self->q1 = 0.0f;
  self->q2 = 0.0f;
  self->q3 = 0.0f;
  self->integral_x = 0.0f;
  self->integral_y = 0.0f;
  self->integral_z = 0.0f;
}

void Mahony_Update(Mahony_t *self, const float gyro_dps[3],
                   const float accel_mps2[3], float dt_s) {
  if (self == NULL || gyro_dps == NULL || accel_mps2 == NULL ||
      dt_s <= 0.0f) {
    return;
  }

  float gx = gyro_dps[0] * MAHONY_DEG_TO_RAD;
  float gy = gyro_dps[1] * MAHONY_DEG_TO_RAD;
  float gz = gyro_dps[2] * MAHONY_DEG_TO_RAD;

  float ax = accel_mps2[0];
  float ay = accel_mps2[1];
  float az = accel_mps2[2];

  /* 加速度计只提供方向，先归一化；无效数据时仍保留陀螺仪积分 */
  float ex = 0.0f;
  float ey = 0.0f;
  float ez = 0.0f;
  float acc_norm = sqrtf(ax * ax + ay * ay + az * az);
  if (acc_norm > MAHONY_EPSILON) {
    float inv_norm = 1.0f / acc_norm;
    ax *= inv_norm;
    ay *= inv_norm;
    az *= inv_norm;

    float q0 = self->q0;
    float q1 = self->q1;
    float q2 = self->q2;
    float q3 = self->q3;

    /* 由当前四元数估计重力方向（机体坐标系） */
    float vx = 2.0f * (q1 * q3 - q0 * q2);
    float vy = 2.0f * (q0 * q1 + q2 * q3);
    float vz = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;

    /* 测量重力方向 × 估计重力方向 = 姿态误差 */
    ex = ay * vz - az * vy;
    ey = az * vx - ax * vz;
    ez = ax * vy - ay * vx;
  }

  /* 误差积分，补偿陀螺仪零偏 */
  if (self->ki > 0.0f) {
    self->integral_x += self->ki * ex * dt_s;
    self->integral_y += self->ki * ey * dt_s;
    self->integral_z += self->ki * ez * dt_s;
  }

  gx += self->kp * ex + self->integral_x;
  gy += self->kp * ey + self->integral_y;
  gz += self->kp * ez + self->integral_z;

  /* 四元数微分：qdot = 0.5 * q ⊗ (0, gx, gy, gz) */
  float q0 = self->q0;
  float q1 = self->q1;
  float q2 = self->q2;
  float q3 = self->q3;
  float half_dt = 0.5f * dt_s;

  float n0 = q0 + (-q1 * gx - q2 * gy - q3 * gz) * half_dt;
  float n1 = q1 + (q0 * gx + q2 * gz - q3 * gy) * half_dt;
  float n2 = q2 + (q0 * gy - q1 * gz + q3 * gx) * half_dt;
  float n3 = q3 + (q0 * gz + q1 * gy - q2 * gx) * half_dt;

  float q_norm = sqrtf(n0 * n0 + n1 * n1 + n2 * n2 + n3 * n3);
  if (q_norm > MAHONY_EPSILON) {
    float inv_norm = 1.0f / q_norm;
    self->q0 = n0 * inv_norm;
    self->q1 = n1 * inv_norm;
    self->q2 = n2 * inv_norm;
    self->q3 = n3 * inv_norm;
  } else {
    Mahony_Reset(self);
  }
}

void Mahony_GetQuaternion(const Mahony_t *self, float *w, float *x, float *y,
                          float *z) {
  if (self == NULL) {
    if (w != NULL) {
      *w = 1.0f;
    }
    if (x != NULL) {
      *x = 0.0f;
    }
    if (y != NULL) {
      *y = 0.0f;
    }
    if (z != NULL) {
      *z = 0.0f;
    }
    return;
  }

  if (w != NULL) {
    *w = self->q0;
  }
  if (x != NULL) {
    *x = self->q1;
  }
  if (y != NULL) {
    *y = self->q2;
  }
  if (z != NULL) {
    *z = self->q3;
  }
}

#include "SBUS_RC.h"
#include <c_board_bsp/sys.h>
#include <c_board_bsp/usart.h>
#include <stdint.h>
#include <string.h>

SBUS_RC sbus_rc;

static BSP_USART bsp_usart_dbus;

static void SBUS_ParseFrame(uint8_t* buf)        //传入一帧数据，解析成各个通道数据，一帧长度必然是25字节
{
  memcpy(&sbus_rc.data_frame, buf, 25);
  sbus_rc.last_rx_tick = (uint32_t)BSP_GetTimestamp_ms_ISR();
}

static uint8_t buf[25]={0}; //保存一帧数据
static int8_t p = 0;   //指向下个字节将要保存的位置
void SBUS_ProcessByte(uint8_t data)
{
    int8_t q;
    buf[p] = data;
    //判断当前缓存是否满足一帧的格式
    q = p+1;
    if(q == 25) {
        q = 0;
    }
    if(buf[p] == 0x00 && buf[q] == 0x0f) {      //当前缓存数据满足0x0f开头且0x00结尾
        uint8_t buf_frame[25] = {0};
        memcpy(buf_frame, buf+q, 25-q);
        memcpy(buf_frame+25-q, buf, q);
        SBUS_ParseFrame(buf_frame);
    }

    p++;
    if(p == 25) {
        p = 0;
    }
}

void SBUS_ProcessBuffer(uint8_t* buf, uint16_t len)
{
    uint16_t i;
    for(i=0;i<len;i++) {
        SBUS_ProcessByte(buf[i]);
    }
}

static void SBUS_RC_RxCallback(uint8_t *data, uint16_t length) {
  if (length == 0) {
    return;
  }
  SBUS_ProcessBuffer(data, length);
}

void SBUS_RC_Init() {
  memset(&sbus_rc, 0, sizeof(sbus_rc));

  BSP_USART_DBUS_Init(&bsp_usart_dbus);
  BSP_USART_DBUS_AddRxCallback(SBUS_RC_RxCallback);
}

uint8_t SBUS_RC_IsOnline(void) {
  if (sbus_rc.last_rx_tick == 0) {
    return 0;
  }
  return ((uint32_t)BSP_GetTimestamp_ms() - sbus_rc.last_rx_tick) <
         RC_OFFLINE_TIMEOUT_MS;
}

uint8_t SBUS_RC_IsOnline_ISR(void) {
  if (sbus_rc.last_rx_tick == 0) {
    return 0;
  }
  return ((uint32_t)BSP_GetTimestamp_ms_ISR() - sbus_rc.last_rx_tick) <
         RC_OFFLINE_TIMEOUT_MS;
}

uint16_t SBUS_RC_GetChannel(SBUS_Channel channel_id) {
  uint8_t *buf = sbus_rc.data_frame;
  switch (channel_id) {
    /* ---- 模拟通道 1~16：11-bit，0~2047 ---- */
    case SBUS_CHANNEL_1:
      return ((buf[1] | buf[2] << 8) & 0x07FF);
    case SBUS_CHANNEL_2:
      return ((buf[2] >> 3 | buf[3] << 5) & 0x07FF);
    case SBUS_CHANNEL_3:
      return ((buf[3] >> 6 | buf[4] << 2 | buf[5] << 10) & 0x07FF);
    case SBUS_CHANNEL_4:
      return ((buf[5] >> 1 | buf[6] << 7) & 0x07FF);
    case SBUS_CHANNEL_5:
      return ((buf[6] >> 4 | buf[7] << 4) & 0x07FF);
    case SBUS_CHANNEL_6:
      return ((buf[7] >> 7 | buf[8] << 1 | buf[9] << 9) & 0x07FF);
    case SBUS_CHANNEL_7:
      return ((buf[9] >> 2 | buf[10] << 6) & 0x07FF);
    case SBUS_CHANNEL_8:
      return ((buf[10] >> 5 | buf[11] << 3) & 0x07FF);
    case SBUS_CHANNEL_9:
      return ((buf[12] | buf[13] << 8) & 0x07FF);
    case SBUS_CHANNEL_10:
      return ((buf[13] >> 3 | buf[14] << 5) & 0x07FF);
    case SBUS_CHANNEL_11:
      return ((buf[14] >> 6 | buf[15] << 2 | buf[16] << 10) & 0x07FF);
    case SBUS_CHANNEL_12:
      return ((buf[16] >> 1 | buf[17] << 7) & 0x07FF);
    case SBUS_CHANNEL_13:
      return ((buf[17] >> 4 | buf[18] << 4) & 0x07FF);
    case SBUS_CHANNEL_14:
      return ((buf[18] >> 7 | buf[19] << 1 | buf[20] << 9) & 0x07FF);
    case SBUS_CHANNEL_15:
      return ((buf[20] >> 2 | buf[21] << 6) & 0x07FF);
    case SBUS_CHANNEL_16:
      return ((buf[21] >> 5 | buf[22] << 3) & 0x07FF);

    /* ---- 数字通道 17~18：1-bit，0/1 ---- */
    case SBUS_CHANNEL_17:
      return ((buf[23] >> 0) & 0x01);
    case SBUS_CHANNEL_18:
      return ((buf[23] >> 1) & 0x01);

    default:
      return 0;
  }
}

uint8_t SBUS_RC_GetFlag(SBUS_Flag flag_id) {
  uint8_t *buf = sbus_rc.data_frame;
  switch (flag_id) {
    case SBUS_LOST_FRAME:
      return ((buf[23] >> 2) & 0x01);
    case SBUS_FAIL_SAFE:
      return ((buf[23] >> 3) & 0x01);
    default:
      return 0;
  }
}

uint8_t SBUS_RC_IsTimeout(void) {
  // 与 IsOnline 严格互补：
  //   - 从未收到过任何帧（last_rx_tick == 0）→ 视为超时
  //   - 超过 RC_OFFLINE_TIMEOUT_MS 未刷新 → 视为超时
  //   - 其余情况 → 正常
  if (sbus_rc.last_rx_tick == 0) {
    return 1;
  }
  return ((uint32_t)BSP_GetTimestamp_ms() - sbus_rc.last_rx_tick) >=
         RC_OFFLINE_TIMEOUT_MS;
}
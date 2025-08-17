/**
 *    ||          ____  _ __
 * +------+      / __ )(_) /_______________ _____  ___
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie Firmware
 */

#define DEBUG_MODULE "CRTP_PWM"

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#include "crtp.h"
#include "crtp_pwm.h"
#include "motors.h"
#include "param.h"
#include "log.h"

struct pwmPacket_s {
  uint16_t m1;
  uint16_t m2;
  uint16_t m3;
  uint16_t m4;
} __attribute__((packed));

static struct {
  uint16_t m[4];
  TickType_t tick;
  uint16_t seq;
} rx;

static uint8_t enable = 0;
static uint16_t timeoutMs = 50;
static bool wasEnabled = false;

static void crtpPwmCrtpCB(CRTPPacket* pk)
{
  if (pk->size < sizeof(struct pwmPacket_s)) {
    return;
  }
  const struct pwmPacket_s* p = (const struct pwmPacket_s*)pk->data;
  rx.m[0] = p->m1;
  rx.m[1] = p->m2;
  rx.m[2] = p->m3;
  rx.m[3] = p->m4;
  rx.tick = xTaskGetTickCountFromISR();
  rx.seq++;
}

void crtpPwmInit(void)
{
  crtpInit();
  crtpRegisterPortCB(CRTP_PORT_PWM, crtpPwmCrtpCB);
}

void crtpPwmStep(void)
{
  TickType_t now = xTaskGetTickCount();

  if (!enable) {
    if (wasEnabled) {
      motorsSetRatio(MOTOR_M1, 0);
      motorsSetRatio(MOTOR_M2, 0);
      motorsSetRatio(MOTOR_M3, 0);
      motorsSetRatio(MOTOR_M4, 0);
      wasEnabled = false;
    }
    return;
  }

  wasEnabled = true;

  if (T2M(now - rx.tick) <= timeoutMs) {
    motorsSetRatio(MOTOR_M1, rx.m[0]);
    motorsSetRatio(MOTOR_M2, rx.m[1]);
    motorsSetRatio(MOTOR_M3, rx.m[2]);
    motorsSetRatio(MOTOR_M4, rx.m[3]);
  } else {
    motorsSetRatio(MOTOR_M1, 0);
    motorsSetRatio(MOTOR_M2, 0);
    motorsSetRatio(MOTOR_M3, 0);
    motorsSetRatio(MOTOR_M4, 0);
  }
}

PARAM_GROUP_START(crtp_pwm)
PARAM_ADD(PARAM_UINT8, enable, &enable)
PARAM_ADD(PARAM_UINT16, timeoutMs, &timeoutMs)
PARAM_GROUP_STOP(crtp_pwm)

LOG_GROUP_START(crtp_pwm)
LOG_ADD(LOG_UINT16, m1, &rx.m[0])
LOG_ADD(LOG_UINT16, m2, &rx.m[1])
LOG_ADD(LOG_UINT16, m3, &rx.m[2])
LOG_ADD(LOG_UINT16, m4, &rx.m[3])
LOG_ADD(LOG_UINT16, seq, &rx.seq)
LOG_GROUP_STOP(crtp_pwm)


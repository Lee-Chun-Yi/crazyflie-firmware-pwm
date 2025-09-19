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
#include <limits.h>

#include "FreeRTOS.h"
#include "task.h"

#include "crtp.h"
#include "crtp_pwm.h"
#include "param.h"
#include "log.h"

struct pwmPacket_s {
  uint16_t m1;
  uint16_t m2;
  uint16_t m3;
  uint16_t m4;
} __attribute__((packed));

static struct {
  uint16_t m[STABILIZER_NR_OF_MOTORS];
  TickType_t tick;
  volatile uint16_t seq;
  volatile bool hasPacket;
} rx;

static uint32_t sinceLastRxMs = UINT32_MAX;

static uint8_t crtpPwmEnable = 0;
static uint8_t pwmEnable = 0;
static paramVarId_t motorPowerSetEnableParam;

static bool isMotorPowerSetInjectionEnabled(void)
{
  if (!PARAM_VARID_IS_VALID(motorPowerSetEnableParam)) {
    return false;
  }

  return paramGetInt(motorPowerSetEnableParam) == 1;
}

static void crtpPwmCrtpCB(CRTPPacket* pk)
{
  if (pk->channel != 0) {
    return;
  }
  if (pk->size != sizeof(struct pwmPacket_s)) {
    return;
  }

  const struct pwmPacket_s* p = (const struct pwmPacket_s*)pk->data;
  rx.m[0] = p->m1;
  rx.m[1] = p->m2;
  rx.m[2] = p->m3;
  rx.m[3] = p->m4;
  rx.tick = xTaskGetTickCountFromISR();
  rx.hasPacket = true;
  rx.seq++;
}

void crtpPwmInit(void)
{
  crtpInit();
  crtpRegisterPortCB(CRTP_PORT_PWM, crtpPwmCrtpCB);

  rx.tick = 0;
  rx.seq = 0;
  rx.hasPacket = false;
  sinceLastRxMs = UINT32_MAX;

  motorPowerSetEnableParam = paramGetVarId("motorPowerSet", "enable");
}

void crtpPwmStep(void)
{
  TickType_t lastTick = 0;
  const bool hasPacket = crtpPwmGetMotorThrust(NULL, &lastTick);
  const TickType_t now = xTaskGetTickCount();

  if (!hasPacket && lastTick == 0) {
    sinceLastRxMs = T2M(now);
    return;
  }

  sinceLastRxMs = T2M(now - lastTick);
}

bool crtpPwmInjectionIsEnabled(void)
{
  return (crtpPwmEnable != 0) || (pwmEnable != 0) || isMotorPowerSetInjectionEnabled();
}

bool crtpPwmGetMotorThrust(motors_thrust_uncapped_t* motorThrust, TickType_t* lastPacketTick)
{
  uint16_t localValues[STABILIZER_NR_OF_MOTORS];
  TickType_t tickSnapshot;
  uint16_t seqSnapshot;
  bool hasPacket;

  do {
    seqSnapshot = rx.seq;
    hasPacket = rx.hasPacket;
    for (int motor = 0; motor < STABILIZER_NR_OF_MOTORS; motor++) {
      localValues[motor] = rx.m[motor];
    }
    tickSnapshot = rx.tick;
  } while (seqSnapshot != rx.seq);

  if (motorThrust) {
    for (int motor = 0; motor < STABILIZER_NR_OF_MOTORS; motor++) {
      motorThrust->list[motor] = localValues[motor];
    }
  }

  if (lastPacketTick) {
    *lastPacketTick = tickSnapshot;
  }

  return hasPacket;
}

PARAM_GROUP_START(crtp_pwm)
PARAM_ADD(PARAM_UINT8, enable, &crtpPwmEnable)
PARAM_GROUP_STOP(crtp_pwm)

PARAM_GROUP_START(pwm)
PARAM_ADD(PARAM_UINT8, enable, &pwmEnable)
PARAM_GROUP_STOP(pwm)

LOG_GROUP_START(crtp_pwm)
LOG_ADD(LOG_UINT16, m1, &rx.m[0])
LOG_ADD(LOG_UINT16, m2, &rx.m[1])
LOG_ADD(LOG_UINT16, m3, &rx.m[2])
LOG_ADD(LOG_UINT16, m4, &rx.m[3])
LOG_ADD(LOG_UINT16, seq, &rx.seq)
LOG_GROUP_STOP(crtp_pwm)

LOG_GROUP_START(pwminject)
LOG_ADD(LOG_UINT16, rx_m1, &rx.m[0])
LOG_ADD(LOG_UINT16, rx_m2, &rx.m[1])
LOG_ADD(LOG_UINT16, rx_m3, &rx.m[2])
LOG_ADD(LOG_UINT16, rx_m4, &rx.m[3])
LOG_ADD(LOG_UINT32, since_last_rx, &sinceLastRxMs)
LOG_GROUP_STOP(pwminject)


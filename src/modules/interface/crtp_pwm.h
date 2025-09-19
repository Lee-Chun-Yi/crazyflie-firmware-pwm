/**
 *    ||          ____  _ __
 * +------+      / __ )(_) /_______________ _____  ___
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie Firmware
 */

#pragma once

#include "autoconf.h"

#include <stdbool.h>

#include "FreeRTOS.h"
#include "stabilizer_types.h"

#define CRTP_PWM_FAILSAFE_TIMEOUT_MS 200

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_CRTP_PWM
void crtpPwmInit(void);
void crtpPwmStep(void);
bool crtpPwmInjectionIsEnabled(void);
bool crtpPwmGetMotorThrust(motors_thrust_uncapped_t* motorThrust, TickType_t* lastPacketTick);
#else
static inline void crtpPwmInit(void) {}
static inline void crtpPwmStep(void) {}
static inline bool crtpPwmInjectionIsEnabled(void) { return false; }
static inline bool crtpPwmGetMotorThrust(motors_thrust_uncapped_t* motorThrust, TickType_t* lastPacketTick)
{
  if (motorThrust) {
    for (int motor = 0; motor < STABILIZER_NR_OF_MOTORS; motor++) {
      motorThrust->list[motor] = 0;
    }
  }
  if (lastPacketTick) {
    *lastPacketTick = 0;
  }
  return false;
}
#endif

#ifdef __cplusplus
}
#endif


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

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_CRTP_PWM
void crtpPwmInit(void);
void crtpPwmStep(void);
#else
static inline void crtpPwmInit(void) {}
static inline void crtpPwmStep(void) {}
#endif

#ifdef __cplusplus
}
#endif


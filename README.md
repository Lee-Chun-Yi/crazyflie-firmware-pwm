# Crazyflie-Firmware-PWM

This repository is a **customized Crazyflie firmware**, derived from the official [bitcraze/crazyflie-firmware](https://github.com/bitcraze/crazyflie-firmware).
It provides **low-level PWM streaming and logging support**.

This firmware is intended for researchers and developers who need **real-time PWM control** of Crazyflie 2.1 or Bolt, including integration with **Vicon motion capture** and external control loops (e.g., MATLAB or Python).



## Key Functions

* **Single-packet 4PWM transmission**

  * Adds a new CRTP command to send **four PWM values** (one packet per update)
  * Eliminates per-motor packet overhead and improves timing consistency




## Major Changes

* Add: 

  * src/modules/src/crtp_pwm.c
  * src/modules/interface/crtp_pwm.h

* Revised: 

  * src/modules/src/stabilizer.c
  * src/modules/src/Kconfig
    
* Test code
  * /test_python/test_4pwm.py
    
---

## Project Timeline – Crazyflie Firmware PWM



# Crazyflie-Firmware-PWM

This repository is a **customized Crazyflie firmware**, derived from the official [bitcraze/crazyflie-firmware](https://github.com/bitcraze/crazyflie-firmware).
It provides **low-level PWM streaming and logging support**.

This firmware is intended for researchers and developers who need **real-time PWM control** of Crazyflie 2.1 or Bolt, including integration with **Vicon motion capture** and external control loops (e.g., MATLAB or Python).



## Key Functions

* **Single-packet 4PWM transmission**

  * Adds a new CRTP command to send **four PWM values** (one packet per update)
  * Eliminates per-motor packet overhead and improves timing consistency

* **PWM logging via micro-SD (Bolt)**

  * Logs raw PWM values directly to the SD card
  * Enables offline analysis and motor calibration (per-motor compensation)


## Typical Workflow

1. Clone and build the firmware:

   ```bash
   git clone https://github.com/<yourname>/crazyflie-firmware-pwm.git
   cd crazyflie-firmware-pwm
   make clean
   make PLATFORM=bolt
   ```
2. Flash the firmware using **cfclient** or `cfloader`.
3. Connect to Crazyflie/Bolt as usual.
4. Send **4PWM packets** from your external program (MATLAB, Python, etc.).
5. (Optional) Retrieve **PWM log data** from SD card after flight.


## Project Structure

* `src/modules/pwm`

  * New CRTP port & packet handling for 4PWM

* `src/logs/pwm_logger`

  * SD-card logging routines

---

## Project Timeline – Crazyflie Firmware PWM



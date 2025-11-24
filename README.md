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

## Build and Flash

## 1. Install Required Tools

### 1.1 Download Docker Desktop

🔗 [Docker Desktop](https://www.docker.com/products/docker-desktop/?utm_source=chatgpt.com)

During installation, enable **WSL 2 Backend**.



### 1.2 Install WSL (Ubuntu)

Open **PowerShell** and run:

```powershell
wsl --install -d Ubuntu
```

```powershell
wsl -d Ubuntu
```

```powershell
cd ~
sudo apt update
```

```powershell
sudo apt install -y git
```



### 1.3 Enable WSL Integration in Docker

1. Open **Docker Desktop** → **Settings**
2. Go to **Resources → WSL Integration**
3. Enable checkbox for **Ubuntu**


### 1.4 Verify Docker in WSL

Re-open **PowerShell** and run:

```powershell
exit
```

```powershell
wsl -d Ubuntu
```

```powershell
docker --version  
```

```powershell
docker run --rm -it bitcraze/toolbelt
```

If successful, you will enter the **Bitcraze Toolbelt** container.

---

## 2. Clone Custom Firmware Repository

### 2.1 Inside the Toolbelt container:

```bash
cd ~
```

(Optional) Only use this if you want to clone same repo second time, otherwise, don't use it.

```bash
rm -rf ~/crazyflie-firmware-pwm
```

### 2.1 Clone (Replace with your GitHub link)

```bash
git clone --recursive https://github.com/Lee-Chun-Yi/crazyflie-firmware-pwm.git   
```

### 2.3 Go into the repo.

⚠️  replace with your repo name

```bash
cd crazyflie-firmware-pwm  
```

---

## 3. Build Firmware

Run the following commands inside the container:

⚠️ Substitute **`cf2`** with your firmware type:

* Crazyflie 2.0, Crazyflie 2.1(+): **`cf2`**
* Crazyflie 2.1 Brushless: **`cf21bl`**
* Crazyflie Bolt: **`bolt`**

```bash
tb make cf2_defconfig
```

```bash
tb make -j$(nproc)
```

The compiled firmware will be located at:

```
build/cf2.bin
```

Copy bin file to Windows desktop:

```
cp build/cf2.bin /mnt/c/Users/USER/Desktop/
```

---

## Troubleshooting: `permission denied: tools/build/make`

If you encounter this error, follow these steps inside your project:

### 1. Navigate to your firmware repo

```bash
cd ~/crazyflie-firmware-pwm
```

### 2. Configure Git to avoid CRLF conversion

```bash
git config --global core.autocrlf input
```

### 3. Convert build scripts to LF and add execution permission

```bash
sudo apt-get update -y
sudo apt-get install -y dos2unix

dos2unix tools/build/*

chmod +x tools/build/*
```


### 4. Rebuild firmware with Toolbelt

```bash
tb make cf2_defconfig
tb make -j"$(nproc)"
```


---

## 4. Flash Firmware to Crazyflie

### 4.1 Put on Crazyradio PA

#### 1. Open powershell, cd to your file

```bash
 cd "your file"
```
#### 2. Use cfloader to flash your .bin file (make sure your cf2/bolt is in bootloader mode)
```bash
py -m cfloader flash build\bolt.bin stm32-fw
```
or
```bash
py -m cfloader flash build\cf2.bin stm32-fw
```
⚠️ Bootloader mode: 
1. Plug your battery but power off
2. Long press power button until it flash quickly
3. You are in bootloader mode if red light isn't on

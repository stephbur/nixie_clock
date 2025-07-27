# Nixie Clock Firmware

Firmware for an ESP32-based Nixie clock, built using [PlatformIO](https://platformio.org/) and the Arduino framework. It supports sensor-driven displays, Wi-Fi connectivity, and MQTT integration. You can upload firmware via USB or OTA (Over-The-Air).

---

## 🚀 Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/stephbur/nixie_clock
cd nixie_clock
```

---

### 2. Install PlatformIO

Install PlatformIO either:

- As a VS Code extension (recommended):  
  https://platformio.org/install/ide?install=vscode

---

## ⚙️ Configuration Overview

### `platformio.ini`

This file defines the build environment, board type, upload method, and compiler flags.

```ini
[env:seeed_xiao_esp32c6]
framework = arduino
board = seeed_xiao_esp32c6

monitor_speed = 115200
upload_speed = 921600


build_flags = 
    -DUSE_LOCAL_CREDENTIALS
    -DMQTT_ENABLED
```

#### Flags explanation:

- `-DUSE_LOCAL_CREDENTIALS`  
  Use your own credentials file for Wifi and MQTT 

- `-DMQTT_ENABLED`  
  Enables MQTT integration, requires an already running MQTT server and valid credentials

---

## 🔐 Local Credentials

### Why a Local Credentials File?

To avoid committing Wi-Fi and MQTT credentials to the repository, use a local credentials override file.

### How to Create It

1. Copy the default credentials header:

```bash
cp include/credentials.h include/credentials_local.h
```

2. Edit `credentials_local.h` and replace the values:

```cpp
#ifndef CREDENTIALS_H
#define CREDENTIALS_H

static const char* WIFI_SSID = "YourWiFiSSID";
static const char* WIFI_PASSWORD = "YourWiFiPassword";

#define MQTT_BROKER      "192.168.xxx.xxx"
#define MQTT_PORT        1883
#define MQTT_USER        "your_user"
#define MQTT_PASSWORD    "your_password"
#define MQTT_TOPIC_STATUS  "nixieclock/status"
#define MQTT_TOPIC_COMMAND "nixieclock/command"

#endif // CREDENTIALS_H
```

> ⚠️ **Do not commit this file to GitHub.** Add it to `.gitignore` if not already ignored.

---


## 🔌 Upload Options

This project supports **two upload methods** for the ESP32-based Nixie Clock:

- USB (Serial)
- OTA (Over-the-Air)

Only **one** upload method should be active at a time in your `platformio.ini`.

---

### How to Choose an Upload Method

In your `platformio.ini`, you'll find this section:

```ini
; ========================
; Choose upload method below
; Uncomment ONLY ONE of the following blocks
; ========================

; --- Option 1: USB/Serial upload ---
; upload_protocol = esptool
; monitor_port = /dev/ttyACM0
; upload_port = /dev/ttyACM0

; --- Option 2: OTA upload over network ---
; Replace IP below with your device's actual IP in a separate local override file (see below)
upload_protocol = espota
upload_port = 192.168.xxx.xxx
```

**Instructions:**

- To use **USB upload**, uncomment the USB block **and comment out the OTA block**.
- To use **OTA upload**, comment out the USB block and **ensure your device's IP is set in `upload_port`**.

---

### 🧪 First-Time Setup Requires USB

You **must upload the firmware via USB** the first time to flash the Wifi credentials:

1. Connect your board via USB.
2. Uncomment the USB block:

```ini
upload_protocol = esptool
monitor_port = /dev/ttyACM0
upload_port = /dev/ttyACM0
```

3. Click the **"Upload"** button in VS Code to flash the firmware.

---

### 📡 Switching to OTA

Once OTA is enabled (in firmware), switch to the OTA block:

```ini
upload_protocol = espota
upload_port = 192.168.xxx.xxx  ; Replace with your device's IP
```

> 💡 Tip: Use your router or mDNS to discover the device’s IP. Alternatively open a serial monitor with baudrate 115200 to see the ip.

---

### 🔍 Finding Your Device’s Serial Port

To determine the correct port when using USB:

- **Linux**: run `dmesg | grep tty` after plugging in
- **Windows**: open Device Manager → Ports (COM & LPT)
- **macOS**: run `ls /dev/cu.*` and find something like `/dev/cu.usbmodemXXXX`

### 🕒 Startup Behavior

On startup, the Nixie clock blinks `88:88:88` continuously until a Wi-Fi connection is successfully established. Once connected, the display briefly shows the device’s IP address split into two 6-digit chunks (e.g. `192168` followed by `001122` for IP `192.168.1.122`). After this, normal timekeeping and sensor-driven behavior resumes.


## 🌐 Web Interface

Once connected to Wi-Fi, open your browser to see:

- Status and sensor readings  
- Debug tools (GPIO toggling, display testing)  
- Manual control buttons

---

## 📡 MQTT Topics

When enabled, the software communicates via these topics:

- Push a number to be displayed on the clock: `nixieclock/command`
- Publish status: `nixieclock/status`

To display a number execute the following on the mosquitto server

```
mosquitto_pub -h localhost -u mosquitto_user -P mosquitto_password -t "nixieclock/command" -m "123456"
```

To test the reception of the the published sensor data
```
mosquitto_sub -h localhost -p 1883 -u "mosquitto_user" -P "mosquitto_password" -t "nixieclock/status" -v
```

---


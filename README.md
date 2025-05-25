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

- Or via pip (CLI only):

```bash
pip install platformio
```

---

## ⚙️ Configuration Overview

### `platformio.ini`

This file defines the build environment, board type, upload method, and compiler flags.

```ini
[env:seeed_xiao_esp32c6]
platform = espressif32
board = seeed_xiao_esp32c6
framework = arduino

monitor_speed = 115200
upload_speed = 921600

build_flags =
    -DCORE_DEBUG_LEVEL=3
    -DLED_BUILTIN=21
    -DOTA_ENABLED

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

#define MQTT_BROKER      "xxx.xxx.xxx.xxx"
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

### Option 1: USB Upload (Wired)

1. Connect the device via USB  
2. Upload firmware:

```bash
pio run --target upload
```

> You may also use the “Upload” button in VS Code.

---

### Option 2: OTA Upload (Wireless)

OTA must be enabled at compile time (`-DOTA_ENABLED`) and the device must be running and reachable on your network.

Upload via:

```bash
pio run --target upload --upload-port YOUR_DEVICE_IP
```

Example:

```bash
pio run -t upload --upload-port 192.168.xxx.xxx
```

---

## 🌐 Web Interface

Once connected to Wi-Fi, open your browser to:

```
http://192.168.xxx.xxx/
```

You will see:

- Status and sensor readings  
- Debug tools (GPIO toggling, display testing)  
- Manual control buttons

---

## 📡 MQTT Topics

When enabled, the firmware communicates via these topics:

- Publish status: `nixieclock/status`
- Receive commands: `nixieclock/command`

---

---

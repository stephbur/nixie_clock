#ifndef CONFIG_H
#define CONFIG_H

// Define pins

// Buttons
static const int BUTTON1_PIN = 20;
static const int BUTTON2_PIN = 19;
static const int BUTTON3_PIN = 17;

// Shift register communication pins
static const int SR_CLK_PIN = 0;
static const int SR_DIN_PIN = 1;
static const int SR_LE_PIN  = 2;
static const int HV_ENABLE_PIN = 16;

// I2C pins for sensors (BMP280 & AHT20)
static const int I2C_SDA_PIN = 21;
static const int I2C_SCL_PIN = 22;

// NTP settings (Swiss time using pool servers)
static const char* NTP_PRIMARY = "0.ch.pool.ntp.org";
static const char* NTP_SECONDARY = "1.ch.pool.ntp.org";
static const long GMT_OFFSET_SEC = 3600; // CET (adjust as needed)
static const int DAYLIGHT_OFFSET_SEC = 0; // DST not handled here

// Future implementation:
// On first boot the device should start in AP mode to allow setting SSID and password.
// For now, the following is a placeholder for that functionality:
// #define ENABLE_WIFI_CONFIG_MODE

#endif // CONFIG_H

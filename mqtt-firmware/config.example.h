#pragma once
// Copy to config.h locally; never commit Wi-Fi/MQTT credentials.
#define WIFI_SSID "<wifi-ssid>"
#define WIFI_PASSWORD "<wifi-password>"
#define MQTT_HOST "<broker-lan-ip>"
#define MQTT_PORT 1883
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""
#define DEVICE_ID "grow-room-1"
#define NTP_SERVER "pool.ntp.org"
// Measure your sensor in dry substrate and fully wet substrate before use.
#define SOIL_DRY_ADC 3500
#define SOIL_WET_ADC 1500
// Confirm relay polarity WITH LOADS DISCONNECTED. 1 for active-high, 0 active-low.
#define ACTUATOR_ACTIVE_HIGH 1
#define MAX_PUMP_ON_MS 10000UL

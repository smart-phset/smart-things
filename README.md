# SmartPhset regular ESP32 firmware

Dedicated project: `smart-phset/smart-things`. This checkout was initialized as a
Git repository for Phase 8; remote existence/delivery is not yet verified.

The original root `smart-things.ino` is preserved unchanged (serial-only sensor
and actuator prototype). The separate **mqtt-firmware/** sketch implements Phase8.
Do not compile both sketches as one Arduino sketch. ESP32-CAM is another project.

## Pins and physical wiring

Classic ESP32 DevKit: DHT11 GPIO4, soil ADC GPIO34 (ADC1), fan GPIO25, light GPIO26,
pump GPIO27. Use drivers/relays/MOSFETs and external supply sized for the loads,
common ground and inductive protection as appropriate. Do not power loads directly
from GPIO. Soil ADC signal must not exceed ESP32 input voltage limits. Verify relay
active-high/active-low polarity **without loads** before connecting equipment.

## Build setup

Official releases checked 2026-10-08: ESP32 Arduino core3.3.12, PubSubClient2.8,
Adafruit DHT1.4.7, ArduinoJson7.4.3. These are new-sketch build candidates;
no full ESP32 compilation is claimed until the toolchain is available.

```bash
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.12 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install 'DHT sensor library@1.4.7' 'PubSubClient@2.8' 'ArduinoJson@7.4.3'
cp mqtt-firmware/config.example.h mqtt-firmware/config.h
# Edit ignored config.h privately; choose your board's correct relay polarity.
arduino-cli compile --fqbn esp32:esp32:esp32 mqtt-firmware
# Upload only after checking wiring and disconnecting/isolating unsafe loads.
arduino-cli upload --fqbn esp32:esp32:esp32 --port <serial-port> mqtt-firmware
```

`config.h` is ignored. Never commit Wi-Fi/MQTT credentials. Use broker LAN address,
not localhost on the board. Give every board its own DEVICE_ID (ASCII letters,
digits, hyphen/underscore, max64), and add it to backend MQTT_DEVICES. Defaults
are examples, not validated calibration: measure dry/wet ADC in your substrate
and set SOIL_DRY_ADC / SOIL_WET_ADC. Equal endpoints invalidate telemetry.
ACTUATOR_ACTIVE_HIGH must match your actual driver; startup code sets OFF before
configuring pins as outputs. GPIO boot/reset behavior still requires board testing.

## MQTT contract

- `smartphset/devices/{DEVICE_ID}/telemetry`: JSON device_id, temperature_c,
  humidity_percent, soil_moisture_percent, captured_at (UTC ISO8601 microseconds).
- `smartphset/devices/{DEVICE_ID}/state`: JSON device_id, fan/light/pump booleans,
  captured_at. These are commanded GPIO state, not independently sensed load feedback.
- `smartphset/devices/{DEVICE_ID}/commands/{fan|light|pump}`:
  `{"command_id":"uuid","on":false,"expires_at":1791460810}`.
  expires_at is integer Unix epoch seconds, not ISO text.

Publishes non-retained QoS0; command subscription QoS0. Backend publishes
non-retained, expiring commands without offline queueing. NTP sync is required:
invalid time skips telemetry/state and rejects actuation. Sensor failures skip
telemetry instead of publishing fake readings. Firmware still reports valid outputs
independently from DHT success. Publishes sensor/output state every5s and dirty
output state promptly. MQTT buffer1024, socket timeout1s, reconnect attempts at
5s intervals. Wi-Fi/broker loss observed by loop turns outputs OFF, successful
reconnect starts OFF and subscribes again. No old state is restored on reboot.

Commands require known topic actuator, actual boolean, command ID and fresh expiry
(at most15 seconds in future). Most recent command ID per actuator is ignored on
replay; ON is assignment, never toggle. Repeated ON while already on does not reset
the pump deadline. Pump defaults to auto-OFF after10s; network/DHT operations can
delay the cooperative loop, so this is not a hard realtime safety interlock.
No MQTT/TLS credential security is claimed for the existing anonymous LAN broker.

## Safe tests

```bash
c++ -std=c++17 -Wall -Wextra -Werror tests/policy_test.cpp -o /tmp/smart-things-policy-test
/tmp/smart-things-policy-test
```

Host test covers moisture calibration/clamps/invalid endpoints, actuator mapping,
command expiry/unsynced time and millis rollover. It does NOT compile Wi-Fi/DHT/
PubSubClient/ArduinoJson APIs and does NOT prove electrical/physical behavior.

Backend docs: sibling `api/docs/iot-integration.md`; normal-terminal software
acceptance script is `api/tools/verify_mqtt_integration.py` (synthetic test device,
fan OFF only). No real board acceptance yet.

Manual board checks: boot OFF; DHT/soil reasonable; NTP; telemetry/database state;
each output ON/OFF through a suitable driver; pump auto-OFF; malformed/expired
commands ignored; Wi-Fi loss/broker loss/recovery; reboot OFF; finish with all loads
OFF. Hardware acceptance remains **PENDING** until exact observations are recorded.

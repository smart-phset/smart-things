# SmartPhset · ESP32 Sensors and Actuators

ESP32 DHT11/soil telemetry and fan, light and pump control over authenticated
Mosquitto TLS. Spring persists reports to PostgreSQL; Next.js reads state and
submits explicit ON/OFF requests through server-side credentials.
The camera is a separate board in [web-cam](https://github.com/smart-phset/web-cam).

## Architecture and source

```text
ESP32 → TLS MQTT :8883 → Mosquitto → Spring → PostgreSQL → Next.js
ESP32 ← expiring non-retained commands ← Spring ← authenticated Next.js controls
```

`mqtt-firmware/mqtt-firmware.ino` is the integrated firmware.
`mqtt-firmware/policy.h` contains expiry/deduplication/pump policy;
`mqtt-firmware/mqtt_ca.h` contains the public CA certificate, not a private key;
`config.example.h` is the private configuration template.
`tests/policy_test.cpp` tests host-side policy.
The root `smart-things.ino` is an older serial-only sketch and does not implement
TLS, MQTT or the integrated safety policy. See [hardware acceptance history](HARDWARE_ACCEPTANCE.md).

## Board, dependencies and wiring

Build profile: generic Espressif ESP32 Dev Module (`esp32:esp32:esp32`), not
ESP32-CAM. Source does not prove the exact module/carrier revision. Verify its
labels and electrical ratings before wiring. Inspected toolchain: Arduino CLI
1.5.1, Espressif core 3.3.12; PubSubClient 2.8, ArduinoJson 7.4.3, DHT sensor
library 1.4.7, Adafruit Unified Sensor 1.1.15. WiFiClientSecure ships with the core.

| Component/signal | Firmware GPIO | Physical status |
|---|---|---|
| DHT11 data | 4 | Assignment verified in source; wiring must be checked |
| Soil analog input | 34 | ADC input; real soil hardware not verified |
| Fan relay/driver | 25 | Output assignment; actual load operation not verified |
| Light relay/driver | 26 | Output assignment; actual load operation not verified |
| Pump relay/driver | 27 | Output assignment; actual load operation not verified |

Use common signal ground where appropriate, rated relay/driver modules and
separate adequate load supplies. GPIOs cannot power fans/pumps/lights directly.
Confirm DHT module supply/pull-up requirements and keep ADC input within ESP32
ratings. **Verify relay polarity with loads disconnected** before applying load
power: `ACTUATOR_ACTIVE_HIGH=1` is the example, 0 selects active-low.
GPIO-reported state is not sensed confirmation of relay or load operation.

The current development setup previously documented disconnected soil hardware.
Firmware still reads GPIO34 and maps/clamps it using `SOIL_DRY_ADC`/`SOIL_WET_ADC`;
there is **no soil-disconnected detection or optional-null switch**. Floating ADC
values can look plausible and must not be represented as real soil measurements.
Calibrate with actual dry/wet substrate before using those readings.
Invalid DHT/mapped readings skip telemetry; independently reported state continues.

## Setup and flashing

```bash
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.12 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install 'DHT sensor library@1.4.7' 'Adafruit Unified Sensor@1.1.15' 'PubSubClient@2.8' 'ArduinoJson@7.4.3'
cp mqtt-firmware/config.example.h mqtt-firmware/config.h
```

Edit ignored `config.h` privately: `WIFI_SSID`, `WIFI_PASSWORD`, `MQTT_HOST`,
`MQTT_PORT`, `MQTT_USERNAME`, `MQTT_PASSWORD`, `DEVICE_ID`, `NTP_SERVER`,
`SOIL_DRY_ADC`, `SOIL_WET_ADC`, `ACTUATOR_ACTIVE_HIGH`, `MAX_PUMP_ON_MS`.
Example broker is `mqtt.ratanak.me:8883`, device `grow-room-1`, NTP `pool.ntp.org`.
The MQTT password is separate from Spring's IoT REST key. Dotenv files are not
read by firmware. Preserve the existing verified public CA in `mqtt_ca.h`.
TLS validates CA and hostname; NTP must establish a valid clock first. Do not use
`setInsecure()` or an IP address that fails certificate hostname validation.

```bash
arduino-cli board list
arduino-cli compile --fqbn esp32:esp32:esp32:UploadSpeed=115200 mqtt-firmware
# Manual flashing only; replace the port with the actual discovered device.
arduino-cli upload --fqbn esp32:esp32:esp32:UploadSpeed=115200 --port /dev/ttyUSB0 mqtt-firmware
arduino-cli monitor --port /dev/ttyUSB0 --config baudrate=115200
```

Arduino IDE: open `mqtt-firmware.ino`, select ESP32 Dev Module and actual serial
port with the same core/libraries. Windows uses `COMx`; Linux needs serial access.
Some boards require BOOT/reset for flashing. Start with every load disconnected.

## MQTT contract

| Topic | Direction and JSON |
|---|---|
| `smartphset/devices/{DEVICE_ID}/telemetry` | Out: `device_id`, `captured_at` UTC, `temperature_c`, `humidity_percent`, `soil_moisture_percent` |
| `smartphset/devices/{DEVICE_ID}/state` | Out: `device_id`, `captured_at`, boolean `fan`, `light`, `pump` |
| `smartphset/devices/{DEVICE_ID}/commands` | In: `cmd`, `command_id`, `expires_at` Unix seconds |

Telemetry and state are normally sent every 5 seconds; state also follows an
accepted change. All firmware publishes and command subscription are QoS0,
non-retained. There is no restore of old output state after reboot.

```json
{"device_id":"grow-room-1","captured_at":"2026-10-28T12:00:00Z","temperature_c":27.5,"humidity_percent":82,"soil_moisture_percent":40}
```

Command shape: `{"cmd":"P0","command_id":"unique-id","expires_at":<future-unix-seconds>}`.
P1/P0 = pump ON/OFF, F1/F0 = fan ON/OFF, L1/L0 = light ON/OFF. Bare strings and
legacy boolean MQTT commands are rejected. Expiry must be fresh and no more than
15 seconds ahead; Spring issues a 10-second expiry. The most recent command ID
per actuator is deduplicated. HTTP `202` means submitted, not physical success;
state messages report GPIO state but do **not** echo `command_id`. There is no
per-command correlated device acknowledgment. PubSubClient does not expose
SUBACK; a successful subscribe call alone is not acceptance proof.

Outputs start OFF. Observed Wi-Fi/MQTT loss, invalid clock and reconnect force
OFF. Pump auto-off defaults to 10000 ms; repeated ON does not extend the timer.
This is a cooperative firmware guard, not a hardware safety interlock. Keep
independent electrical protection appropriate to the loads.

## Testing and verification

```bash
c++ -std=c++17 -Wall -Wextra -Werror tests/policy_test.cpp -o /tmp/smart-things-policy-test
/tmp/smart-things-policy-test
git diff --check
```

Read-only TLS check (Linux CA path must exist):

```bash
openssl s_client -connect mqtt.ratanak.me:8883 -servername mqtt.ratanak.me \
  -verify_hostname mqtt.ratanak.me -verify_return_error \
  -CAfile /etc/ssl/certs/ISRG_Root_X1.pem -no-CApath -no-CAstore -brief </dev/null
```

For broker messages use a dedicated read-only account with telemetry/state ACLs;
store its credentials in the Mosquitto client's private config rather than command
history. Do not reuse device credentials if their ACL prohibits subscribing.

```bash
mosquitto_sub -h mqtt.ratanak.me -p 8883 --cafile /etc/ssl/certs/ISRG_Root_X1.pem -v \
  -t smartphset/devices/grow-room-1/state -t smartphset/devices/grow-room-1/telemetry
```

Serial should show valid NTP time, TLS authenticated, subscription sent and
successful publishes. Read Spring `/api/iot/devices/grow-room-1/state` with
`X-SmartPhset-IoT-Key` through an authorized client. Compare two readings at least
5 seconds apart: capture/receive timestamps must advance near current UTC.
HTTP 200 can return old offline state. No automated test here proves hardware.
With safely isolated loads, manually check boot OFF, mapping, pump timeout,
disconnect/reboot OFF and rejected expired commands; finish all outputs OFF.
Do not send experimental commands to the production broker.

## Troubleshooting and security

- Wi-Fi: private credentials, 2.4 GHz network, power/signal and DHCP; inspect serial IP.
- NTP: allow DNS/UDP123. Clock failure prevents TLS and timestamped telemetry and keeps outputs OFF.
- TLS: check DNS/TCP8883, valid clock and served certificate chain; preserve CA/hostname verification.
- MQTT state 4/5: credentials/ACL rejected; -2 commonly transport/TLS, -4 timeout. Never substitute the REST key for MQTT_PASSWORD.
- Stale telemetry but fresh state: inspect DHT readings/wiring. Floating soil ADC does not prove a sensor works.
- Upload/monitor: correct cable, driver, port permissions; close other serial owners.
- Reported ON with no load: inspect polarity/driver/supply safely. Relay/load wiring is not verified by software tests.

Never commit `config.h`, Wi-Fi/broker passwords or compiled firmware containing
credentials. The public CA certificate is safe to track. Review staged diffs and
rotate any credential that was previously exposed.

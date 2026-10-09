# Phase 8 physical ESP32 acceptance

Status: SOFTWARE INTEGRATION VERIFIED; PHYSICAL HARDWARE ACCEPTANCE PENDING.
Use the regular ESP32, not ESP32-CAM. Record observations below; software acceptance
and GPIO-reported state do not prove physical load operation. Do not start Phase 9.

## Current checkpoint — 2026-10-09

User-reported ESP32 discovery, upload/boot and DHT11 real end-to-end path VERIFIED.
Available: ESP32 DevKit, DHT11 3-pin module, ESP32-CAM and breadboard.
DHT11 wiring: VCC -> 3.3V, GND -> GND, DATA -> GPIO4; plausible readings and no
abnormal heating. After a Wi-Fi change left MQTT_HOST pointing at a stale laptop
IP, private firmware config was corrected to 192.168.1.200:1883, grow-room-1;
firmware recompiled/uploaded successfully.

Fresh real MQTT state reported fan/light/pump=false and advancing captured_at;
telemetry included 24.8 C / 55%. REST grow-room-1 state returned 24.9 C / 54.0%,
all outputs false and fresh state/telemetry captured/received timestamps.
DHT11 -> ESP32 -> Wi-Fi -> Mosquitto -> Spring -> PostgreSQL -> REST VERIFIED,
including Wi-Fi/MQTT connection, state publishing and NTP/time synchronization.

Soil sensor, drivers, fan, light, pump and multimeter are unavailable. Soil wiring,
ADC validation, calibration and real telemetry DEFERRED. Any soil value (including
100%) is floating GPIO34 data, not acceptance. All physical actuator compatibility,
wiring, load tests, startup OFF, pump auto-OFF and disconnect/reboot safety DEFERRED.
Do not execute the actuator steps below until the required hardware is available
and compatibility/polarity confirmed. Software-reported false is not measured OFF.
Full Phase 8 INCOMPLETE; no Phase 9.

### Final real Swagger checkpoint — 2026-10-09

User verified Swagger UI and iotKey Authorize on the real developer machine.
GET grow-room-1 state returned fresh 24.5 C / 51%, fan/light/pump=false and fresh
state/telemetry captured/received timestamps. DHT11 -> ESP32 -> MQTT -> Spring ->
PostgreSQL/current state -> REST -> Swagger VERIFIED. Fan OFF POST returned202
with generated command_id/expires_at: command acceptance only, not fan operation.
Focused IoT/MQTT tests, full tests (including AI), full build and diff check PASS
in the normal environment. Soil 100% remains INVALID floating GPIO34 data.
All previously listed soil/actuator physical checks remain DEFERRED; full Phase8
INCOMPLETE, no Phase9. No need to repeat completed software/DHT11 acceptance.

## 1. Identify and prepare the board

Disconnect fan/light/pump load power before flashing or testing polarity. Check
board model, GPIO wiring and driver compatibility against README.md. Never connect
loads directly to GPIO. Verify soil signal voltage is compatible with ESP32 input.
In a normal terminal, connect the board over a data-capable USB cable and run:

```bash
cd /home/ratanak/smart-phset/smart-things
arduino-cli board list
ls -l /dev/serial/by-id/
```

Compare before/after plugging in if multiple boards appear. Record the actual port
and board model; do not guess from an ESP32-CAM or unrelated serial adapter. Use the
correct FQBN for the identified board; commands below assume classic ESP32 DevKit.

Privately inspect existing mqtt-firmware/config.h; do not overwrite it or paste
credentials into logs. Confirm DEVICE_ID=grow-room-1, correct Wi-Fi and broker LAN
address (not localhost), NTP_SERVER, relay polarity and measured soil calibration.
Backend MQTT_DEVICES must include grow-room-1. Backend and terminal must use the
same privately configured, rotated IOT_API_KEY. Keep private config Git-ignored.

## 2. Flash and monitor, with load power disconnected

Use the previously compiled firmware if configuration/source is unchanged. If you
change config.h, compile that configuration before upload; do not reinstall tools:

```bash
# Only when config/source changed or the upload reports no compiled artifact:
arduino-cli compile --fqbn esp32:esp32:esp32 mqtt-firmware
# Replace the example port with the detected board port:
arduino-cli upload --fqbn esp32:esp32:esp32 --port /dev/ttyUSB0 mqtt-firmware
arduino-cli monitor --port /dev/ttyUSB0 --config baudrate=115200
```

Close another serial monitor before uploading. Record upload result. Press RESET
and verify each output is electrically OFF using an appropriate meter/driver
indicator with load power disconnected; software false alone does not prove this.
Check for reset-time glitches. Stop if polarity or boot behavior is unsafe.

Serial firmware prints failures such as `Sensor/time unavailable; telemetry skipped`
and `MQTT publish failed`; it has no Wi-Fi/MQTT/NTP success banners. A quiet serial
monitor is not a failure or proof of successful connection. Do not add diagnostic
code merely to repeat already-completed software acceptance.

## 3. Verify board connectivity, clock, sensors and backend

From the broker host in a second terminal:

```bash
mosquitto_sub -h localhost -p 1883 -v \
  -t 'smartphset/devices/grow-room-1/telemetry' \
  -t 'smartphset/devices/grow-room-1/state'
```

Fresh board messages should arrive about every 5 seconds. With only the intended
board using grow-room-1, this establishes Wi-Fi and MQTT operation. Confirm current
UTC captured_at advances and matches real time: this is evidence of a valid synced
clock, since firmware suppresses timestamped publications until time is valid.
Confirm sensor fields are finite/plausible; compare DHT11 with room conditions.
Observe soil response between measured dry and wet substrate conditions without
immersing sensor electronics. Percentage plausibility alone does not establish
calibration; the firmware currently does not publish raw ADC readings.

Read backend state without exposing the key in shell arguments:

```bash
cd /home/ratanak/smart-phset/api
python3 - <<'PY'
import json, os
from urllib.request import Request, urlopen
request = Request('http://localhost:9090/api/iot/devices/grow-room-1/state',
                  headers={'X-SmartPhset-IoT-Key': os.environ['IOT_API_KEY']})
with urlopen(request, timeout=5) as response:
    print(response.status, json.dumps(json.load(response), indent=2))
PY
```

Confirm actual grow-room-1 values and timestamps match fresh board publications.
This is first physical board acceptance, not a rerun of synthetic phase8-test.

## 4. Verify command handling and pump timeout

First verify GPIO/driver indications with loads disconnected. In the normal api/
terminal use this helper; IOT_API_KEY must be exported privately:

```bash
iot_command() {
  python3 - "$1" "$2" <<'PY'
import json, os, sys
from urllib.request import Request, urlopen
actuator, setting = sys.argv[1:]
assert actuator in ('fan', 'light', 'pump') and setting in ('on', 'off')
request = Request(
    'http://localhost:9090/api/iot/devices/grow-room-1/actuators/' + actuator,
    data=json.dumps({'on': setting == 'on'}).encode(),
    headers={'Content-Type': 'application/json',
             'X-SmartPhset-IoT-Key': os.environ['IOT_API_KEY']})
with urlopen(request, timeout=5) as response:
    print(response.status, response.read().decode())
PY
}
# Establish OFF first:
iot_command fan off
iot_command light off
iot_command pump off
```

For each output, issue `iot_command fan on` (substitute light/pump), observe its
actual GPIO/driver indication and MQTT state, then issue the matching OFF and
verify it. HTTP 202 means accepted publication only. Stop on unexpected operation.

For pump auto-OFF, issue pump ON once and time the actual output transition to OFF
without another ON. Default MAX_PUMP_ON_MS is 10000; enforcement is cooperative,
so record measured timing rather than claiming a hard 10-second guarantee. Verify
pump=false subsequently appears in board state and backend state.

Only after wiring, polarity and OFF behavior are confirmed should you connect
properly powered loads. Test one at a time with a manual power cutoff available;
provide suitable water flow for a pump that must not run dry. Record actual fan,
light and pump response separately from GPIO state. End each test OFF.

## 5. Verify reboot/reconnect safety

With load power disconnected initially, test reset/reboot and Wi-Fi loss/recovery.
Observe actual outputs OFF at startup and on detected connection loss; recovery
must start OFF, resubscribe and resume fresh telemetry. Record detection delay;
connection loss is observed by the loop and MQTT keepalive, not instantaneously.
Test broker loss/recovery for this physical board only when no other equipment
relies on the broker; always restore it. This board safety test remains pending
and is distinct from completed backend broker-outage software acceptance.

After reconnect, verify a fresh OFF command is handled. Repeat physical safety
observations with loads only after the disconnected-load checks pass. Finish fan,
light and pump OFF and verify actual outputs plus fresh backend state.

## Evidence record

| Check | Actual observation / timestamp |
|---|---|
| Board model / FQBN / serial port | VERIFIED: ESP32-D0WD-V3 via CP2102, /dev/ttyUSB0; actual upload FQBN not reported |
| Private configuration checked, key rotated (no values) | Pending |
| Upload / serial monitor | VERIFIED: upload successful, firmware boots/runs normally (user-reported) |
| Startup/reset electrical OFF, no unsafe glitch | DEFERRED: actuator hardware and multimeter unavailable |
| Wi-Fi / MQTT board publications | VERIFIED: real grow-room-1 state and DHT11 telemetry after stale broker IP correction |
| NTP/current advancing UTC timestamps | VERIFIED: fresh advancing captured_at in real publications |
| grow-room-1 backend state matches board | VERIFIED: real REST 24.9 C / 54.0%, outputs false, fresh captured/received timestamps |
| DHT11 reading / comparison | VERIFIED: physical readings and real end-to-end MQTT/backend/REST path (user-reported) |
| Soil wiring / ADC validation | DEFERRED / PENDING: hardware unavailable; ignore floating GPIO34 readings |
| Real soil telemetry | DEFERRED: hardware unavailable; floating GPIO34 is invalid |
| Soil dry/wet response / calibration | DEFERRED / PENDING: hardware unavailable |
| Fan actual ON/OFF / state | DEFERRED: actuator hardware unavailable |
| Light actual ON/OFF / state | DEFERRED: actuator hardware unavailable |
| Pump actual ON/OFF / state | DEFERRED: actuator hardware unavailable |
| Pump measured auto-OFF time | DEFERRED: actuator hardware unavailable |
| Reboot safety | DEFERRED: actuator hardware unavailable |
| Wi-Fi loss/reconnect safety and detection time | DEFERRED: actuator hardware unavailable |
| Broker loss/reconnect safety and detection time | DEFERRED: actuator hardware unavailable |
| Final actual outputs OFF | DEFERRED: actuator hardware unavailable |

Keep PHYSICAL HARDWARE ACCEPTANCE PENDING until observations are recorded and
reviewed. Full Phase 8 completion and delivery reconciliation remain separate.

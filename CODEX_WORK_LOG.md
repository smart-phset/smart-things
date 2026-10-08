# SmartPhset regular ESP32 — work log

## Phase 8 — MQTT telemetry and actuator control

Status: IN_PROGRESS. Host policy tests PASS; full ESP32 compilation, real broker
acceptance and physical board acceptance PENDING. No commit/staging/push.
Date: 2026-10-08. Project root: /home/ratanak/smart-phset/smart-things.

User selected dedicated smart-phset/smart-things repository; initialized local Git
main, no remote configured. Candidate GitHub URL checked with git ls-remote: DNS
failure; official web lookup cache miss. Existence UNKNOWN; do not invent/add/push
remote or claim repo absent. Check it before committing/pushing as requested.

Original serial-only smart-things.ino preserved unchanged, SHA256: d45cbb2fc6d75b962e44613dd9112b51dbc3819d90d28be9dbefdd22d4fea546.
MQTT firmware added alongside it as separate Arduino sketch, not inside AI/backend.
AI Phase7 c2b8b0091d9c256c43cc51211c5ac1ad29923086 and backend Phase7
 ae9963c38efec13ad71fbcd89de53f94c66e8bda delivered externally; both clean/origin
matched at reconciliation. Phase7 COMPLETE, Phase9 not started.

## Research before implementation

Official sources checked 2026-10-08:
https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html
https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html
https://github.com/espressif/arduino-esp32/releases (stable3.3.12 listed)
https://pubsubclient.knolleary.net/api
https://github.com/knolleary/pubsubclient/releases (latest2.8)
https://github.com/adafruit/DHT-sensor-library/releases (latest1.4.7)
https://github.com/bblanchon/ArduinoJson/releases (latest7.4.3)
https://arduinojson.org/v7/api/json/deserializejson/
Verified documented input/output: raw ADC/configured resolution, Wi-Fi STA/reconnect,
PubSubClient callback explicit byte length, QoS0 publish, buffer/socket timeout,
ArduinoJson7 JsonDocument/type checks/length-based parse. Versions are documented
build candidates; no installed ESP32 toolchain/runtime compatibility claim yet.
No cloud/API/payment. No original library/core upgrade performed.

## Implemented decisions/files

Created .gitignore, README.md, this CODEX_WORK_LOG.md;
mqtt-firmware/config.example.h, policy.h, mqtt-firmware.ino; tests/policy_test.cpp.
Original smart-things.ino not modified. No config.h/private credentials created.

GPIO DHT11=4,soilADC=34 (classicESP32 ADC1), fan25/light26/pump27. Appropriate external
power/driver/relay protection required, never GPIO-powered loads. Relay polarity
config must be verified without loads. Soil endpoints examples only, actual substrate
calibration required; reversed calibration works, identical endpoints invalid.

New topic contract shared with backend docs/iot-integration.md:
smartphset/devices/{id}/telemetry, /state, /commands/{fan|light|pump}.
JSON sensors/device/timestamp separate from output state. Commands explicit boolean,
command UUID, integer epoch-second expires_at. MQTT QoS0/nonretained; no old ON
command queue/restore. NTP required; skip invalid sensors/time, reject malformed/
expired/unknown commands. Most recent commandID per actuator ignored. Startup,
observed disconnect and reconnect OFF; pump limit10s and repeatedON while already
ON does not extend deadline. Cooperative loop/network operations mean this is not
a certified hard realtime interlock. Firmware reports GPIO assignment, not measured
physical actuator feedback. Credentials belong to ignored config.h; no secrets logged.

Wi-Fi/MQTT reconnect attempts5s; telemetry/state5s, dirty state prompt; DHT failure
does not hide reported outputs. Buffer1024/socket timeout1s. Existing anonymous
broker stays unchanged (LAN dev only); no TLS/auth strength claim.

## Validation

PASS c++ -std=c++17 -Wall -Wextra -Werror tests/policy_test.cpp and executable:
calibration/clamps/reversed/equal endpoints, actuator mapping/unknown, expiry and
unsynced clock, millis rollover. This compiles pure policy only, NOT full sketch.
Full arduino-cli compile ATTEMPTED: command not found. Not installed because package
network/toolchain unavailable here. No upload or hardware actuation performed.
Backend Java25 compile and five direct mocked methods PASS; full Gradle/Docker/
MQTT runtime blocked in Codex sandbox, tracked in api/docs/iot-integration-work-log.md.
No real ESP32 hardware, sensor, relay, pump or broker acceptance claimed.

## Exact next actions / blockers

1. Follow README to install Arduino core/library candidates and compile only
   mqtt-firmware (copy config.example.h to ignored config.h privately first).
2. Normal environment backend focused/full tests/build, restart with V4 and
   private IOT_API_KEY/MQTT_DEVICES allowlist. Run synthetic OFF-only broker script.
3. Board acceptance: startupOFF, sensors/calibration, NTP/telemetry/database, each
   actuator through drivers, malformed/expired commands, pump cutoff, Wi-Fi/broker
   loss/recovery/reboot, end all loads OFF. Record observations; hardware PENDING.
4. Verify GitHub smart-phset/smart-things exists. If truly absent ask user to create;
   DNS/cache failure alone cannot determine existence. No remote invented.
5. Only after required compile/tests pass update this log accurately, review secrets/
   diff, commit firmware separately with feat(esp32): add MQTT telemetry and actuator
   control; backend separate feat(iot): integrate MQTT sensor and actuator flow.
   No commit while validation is incomplete. No Phase9 until completion/delivery.

# SmartPhset regular ESP32 — work log

## Phase 8 — MQTT telemetry and actuator control

Status: PHASE 8 SOFTWARE INTEGRATION VERIFIED; PHYSICAL HARDWARE ACCEPTANCE PENDING.
Full Phase 8 remains INCOMPLETE; no Phase 9. See latest acceptance entry.
Final sandbox Gradle reruns blocked; completed real MQTT validation is preserved.

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

## SHUTDOWN CHECKPOINT — 2026-10-08T18:01:47.390822+07:00

Status: Phase 8 INCOMPLETE / IN_PROGRESS. Stop for the day, not completion.
This checkpoint supersedes stale missing-tool/remote/build statuses above while
preserving the historical record. No new Phase8 implementation, commit, push,
stash, reset, untracked cleanup, hardware test or shutdown performed.

### Verified/preserved state

- Backend implementation present, uncommitted on main HEAD
  ae9963c38efec13ad71fbcd89de53f94c66e8bda (published Phase7).
- Backend focused IoT/MQTT tests PASS externally; full Gradle tests PASS externally;
  Gradle build PASS externally, reported by user. Local generated XML reports
  independently inspected:21 tests,0 failures/errors/skips, including8 IoT/MQTT.
- Mosquitto was running successfully in normal developer environment, TCP1883 and
  WebSocket9001 available, per user. No new Codex broker-runtime acceptance claim.
- arduino-cli installed via Arch package, version1.5.1 independently confirmed.
- smart-things repository created/pushed by user:
  https://github.com/smart-phset/smart-things.git, branch main tracks origin/main.
  HEAD and local origin/main both6fbeb94cf354272195b54545e01dc18b202eb3e1.
  Fresh remote lookup still fails github.com DNS in Codex; external publication
  confirmed by user and matching recorded tracking ref. Do not amend/rewrite it.
- Firmware commit was made BEFORE actual Arduino compilation/live acceptance.
  Firmware has NOT yet been compiled for ESP32. Host policy tests are not a substitute.
- Original smart-things.ino checksum unchanged:
  d45cbb2fc6d75b962e44613dd9112b51dbc3819d90d28be9dbefdd22d4fea546.
  Separate MQTT sketch, policy.h, config.example.h and README compile steps present.
- SmartPhset-AI feat/live-camera HEAD
  c2b8b0091d9c256c43cc51211c5ac1ad29923086 clean, no Phase8 changes.
- No real credentials detected in inspected backend diff/untracked source or tracked
  firmware; config.h not tracked. Environment/private credential files not backed up.
- No project Spring/bootRun process visible in this sandbox's /proc scan. No process
  killed; host processes may be outside its view. If a normal-terminal bootRun is
  still running, use Ctrl+C there. Containers/data remain intact; no volume deletion.

### Remaining acceptance (all pending)

ESP32 Arduino compile; real Mosquitto telemetry -> backend -> DB/state -> REST;
REST actuator -> MQTT observation; real broker command non-retention; malformed
telemetry runtime safety; relevant broker-unavailable/recovery check. Physical ESP32,
DHT11, soil, fan/light/pump, Wi-Fi/MQTT reconnect/reboot acceptance NOT performed.
Do not energize loads automatically; finish hardware tests all OFF. Do not mark
Phase8 COMPLETE or start Phase9 until completion/delivery conditions are met.

### Recovery / resume instructions

Backend primary copy is its current working tree. Recovery directory:
/home/ratanak/smart-phset/phase8-backup (see manifest and checksums there).
Contains binary tracked-diff patch plus relative-path copies of every intended
untracked Phase8 file, excluding build/.gradle/dependencies/private configs/secrets.
Firmware checkpoint log modification is separately patched/backed up; published
6fbeb94 is unchanged. No shutdown-only WIP commit. Checkpoints deliberately remain
uncommitted. Backups are disaster recovery, never apply blindly over current work.
Read manifest before any recovery; do not restart completed work or discard WIP.
Exact first action next session:
cat /home/ratanak/smart-phset/api/docs/iot-integration-work-log.md
Then read /home/ratanak/smart-phset/smart-things/CODEX_WORK_LOG.md completely.

## Useful commands — preserved from implementation/docs

Run in the indicated repository; this checkpoint does not execute installs,
compilation, firmware upload, live actuation or new acceptance work.

Backend (`/home/ratanak/smart-phset/api`):
```bash
git status
git status -sb
git branch --show-current
git log --oneline -10
git remote -v
./gradlew test --tests '*Iot*' --tests '*Mqtt*'
./gradlew test
./gradlew build
docker ps
docker logs --tail 30 smartphset-mosquitto
docker compose up -d postgres mosquitto
./gradlew bootRun
python3 tools/verify_mqtt_integration.py --backend-url http://localhost:9090
mosquitto_sub -h localhost -p 1883 -t 'smartphset/devices/phase8-test/#' -v
```

Before bootRun set correct local DB environment privately (DB_URL, DB_USERNAME,
DB_PASSWORD), MQTT_ENABLED=true, MQTT_BROKER_URL=tcp://localhost:1883,
MQTT_DEVICES=grow-room-1,phase8-test and private IOT_API_KEY. Never put secret
values in this log or command output. Script uses IOT_API_KEY from environment;
REST header is X-SmartPhset-IoT-Key. Do not overwrite existing private .env.

Actual mosquitto CLI invocation templates used by verify_mqtt_integration.py
(the angle-bracket values are script-generated placeholders, not literal input):
```text
mosquitto_pub -h localhost -p 1883 -q 0 -t smartphset/devices/phase8-test/telemetry -m '<JSON built by script with current captured_at>'
mosquitto_pub -h localhost -p 1883 -q 0 -t smartphset/devices/phase8-test/state -m '<OFF-state JSON built by script with current captured_at>'
mosquitto_sub -h localhost -p 1883 -t smartphset/devices/phase8-test/commands/fan -q 0 -C 1 -W 10
```
Use the documented Python script for executable end-to-end software acceptance.
It sends fan OFF only. The script does not itself prove retain=false; explicitly
verify non-retention with a fresh subscriber after command publication next session.
Malformed telemetry and broker unavailable/recovery checks remain pending.
Do not interrupt the broker while real equipment depends on it.

Firmware (`/home/ratanak/smart-phset/smart-things`), copied from README:
```bash
git status
git status -sb
git branch --show-current
git log --oneline -10
git remote -v
arduino-cli version
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.12 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install 'DHT sensor library@1.4.7' 'PubSubClient@2.8' 'ArduinoJson@7.4.3'
# Only if config.h does not already exist: copy the template, then edit privately.
cp mqtt-firmware/config.example.h mqtt-firmware/config.h
arduino-cli compile --fqbn esp32:esp32:esp32 mqtt-firmware
```
Do not overwrite an existing private config.h. It is ignored and deliberately
excluded from recovery backups. No need to reinstall core/libraries already present;
inspect installed state first. No upload or hardware ON command at shutdown.

## NEXT SESSION — PHASE 8 RESUME ORDER

1. Read the entire work log.
2. Reconcile Git state in api and smart-things.
3. Confirm Docker PostgreSQL and Mosquitto.
4. Follow smart-things/README.md and compile firmware with arduino-cli.
5. Fix compilation defects if any; do not amend 6fbeb94.
6. Follow api/docs/iot-integration.md.
7. Start backend with the correct local DB/MQTT environment.
8. Test real MQTT telemetry:
   publisher -> broker -> backend -> DB/state -> REST readback.
9. Test real actuator flow:
   REST -> backend -> MQTT -> subscriber.
10. Verify command retain=false.
11. Verify malformed telemetry remains safe.
12. Perform safe broker failure/recovery test if documented.
13. If board is available, perform ESP32 hardware acceptance.
14. Only after software acceptance passes, finalize Phase 8 commits.
15. Phase 9 frontend integration begins only after Phase 8 delivery is reconciled.

## Phase 8 resumed — normal-environment runtime results (2026-10-08)

User-reported normal-environment evidence, not re-executed in Codex:
- PostgreSQL and Mosquitto running; Spring Boot started on port 9090.
- Flyway V4 (iot_device_state) applied successfully.
- ESP32 core 3.3.12; DHT 1.4.7, PubSubClient 2.8, ArduinoJson 7.4.3 installed.
- MQTT firmware compiled: 949265 bytes program storage (72%); 48628 bytes
  dynamic memory (14%). Private mqtt-firmware/config.h is Git-ignored.
- Real integration verifier PASS: telemetry/state -> Mosquitto -> Spring ->
  PostgreSQL/current state -> REST; REST fan OFF -> Spring -> MQTT observation.
  No physical response asserted. These completed checks must not be restarted.
- Temporary local test API key was weak; its value is deliberately omitted.
  Replace with a generated private key before hardware acceptance; configure the
  backend and verifier privately with the same key. No secret recorded here.

Remaining software acceptance: command non-retention, malformed telemetry through
real broker, safe broker outage/reconnect. New tools/verify_mqtt_remaining.py in
api provides separate normal-terminal checks (synthetic phase8-test, OFF only).
Syntax/help checks PASS; new live checks NOT RUN here: localhost socket access
is denied, Docker socket inaccessible, IOT_API_KEY absent from this session.
Follow api/docs/iot-integration.md remaining-check procedure. Outage test only
when no equipment relies on broker; keep Spring running and always restart broker.

After software acceptance: identify board/serial port, verify private config,
LAN broker/device allowlist and relay polarity with loads disconnected; upload
compiled MQTT sketch, then record actual hardware checklist observations.
Physical upload/board/sensor/actuator acceptance remains PENDING. Do not claim
completion, amend/rewrite firmware 6fbeb94, or start Phase 9. Backend WIP preserved;
no staging, commit or push. Original shutdown recovery backup remains unchanged;
it predates this resume entry and the new verifier.

## BREAK CHECKPOINT — 2026-10-08T19:06:44.967713+07:00

Phase 8 INCOMPLETE. Preservation only; this section supersedes earlier pending
software statuses and resume orders. User requested a pause, not further debugging.

### Verified runtime state (normal-environment results reported by user)

PASS: backend focused IoT/MQTT tests; full backend tests; backend build;
PostgreSQL/Flyway V4 migration; ESP32 firmware compilation; real MQTT telemetry
-> Spring -> PostgreSQL/current state -> REST; real REST actuator OFF -> MQTT
command observation; command non-retention; malformed telemetry safety; broker
recovery after restart. Firmware: 949265 bytes flash (72%), 48628 bytes RAM (14%).
These passes do not prove offline HTTP behavior or physical hardware acceptance.

### CURRENT BLOCKER — real broker outage returns 403 instead of 503

Same authenticated actuator request with broker ONLINE: HTTP 202.
Then `docker stop smartphset-mosquitto`; docker inspect confirmed Running=false;
waited 10 seconds; same authenticated actuator request OFFLINE: HTTP 403.
Expected HTTP 503 Service Unavailable. Broker restart/recovery works afterward.
Do NOT change the contract to accept 403. Required behavior remains online 202,
unavailable 503, wrong/missing IoT key existing 401, unknown device existing 404.
Likely investigation areas (unconfirmed): MQTT disconnect-state tracking,
MqttGateway availability, exception mapping, Spring Security ERROR dispatch and
/error handling, and why the intended unavailable response becomes 403.
No outage fix investigated or implemented during this break checkpoint.

The temporary IOT_API_KEY was exposed and MUST be rotated before further non-local
testing. Its value is not recorded or backed up. Private config.h/.env/credentials
are excluded from recovery copies; preserve/configure them privately.

### Repository preservation

api main HEAD ae9963c38efec13ad71fbcd89de53f94c66e8bda: unfinished backend WIP
remains uncommitted. smart-things main published HEAD
6fbeb94cf354272195b54545e01dc18b202eb3e1: unchanged, never amend/rewrite; checkpoint
log remains modified. SmartPhset-AI feat/live-camera HEAD
c2b8b0091d9c256c43cc51211c5ac1ad29923086: clean, unchanged. All match local
upstream refs; no fresh remote verification claimed. No staging/commit/push,
stash/reset/clean, process/container shutdown, hardware testing or Phase 9 work.

Recovery directory: /home/ratanak/smart-phset/phase8-backup (refreshed current
patches, intended untracked files, manifests and SHA256SUMS.json). Older snapshot
preserved in a timestamped sibling archive. Primary copy remains working trees;
never apply backup blindly or overwrite newer work. Generated build/dependencies
and private configuration are excluded. Checkpoint checks are recorded in backup
CHECKS.txt after preservation.

### NEXT SESSION — exact resume action

Read both complete work logs and reconcile Git first. Then investigate and fix the
real MQTT broker-outage 403 -> expected 503 behavior, and rerun:
- focused IoT/MQTT tests: ./gradlew test --tests '*Iot*' --tests '*Mqtt*'
- full Gradle tests: ./gradlew test
- Gradle build: ./gradlew build
- real online 202 / offline 503 / recovery PASS validation

Only after that should physical ESP32 acceptance begin. Do not redo Phase 7,
restart Phase 8, start Phase 9, or mark hardware acceptance complete without actual
board flash/testing evidence. Stop work now after checkpoint preservation.

## Phase 8 outage defect resume — 2026-10-08

Read both work logs and reconciled repository heads against the latest break:
api main ae9963c38efec13ad71fbcd89de53f94c66e8bda and smart-things main
6fbeb94cf354272195b54545e01dc18b202eb3e1 unchanged; existing WIP preserved.
Completed Phase 8 acceptance checks were not repeated. No Phase 7/9 work,
firmware build/upload, hardware actuation, staging, commit or push.

Reproduced the HTTP masking path in new IotErrorDispatchTests: controller returns
503 for BrokerUnavailableException, but servlet ERROR dispatch to Boot /error was
rejected with 403 by SecurityConfig.anyRequest().authenticated(). Existing
MockMvc integration tests checked only the first response and missed redispatch.
SecurityConfig now permits DispatcherType.ERROR before request-path rules. Direct
REQUEST access to /error and unrelated protected endpoints still returns 403;
IoT controller key/device checks and MQTT connection logic remain unchanged.

Regression evidence: direct Java 25.0.4.1 compilation and JUnit Platform execution
with cached Spring Boot 4.1.1 dependencies, existing compiled main classes and
Mockito agent. Before fix: 3 tests, 1 pass, 2 failures (503 and 401 masked as 403).
After fix: all 3 PASS, including 503, 401/404 error rendering and protected ordinary
requests. This is a Spring MVC/security slice with explicitly simulated container
ERROR dispatch, not a live broker or embedded-server acceptance claim. Temporary
runner/output under /tmp/phset-outage-check; existing Gradle reports not replaced.

Focused IoT/MQTT Gradle tests, full tests and build ATTEMPTED with Java 25 and a
writable /tmp/phset-outage-gradle cache copy; all blocked BEFORE tasks by
"Could not determine a usable wildcard IP for this machine." Docker ps denied
access to /var/run/docker.sock. No approval bypass attempted. Live outage retest
NOT RUN; do not mark the real defect validated yet. No broker/process stopped.

Added --check available (synthetic fan OFF, asserts HTTP 202) to the existing
remaining-check verifier and documented an isolated 202 -> 503 -> 202 procedure
in api/docs/iot-integration.md. Python syntax/help PASS. This avoids rerunning the
completed telemetry/non-retention/malformed checks. New available live check has
not run. Rotate the previously exposed key privately before non-local testing.

NEXT: in the normal Java25/Docker terminal run focused IoT/MQTT tests, full tests,
and build; restart Spring with changed code/private configuration; use the latest
HTTP regression retest procedure (only with no equipment depending on broker).
Record online202/offline503/recovery202 evidence. Physical ESP32 acceptance remains
PENDING. Phase 8 remains INCOMPLETE; published firmware commit is untouched.

## Phase 8 software acceptance VERIFIED — 2026-10-08

User supplied final normal-environment runtime evidence after the ERROR-dispatch
fix. Root cause: Spring Security blocked servlet ERROR dispatch to /error,
masking the intended 503 with 403. Fix: permit DispatcherType.ERROR before path
rules; ordinary requests and controller IoT authorization retain their checks.
Fix VERIFIED in real runtime:
- Broker ONLINE: authenticated actuator POST -> HTTP 202.
- docker stop smartphset-mosquitto; inspect Running=false; waited 10 seconds.
- Broker OFFLINE: same authenticated POST -> HTTP 503.
- docker start smartphset-mosquitto; waited 8 seconds.
- Recovered: tools/verify_mqtt_integration.py --backend-url http://localhost:9090
  PASS: real MQTT telemetry/state persistence and backend OFF command observed.
No physical device response asserted. Evidence is user-reported from the actual
normal developer environment, not a new Codex runtime execution.

PHASE 8 SOFTWARE INTEGRATION VERIFIED.
PHYSICAL HARDWARE ACCEPTANCE PENDING. Full Phase 8 remains INCOMPLETE.
This supersedes prior pending live software-outage/recovery status. No completed
real MQTT validation repeated; no production/firmware source changed this turn.
No Phase 9, staging, commits or pushes.

Requested final safe checks:
- Focused IoT/MQTT tests, full Gradle tests, Gradle build each ATTEMPTED with Java25
  and writable cached GRADLE_USER_HOME. Each blocked before tasks by sandbox
  "Could not determine a usable wildcard IP for this machine." These final reruns
  are NOT claimed passed; earlier reported normal-environment passes remain recorded.
- git diff --check PASS in api and smart-things.
- Firmware policy tests: c++ -std=c++17 -Wall -Wextra -Werror, executable PASS.
- Heuristic secret scan of 66 non-ignored text files across both repositories:
  no detected secret literals after review. Two candidate README matches were
  PGPASSWORD="$DB_PASSWORD" shell references, not credentials. Scan included
  private-key markers, AWS/GitHub token patterns, JWTs and credential assignments;
  this is not an exhaustive guarantee. .env and config.h remain Git-ignored;
  no secret values printed, copied or changed.

Hardware inspection: arduino-cli board list could not start serial/DFU discovery
(operation not permitted / libusb error); no serial device paths visible. This
cannot establish whether a board is connected in the host environment. No upload,
serial monitor, physical response or hardware acceptance performed here.
Added smart-things/HARDWARE_ACCEPTANCE.md with normal-terminal port discovery,
load-disconnected upload/polarity/startup checks, serial/MQTT/time/sensor/backend
observations, per-actuator tests, pump timeout and reboot/reconnect evidence table.
Firmware has no connection-success serial banners; use fresh MQTT timestamps and
backend readback for evidence. Never infer physical state solely from GPIO JSON.

NEXT: normal-terminal final Gradle checks (sandbox cannot execute them); follow
physical acceptance guide on the actual board. Record each actual observation,
finish outputs OFF. Do not repeat completed software MQTT acceptance unless code
changes; do not mark full Phase 8 COMPLETE until hardware acceptance is done.

## FINAL PAUSE CHECKPOINT — 2026-10-08T19:38:56+07:00

PHASE 8 SOFTWARE INTEGRATION VERIFIED.
PHYSICAL HARDWARE ACCEPTANCE PENDING. Full Phase 8 is NOT COMPLETE.
This checkpoint supersedes earlier resume instructions and hardware-discovery/
upload pending statuses. Evidence below was reported by the user from the normal
developer environment; it was not re-executed during this pause.

Verified in the normal developer environment:
- Broker online actuator request -> HTTP 202.
- Broker offline actuator request -> HTTP 503.
- Broker restored -> full MQTT integration verifier PASS.
- Firmware compilation PASS.
- ESP32 firmware uploaded successfully to /dev/ttyUSB0.
- Detected hardware: ESP32-D0WD-V3 via CP2102.
- Firmware boots and runs.
- Wi-Fi/MQTT execution path is reached.
- No sensors are currently connected.
- Serial repeatedly reports: "Sensor/time unavailable; telemetry skipped".
  This is expected because DHT11/soil sensors are not connected yet. The message
  does not independently establish NTP sync or successful real sensor telemetry.

Current hardware acceptance state:
- ESP32 discovery ✅
- Firmware upload ✅
- Firmware boot ✅
- DHT11 ⏳ not connected
- Soil sensor ⏳ not connected
- Real sensor telemetry ⏳
- Fan/light/pump hardware ⏳
- Actuator physical acceptance ⏳

Preservation: all existing work remains in the working trees, including uncommitted
backend Phase 8 files, both work logs and HARDWARE_ACCEPTANCE.md. Only these two
work logs were changed for this checkpoint. No staging, commit, push, stash, reset,
cleanup, backup overwrite, private configuration change, process/container stop,
firmware upload or hardware actuation performed during this pause. Existing
recovery backups remain untouched and predate this checkpoint; never apply them
blindly over the current working trees. No completed MQTT validation repeated.
Safe local checks only: git status inspected; git diff --check PASS in api and
smart-things after updating this checkpoint. No new Gradle or firmware tests run.

NEXT SESSION: read both work logs, use this latest checkpoint and preserve current
WIP. Resume by connecting DHT11 and soil sensor safely with board power removed
and actuator load power disconnected; follow smart-things/HARDWARE_ACCEPTANCE.md.
Then verify real sensor telemetry from grow-room-1 and matching backend state,
then proceed to actuator hardware acceptance, including startup OFF, pump auto-OFF
and reboot/reconnect safety. Record actual observations and finish outputs OFF.
Do not redo completed software MQTT acceptance unless code changes. Do NOT mark
full Phase 8 COMPLETE until hardware acceptance is done. Do NOT start Phase 9.

Pause after saving this checkpoint. SAFE TO CLOSE CODEX.


## HARDWARE ACCEPTANCE CHECKPOINT — 2026-10-09

PHASE 8 SOFTWARE INTEGRATION VERIFIED.
PHYSICAL HARDWARE ACCEPTANCE IN PROGRESS; full Phase 8 INCOMPLETE. No Phase 9.
This entry supersedes earlier DHT11-pending and soil-first resume instructions.
Evidence below is user-reported physical observation, not re-executed by Codex.

Verified:
- ESP32 detected on /dev/ttyUSB0, identified as ESP32-D0WD-V3 via CP2102.
- Firmware uploaded successfully; boots and runs normally.
- Blue 3-pin DHT11 module: VCC -> ESP32 3.3V, GND -> GND, DATA -> GPIO4.
- Stable, plausible DHT11 readings: temperature 25.30 C; humidity 69.00%.
- No abnormal heating observed.

| Acceptance area | Status |
|---|---|
| ESP32 hardware | VERIFIED |
| Firmware upload/boot | VERIFIED |
| DHT11 | VERIFIED — reported stable/plausible readings |
| Soil sensor wiring / ADC validation | DEFERRED / PENDING — hardware unavailable |
| Real soil calibration | DEFERRED / PENDING |
| Fan/light/pump hardware | PENDING |
| Actuator safety tests | PENDING |
| Full Phase 8 | INCOMPLETE |

Soil sensor is not available or connected. Any readings from disconnected GPIO34
are floating ADC values and MUST NOT count as soil acceptance or calibration.
DHT11 acceptance does not establish matching grow-room-1 MQTT/backend/REST
readback; that physical telemetry path remains pending explicit evidence. Firmware
currently includes a soil field even with no soil sensor; ignore that field for
acceptance. Do not infer valid soil hardware from successful JSON ingestion.

Soil-specific acceptance is deferred without blocking independent actuator tests.
NEXT: identify actual relay/driver modules, input compatibility, supply and polarity
with ESP32 power removed and all actuator loads disconnected. Then proceed one
physical step at a time, waiting for user observations: disconnected-load output
checks, fan/light/pump commands, startup OFF, measured pump auto-OFF, and physical
Wi-Fi/MQTT disconnect/recovery/reboot safety. Record DHT11 MQTT/backend/REST
readback separately when available; complete soil wiring/ADC/calibration later.
Never infer physical output behavior solely from GPIO JSON or HTTP 202.

Preservation: repository branches/HEADs unchanged (api main ae9963c; smart-things
main 6fbeb94). Existing WIP and backups preserved. Only both Phase 8 work logs and
HARDWARE_ACCEPTANCE.md updated; no source/private config changes, staging, commit,
push, upload or actuation. Completed software/MQTT validation was not repeated.


## DHT11 end-to-end acceptance and local Swagger — 2026-10-09

PHASE 8 SOFTWARE INTEGRATION VERIFIED. DHT11 REAL END-TO-END HARDWARE PATH
VERIFIED. Full Phase 8 INCOMPLETE; soil and actuator physical acceptance DEFERRED.
This entry supersedes earlier pending DHT11 MQTT/backend/REST and actuator statuses.

Read both full Phase 8 logs before changes; reconciled actual local WIP, not only
committed main. api main ae9963c38efec13ad71fbcd89de53f94c66e8bda; smart-things main
6fbeb94cf354272195b54545e01dc18b202eb3e1, both match local upstream refs. Existing
backend WIP preserved. AI feat/live-camera and frontend main clean. No fresh remote
verification, commits, staging, push, backup application, firmware change or Phase 9.

User-reported normal-environment hardware evidence (not re-executed here):
- Available: ESP32 DevKit, DHT11 3-pin module, ESP32-CAM, breadboard.
- ESP32-D0WD-V3 via CP2102 on /dev/ttyUSB0; compiled/uploaded/boots normally,
  no abnormal heating. DHT11 VCC=3.3V, GND=GND, DATA=GPIO4.
- Laptop Wi-Fi network change left MQTT_HOST pointing at its previous LAN IP.
  Private firmware config corrected to 192.168.1.200, port 1883, grow-room-1;
  firmware recompiled and uploaded successfully. No private config read or changed
  by Codex this turn; no Wi-Fi passwords or API keys recorded.
- Real grow-room-1 MQTT state: fan=false, light=false, pump=false, fresh captured_at.
  Telemetry example: temperature_c=24.8, humidity_percent=55, fresh captured_at.
  Verifies ESP32 Wi-Fi, Mosquitto connection, device ID, synced NTP/time, state
  publishing and real DHT11 -> ESP32 -> MQTT telemetry.
- Correctly authenticated GET /api/iot/devices/grow-room-1/state returned real
  temperature_c=24.9, humidity_percent=54.0, outputs=false and fresh independent
  state/telemetry captured_at and received_at timestamps. Verifies DHT11 -> ESP32
  -> Wi-Fi -> Mosquitto -> Spring MQTT ingestion -> PostgreSQL/current state -> REST.

Soil sensor is absent. soil_moisture_percent=100 is invalid floating GPIO34 data.
Soil wiring, ADC validation, calibration and real moisture telemetry DEFERRED;
never count JSON ingestion or that field as acceptance. Relay/MOSFET drivers,
fan, light, pump and multimeter unavailable. Driver compatibility/polarity, wiring,
physical fan/light/pump tests, startup physical OFF, physical pump auto-OFF and
physical disconnect/reboot safety all DEFERRED. MQTT booleans are not physical
acknowledgement or measured OFF. Updated HARDWARE_ACCEPTANCE.md accordingly.

Swagger implementation:
- Existing springdoc 3.1.1 dependency retained; no build.gradle changes.
- Added OpenApiConfig: OpenAPI component schemes iotKey (X-SmartPhset-IoT-Key)
  and aiKey (X-SmartPhset-AI-Key), plus operation metadata customizer. No environment
  or secret values are read into documentation. IoT tagged IoT, AI tagged AI Detection.
- IoT operations require iotKey in documentation; Authorize injects the header.
  Removed duplicate manual header inputs. State is latest/current, not history;
  POST lists fan/light/pump and defaults example to {"on":false}; documents 202 as
  accepted MQTT transmission only, 503 unavailable, 401 key errors, 404 unknown/no
  state and 400 invalid command. Existing route parameter remains {device}.
- AI ingestion retains its existing conditional key check. Its separate scheme is
  documented with an anonymous alternative and explicit configured-key condition;
  AI reads do not inherit ingestion security. No AI/controller auth source changed.
- SecurityConfig only adds documentation matchers /swagger-ui/**, /swagger-ui.html,
  /v3/api-docs and /v3/api-docs/**. Existing narrow machine-route controller auth,
  CSRF exemptions, anyRequest().authenticated() and ERROR-dispatch permission
  preserved. No /api/iot/** wildcard permission added. Direct /error stays protected.
- Added IotOpenApiTests using real springdoc/security/resources and mocked backend
  services, without broker/database. Documented local UI and OFF-only usage in
  api/docs/iot-integration.md. Official springdoc reference: https://springdoc.org/faq.html.

Validation:
- Java25.0.4.1 direct compilation of updated config and focused tests PASS, using
  cached dependencies and existing compiled main classes. Temporary harness in
  /tmp/phset-swagger-check; production Gradle reports not overwritten.
- Direct JUnit Platform: IotOpenApiTests 4 PASS + IotErrorDispatchTests 3 PASS =
  7 tests, 0 failures, 0 errors/aborts/skips. Actual generated OpenAPI and Swagger
  HTML/CSS/JS/config routes verified without keys; separate header schemes, IoT
  operation security, actuator enum/OFF example, optional AI security, and absence
  of configured synthetic test keys in generated document checked. IoT missing/
  wrong keys ->401, available command ->202, unavailable ->503, unknown ->404,
  unrelated routes/direct /error ->403; ERROR redispatch retains 503 and 401/404.
  Initial additional enum assertion used a wrong JsonPath selector (only first enum
  item); corrected assertion. An experimental TestConfiguration caused Boot to
  include the real application and fail on absent JdbcTemplate; replaced it with
  an explicit Configuration marked TestComponent so other suites cannot scan it.
  Final direct rerun: 7/7 PASS. No production MQTT/auth defect found.
- Focused Gradle Swagger/error tests ATTEMPTED first, then required
  ./gradlew test --tests '*Iot*' --tests '*Mqtt*', ./gradlew test, ./gradlew build
  with Java25, writable /tmp/phset-swagger-gradle, offline/no-daemon: each BLOCKED
  BEFORE tasks by "Could not determine a usable wildcard IP for this machine."
  No Gradle pass claimed; direct cached-classpath run is not a full build.
- git diff --check PASS in api and smart-things.
- No completed real Mosquitto/Docker acceptance repeated: metadata/doc-route changes
  do not alter MQTT runtime. No hardware test, ON command or broker interruption.

NEXT: normal developer terminal run the three required Gradle commands, restart
backend with current source/private config, open
http://localhost:9090/swagger-ui/index.html; Authorize iotKey privately, then GET
state with device=grow-room-1. OpenAPI JSON: http://localhost:9090/v3/api-docs.
Use OFF command examples for software testing; physical ON acceptance waits for
hardware/driver identification. Full Phase 8 remains INCOMPLETE; no Phase 9.


## Final software/DHT11 validation checkpoint — 2026-10-09

PHASE 8 SOFTWARE INTEGRATION VERIFIED. DHT11 -> ESP32 -> MQTT -> Spring ->
PostgreSQL/current state -> REST -> SWAGGER END-TO-END PATH VERIFIED.
FULL PHASE 8 REMAINS INCOMPLETE. No Phase 9.
This entry supersedes pending normal-machine Gradle and Swagger smoke-test actions
above; historical sandbox failures remain preserved as historical evidence.

User ran on the real developer machine:
- ./gradlew test --tests '*Iot*' --tests '*Mqtt*': PASS, including IotControllerTests,
  IotErrorDispatchTests, IotIngestionTests, IotIntegrationTests, IotOpenApiTests and
  MqttGatewayTests. Covers docs accessibility/security schemes, IoT key protection,
  success responses, broker unavailable503, unknown404, ERROR dispatch and MQTT
  reconnect/non-retention behavior.
- ./gradlew test: PASS, including all AI integration/contract tests.
- ./gradlew build: PASS.
- git diff --check: PASS.
Codex independently inspected the available Gradle XML reports: 28 tests, zero
failures/errors/skips across 10 suites (15 IoT/MQTT tests and 13 application/AI
tests). These are normal-environment reports, not a new sandbox Gradle execution.
Build and real Swagger observations below are user-reported, not re-executed here.

Real Swagger UI opened at http://localhost:9090/swagger-ui/index.html; iotKey
Authorize worked. GET /api/iot/devices/grow-room-1/state returned temperature_c=24.5,
humidity_percent=51, fan=false, light=false, pump=false and fresh independent
state/telemetry captured_at and received_at timestamps. Swagger -> authenticated
REST -> current real IoT state VERIFIED, completing the DHT11 path through Swagger.
POST /api/iot/devices/grow-room-1/actuators/fan with {"on":false} returned HTTP202
with generated command_id and expires_at. Swagger -> REST -> IoT key authentication
-> MQTT command acceptance VERIFIED; does NOT verify physical fan operation.

soil_moisture_percent=100 remains INVALID: no physical sensor on floating GPIO34.
DEFERRED due unavailable hardware: soil wiring, ADC validation, calibration and
real soil telemetry; relay/MOSFET compatibility/polarity/wiring; physical fan,
light and pump; physical startup OFF, pump auto-OFF, reboot/disconnect fail-safe.
Available ESP32/DHT11/Wi-Fi/MQTT/NTP/state publishing and real sensor path are
VERIFIED. No physical actuator pass inferred from output booleans or HTTP202.

Delivery preparation: reconciled api main ae9963c and smart-things main 6fbeb94,
with existing Phase8 WIP preserved and no pre-existing staged changes. Completed
backend IoT/MQTT/OpenAPI source, V4 migration, regression tests, software verifiers
and documentation are ready for a normal new backend commit. Firmware repository
changes are documentation only (this log and HARDWARE_ACCEPTANCE.md); published
firmware 6fbeb94cf354272195b54545e01dc18b202eb3e1 will not be amended or rewritten.
Private .env/config.h remain ignored, excluded from staging, unread and unchanged.
No further validation reruns, MQTT commands, source changes, uploads, broker stops
or Phase9 work performed for this checkpoint. Local diff checks passed.

NEXT: preserve these completed software/DHT11 results; physical soil/actuator
acceptance resumes only when hardware is available. Full Phase8 stays INCOMPLETE.

## Physical MQTT relay control VERIFIED — 2026-10-10

User-reported real hardware result: Swagger pump command sent successfully;
Spring REST published the MQTT pump command; ESP32 received it and used GPIO27;
the physical 5V relay switched successfully. Relay ON/OFF control VERIFIED:
Swagger -> Spring REST -> MQTT -> ESP32 -> GPIO27 -> physical relay switching.
This supersedes the earlier pending relay-control status only. Codex did not
perform the physical test. Actual water pump/load operation is NOT VERIFIED.
No physical pump timeout, startup/reset or disconnect safety acceptance is inferred.

DEFERRED / UNVERIFIED: real soil sensor, soil ADC validation/calibration, real soil
telemetry, physical fan, physical light, actual water pump/load operation and
remaining actuator physical safety acceptance. Floating GPIO34 values remain
INVALID as soil acceptance. Full Phase 8 remains INCOMPLETE; no Phase 9.

Firmware logging cleanup: removed recurring five-second sensor debug dumps and
redundant command-received messages. Retained concise Wi-Fi/MQTT connection/retry
and subscription logs, time-readiness transitions, accepted actuator commands,
GPIO assignments and meaningful rejection/telemetry errors. MQTT topics/payloads,
device ID behavior, GPIO mappings, polarity, pump timeout, command validation,
fail-safe behavior and telemetry/NTP validation are preserved. Private config.h
remains ignored, unchanged and excluded from the commit; no credentials logged.

Validation: host policy/unit tests PASS with C++17, -Wall -Wextra -Werror;
arduino-cli compile for esp32:esp32:esp32 PASS using the /tmp build-path workaround
(default cache read-only): 952757 bytes flash (72%), 48636 bytes RAM (14%).
git diff --check PASS. HARDWARE_ACCEPTANCE.md records the same relay-only result.
No upload performed. New commit/push authorized by the user; prior checkpoint
commits will not be amended or rewritten. Other repositories remain untouched.

## Explicit command codes / single MQTT topic — 2026-10-24

User authorized paired Spring/firmware protocol change; no commit/push/upload.
Started clean at main bbf860d, paired backend main f66e05d. Existing frontend and
AI work preserved. Read current logs and code before changes.

Subscribe exactly smartphset/devices/{DEVICE_ID}/commands on initial connect and
reconnect. Parse only P1/P0/F1/F0/L1/L0 in JSON cmd; old actuator suffix and MQTT on
field removed. command_id stays nonempty/max64, now also rejects embedded NUL;
expires_at stays integer with synchronized-time/current..+15s checks. Preserve
most-recent-ID-per-actuator replay rejection (not an unlimited replay history).
Fixed-size ID storage exposes the same duplicate policy to host tests. No toggles.
Received-code/accepted/GPIO logs are concise and contain no credentials.

GPIO25/26/27, configured polarity, startup/disconnect/reconnect OFF, state JSON,
DHT telemetry and 5s reporting remain unchanged. Extracted existing pump timer
start predicate for tests; repeated ON while already ON cannot reset its timer.
MAX_PUMP_ON_MS stays configured as before (example10000); auto-OFF loop preserved.
No private config read/modified. Floating GPIO34 is not valid soil telemetry.

Validation: C++17 -Wall -Wextra -Werror host policy tests PASS: six codes, invalid
codes, ID validation/duplicates, expired/future/unsynced time, pump timer start and
no-reset, 10s expiry predicate and millis rollover. arduino-cli compile for
esp32:esp32:esp32 using /tmp/smartphset-command-codes-build PASS: 952105 bytes flash
(72%), 48780 bytes RAM (14%). These checks do not prove physical relay behavior.
Backend Gradle checks blocked before tasks by sandbox wildcard-IP restriction;
supplemental focused JUnit tests passed. git diff --check PASS in both repositories.

README documents paired deployment, exact upload/monitor/subscriber commands and
manual acceptance. NEW PROTOCOL NOT PHYSICALLY VERIFIED. Prior GPIO27/5V relay
verification remains historical. Actual water pump/load, physical fan/light and
real soil hardware/calibration/telemetry remain unverified. Await user relay test.

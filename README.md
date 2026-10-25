# SmartPhset regular ESP32

Firmware and onboarding for the regular ESP32 grow-room MQTT controller.
ESP32-CAM source and its Linux/Windows guide live exclusively in the dedicated
[smart-phset/web-cam repository](https://github.com/smart-phset/web-cam).

## Role in SmartPhset

Regular ESP32 ↔ Wi-Fi/Mosquitto ↔ [Spring](https://github.com/smart-phset/phset-api)
↔ PostgreSQL ↔ [Next.js](https://github.com/smart-phset/phset-client/tree/feat/grow-room-monitoring).
[ESP32-CAM](https://github.com/smart-phset/web-cam) → raw MJPEG → [Python AI](https://github.com/smart-phset/phset-ai/tree/feat/live-camera)
→ annotated video and detection metadata. The camera does not run YOLO or use MQTT.
See the frontend [complete startup order](https://github.com/smart-phset/phset-client/blob/feat/grow-room-monitoring/README.md#start-smartphset-from-a-fresh-computerreboot).

```text
mqtt-firmware/                 regular ESP32 MQTT sketch
tests/policy_test.cpp          host-side controller policy tests
smart-things.ino               preserved original serial-only prototype
```

Compile `mqtt-firmware`, not the repository root. The root sketch is the preserved
serial-only prototype. Do not copy camera source into this repository; clone
[web-cam](https://github.com/smart-phset/web-cam) separately for camera work.

## Requirements

Linux: Git, Arduino CLI, USB serial access (distribution-specific device group/udev
permissions), and a C++17 compiler for host tests. Windows: Git for Windows,
PowerShell, Arduino CLI on PATH and the USB-UART driver's correct COM port. For
host tests install a C++17 compiler such as MSVC Build Tools and use Developer
PowerShell, or g++ on PATH. No WSL is required. Backend Mosquitto needs Docker
Engine on Linux or running Docker Desktop on Windows.

Inspected toolchain: Arduino CLI **1.5.1**, Espressif `esp32:esp32` **3.3.12**,
DHT sensor library **1.4.7**, Adafruit Unified Sensor **1.1.15**, PubSubClient **2.8**,
ArduinoJson **7.4.3**. Use the Espressif core, not the separately installed
`arduino:esp32` core.

Regular board: ESP32 DevKit, DHT11, appropriate relay/MOSFET drivers and load power.

## Clone

```text
git clone https://github.com/smart-phset/smart-things.git
cd smart-things
git switch main
```

## Configuration

Copy safe examples once, edit privately, and recompile/reflash after changes:

| File | Purpose | Secret? / commit? |
|---|---|---|
| mqtt-firmware/config.example.h | Safe controller template | No secrets; commit |
| mqtt-firmware/config.h | Actual controller/network settings | Contains secrets; never commit |

### Regular ESP32 pins and values

| Hardware | GPIO |
|---|---|
| DHT11 data | 4 |
| Soil ADC input | 34 |
| Fan driver | 25 |
| Light driver | 26 |
| Pump driver | 27 |

**The current development setup has no real soil sensor installed. Floating GPIO34
readings are not valid soil moisture**, even if firmware emits a number. Do not
use them for calibration, automation decisions or acceptance.

| config.h define | Purpose / example | Secret? |
|---|---|---|
| WIFI_SSID | Your network, `YOUR_WIFI_NAME` | Private network information |
| WIFI_PASSWORD | `YOUR_WIFI_PASSWORD` | Yes |
| MQTT_HOST | Mosquitto computer LAN IPv4, `YOUR_LAPTOP_IP` | No |
| MQTT_PORT | `1883` | No |
| MQTT_USERNAME / MQTT_PASSWORD | Empty for current anonymous broker | Password yes |
| DEVICE_ID | Unique board ID, `grow-room-1`; also in Spring MQTT_DEVICES | No |
| NTP_SERVER | `pool.ntp.org`; clock sync required | No |
| SOIL_DRY_ADC / SOIL_WET_ADC | Template 3500 / 1500; unverified examples, measure actual sensor | No |
| ACTUATOR_ACTIVE_HIGH | `1` active-high or `0` active-low; verify with loads disconnected | No |
| MAX_PUMP_ON_MS | `10000UL`, approximately 10-second pump limit | No |

Use drivers and external supply sized for loads, common ground and inductive
protection. Never power a load directly from GPIO. Verify voltage and relay
polarity with loads disconnected. GPIO-reported state is not physical feedback.

### Find your laptop IP — essential MQTT rule

**MQTT_HOST must be the IP address of the computer running Mosquitto.**
`localhost` on an ESP32 means the ESP32, not your laptop. Linux:

```bash
ip route
ip -br addr
```

Windows PowerShell:

```powershell
ipconfig
```

Choose the IPv4 of the Wi-Fi/Ethernet adapter connected to the same reachable
network as the ESP32, not a Docker, VPN, virtual or disconnected adapter.
For example only, if your laptop IPv4 is **192.168.1.25**, use:

```cpp
#define MQTT_HOST "192.168.1.25"
```

Replace that example with your own IP. DHCP/network changes can require editing
config.h and uploading again. Boards and laptop must be able to reach each other;
guest Wi-Fi/client isolation can block them even with similar addresses.

On Windows, allow inbound **TCP 1883** for Mosquitto/Docker on the trusted private
network if blocked. Windows Defender Firewall → Advanced settings → Inbound Rules
→ New Rule → Port → TCP 1883 → allow on Private profile; scope to your local
network. Do not disable the firewall. Spring and firmware MQTT credentials must
match broker settings if you later enable broker authentication.

### Separate ESP32-CAM setup

Use the [web-cam README](https://github.com/smart-phset/web-cam/blob/main/README.md)
for AI-Thinker/OV2640 hardware, private Wi-Fi config, PSRAM build, GPIO0 flashing,
COM/tty ports, flash PWM, stream URLs and AI integration. Its commands run from the
separate `web-cam` clone. The camera gets its own DHCP address and does not use
MQTT_HOST; Python connects to `http://<ESP32_CAM_IP>:81/stream`.

## Linux setup

One-time core/library setup (same Arduino CLI commands also work in PowerShell):

```bash
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.12 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install "DHT sensor library@1.4.7" "Adafruit Unified Sensor@1.1.15" "PubSubClient@2.8" "ArduinoJson@7.4.3"
cp mqtt-firmware/config.example.h mqtt-firmware/config.h
```

Edit the private mqtt-firmware/config.h before compiling; do not overwrite existing config.h files.
Discover ports with `arduino-cli board list`. Examples below are **not guaranteed**
to be your ports. Close any serial monitor before uploading.

```bash
arduino-cli board list
arduino-cli compile --fqbn esp32:esp32:esp32:UploadSpeed=115200 mqtt-firmware
arduino-cli upload --fqbn esp32:esp32:esp32:UploadSpeed=115200 --port /dev/ttyUSB0 mqtt-firmware
arduino-cli monitor --port /dev/ttyUSB0 --config baudrate=115200
```

## Windows setup

Install Arduino CLI and the USB-UART driver for your board/adapter, then open a new
PowerShell so PATH is refreshed. One time:

```powershell
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.12 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install "DHT sensor library@1.4.7" "Adafruit Unified Sensor@1.1.15" "PubSubClient@2.8" "ArduinoJson@7.4.3"
Copy-Item mqtt-firmware/config.example.h mqtt-firmware/config.h
notepad mqtt-firmware/config.h
arduino-cli board list
```

Use the discovered **COM port**, not /dev/ttyUSB*. COM3 below is an example;
compare before/after plugging each board in or check Device Manager → Ports.
The board may show unknown while its serial port is still usable with explicit FQBN.

```powershell
arduino-cli compile --fqbn "esp32:esp32:esp32:UploadSpeed=115200" mqtt-firmware
arduino-cli upload --fqbn "esp32:esp32:esp32:UploadSpeed=115200" --port COM3 mqtt-firmware
arduino-cli monitor --port COM3 --config baudrate=115200
```

## Running

Flash only when firmware/network configuration changes. Each session, start Docker
and Mosquitto/Spring, then power the regular ESP32. The firmware connects Wi-Fi,
MQTT and NTP, publishes approximately every 5 seconds and reports connection status
on serial. DEVICE_ID must be in Spring's MQTT_DEVICES allowlist.

### MQTT contract and pump safety

| Topic | Purpose |
|---|---|
| smartphset/devices/{DEVICE_ID}/telemetry | Sensor JSON with device_id and captured_at UTC |
| smartphset/devices/{DEVICE_ID}/state | Reported fan/light/pump booleans and captured_at |
| smartphset/devices/{DEVICE_ID}/commands | Expiring command JSON |

Command shape: `{"cmd":"P0","command_id":"unique-id","expires_at":<future-unix-seconds>}`.
The expiry must be fresh and at most 15 seconds ahead; use Spring/Swagger to
create actual commands. P1/P0 = pump ON/OFF; F1/F0 = fan ON/OFF; L1/L0 = light
ON/OFF. Bare strings and legacy topic/boolean commands are rejected. Most recent
command ID per actuator is deduplicated. Commands/state are non-retained; firmware
uses QoS0 and does not restore old outputs on restart.

Pump auto-off is approximately **10 seconds**, with repeated ON not extending the
deadline. This is a cooperative loop, not a hardware safety interlock. Observed
Wi-Fi/MQTT loss and reconnect force outputs OFF. NTP must be synchronized; invalid
time rejects commands and suppresses timestamped telemetry/state. DHT failure
skips telemetry but does not prevent independently reported output state.

## Testing

Linux host policy tests:

```bash
c++ -std=c++17 -Wall -Wextra -Werror tests/policy_test.cpp -o /tmp/smart-things-policy-test
/tmp/smart-things-policy-test
git diff --check
```

Windows Developer PowerShell with MSVC:

```powershell
New-Item -ItemType Directory -Force build | Out-Null
cl /nologo /std:c++17 /EHsc /W4 /WX tests/policy_test.cpp /Fe:build/policy-test.exe /Fo:build/policy-test.obj
.\build\policy-test.exe
git diff --check
```

Or PowerShell with g++ on PATH:

```powershell
New-Item -ItemType Directory -Force build | Out-Null
g++ -std=c++17 -Wall -Wextra -Werror tests/policy_test.cpp -o build/policy-test.exe
.\build\policy-test.exe
```

Also run the regular ESP32 compile command for your platform above; host tests
alone do not compile Arduino APIs. Camera compilation is documented and validated
separately in [web-cam](https://github.com/smart-phset/web-cam).

Observe real MQTT from either shell without installing a host MQTT client:

```text
docker exec smartphset-mosquitto mosquitto_sub -h localhost -p 1883 -v -t smartphset/devices/grow-room-1/state -t smartphset/devices/grow-room-1/telemetry -t smartphset/devices/grow-room-1/commands
```

Stop subscription with Ctrl+C. Verify fresh timestamps and Spring GET
`/api/iot/devices/grow-room-1/state` using X-SmartPhset-IoT-Key (commands in API README).
With safe isolated loads, verify boot OFF, each command mapping, pump auto-off, disconnect/reboot OFF
and invalid/expired-command rejection. Finish all outputs OFF. Compiling is not
physical acceptance; see [hardware acceptance history](HARDWARE_ACCEPTANCE.md).
Real soil, fan/light and actual pump/load operation remain unverified.

## Common problems

- Wi-Fi works but MQTT does not: stale MQTT_HOST after moving networks is a known
  failure. Recheck laptop IPv4, Docker/Mosquitto, TCP1883 firewall and client isolation.
- No telemetry: inspect DHT wiring and NTP reachability. Floating soil readings
  are invalid even when numeric. Backend must allow the exact DEVICE_ID.
- Serial port unavailable: use board list, a data-capable cable, correct driver,
  Linux serial permissions, and close other monitors. Windows uses COMx.
- Upload hangs connecting: the regular ESP32 may need BOOT held/reset. Close
  serial monitors and use the detected port and 115200 upload speed.
- Brownouts or repeated resets: check power, USB cable and wiring. For camera
  flash-chip/stream/PSRAM issues, use the separate
  [camera troubleshooting guide](https://github.com/smart-phset/web-cam/blob/main/README.md#troubleshooting).
- GPIO ON reported but load does not run: reported GPIO state is not sensed load
  feedback. Check driver/polarity/supply safely; never bridge loads directly to GPIO.

## Git workflow

Start with a clean worktree (`git status`). Update the branch named in Clone before
creating your own feature branch. These commands work in Bash and PowerShell:

```text
git pull --ff-only
git switch -c feat/my-change
# Edit and run the checks above.
git status
git add README.md
# Add other intended files explicitly; never add private configuration.
git diff --cached
git commit -m "Describe the change"
git push -u origin feat/my-change
```

Open a Pull Request against the team's agreed integration branch. Do not push
teammate changes directly to main. Never use `git push --force` unless the team
explicitly agrees and understands why. If pull refuses, inspect local changes and
coordinate; do not discard work with a hard reset.

## Security notes

Never commit `.env`, `.env.local`, private `config.h`, Wi-Fi passwords, API keys,
DB passwords or auth secrets. Commit only safe example files. Inspect
`git diff --cached` before each commit; do not force-add ignored files. If a secret
was committed, rotate it and coordinate cleanup; deleting it does not revoke it.

## 0.4.0 — 2026-10-30

Reconciles four months of fixes made in a vendored copy of this component
inside a product repo, and fixes the packaging bug that stopped anyone
building the examples from a fresh clone.

### Fixed

- **The examples could not be built from a clone.** ESP-IDF names a `path:`
  dependency after its *directory*, not after the key in the manifest. The
  component sat at the repo root, so a clone (folder `hyperwisor_esp_idf`)
  produced `component 'hyperwisor' could not be found` — the first thing a new
  user would hit. The component now lives in `components/hyperwisor/`, which is
  the conventional layout and works whatever the clone is called.
- **The websocket ping was configured on the wrong layer.** `keep_alive_*` in
  `esp_websocket_client` are the TCP socket's SO_KEEPALIVE options, not the
  websocket ping. They were being set to 15s/3s/2, which declares a connection
  dead ~21s after it goes quiet if two probes are lost — ordinary on an
  intercontinental link. The websocket ping (`ping_interval_sec`) was never set
  at all. Both are now explicit and separate.
- **`pingpong_timeout_sec` is three ping periods, not one.** It was unset, so
  the feature was off; enabling it naively would terminate healthy connections
  on a single lost frame.
- **Destroying the client from its own task panicked the board.** Teardown is
  now deferred to the caller via `hyperwisor_ws_service()`, with
  `hyperwisor_ws_reconnect_due()` gating the backoff window.
- **The websocket connected before an IP was assigned.** The core now waits on
  `ip_ready` rather than `wifi_connected`.
- **The provisioning deep link was silently truncated.** `suffix[220]` could not
  hold a URL-encoded P-256 public key plus a device id; it is now 300.
- **The relay's application-level keepalive went unanswered.** The device now
  replies, which is what refreshes a NAT mapping — the reply travels outbound,
  and nothing the server sends can do that.

### Changed

- **`CONFIG_HYPERWISOR_WS_HOST` now defaults to the secure relay.** The previous
  default was retired on 2026-07-30 and has since been repurposed for an
  unrelated project, so anything left on it was connecting somewhere it had no
  business being.
- **Wi-Fi power save is an explicit Kconfig choice** (`HYPERWISOR_WIFI_POWER_SAVE`),
  defaulting to NONE. IDF defaults to `MIN_MODEM`, which is right on a battery
  and wrong on anything mains-powered: the station misses DTIM beacons on a busy
  band and disassociates (`wifi:bcn_timeout`), so the link drops every couple of
  minutes in a way that looks like a server fault. A battery node should choose
  MIN_MODEM deliberately rather than inherit it.

### Added

- `hyperwisor_ota_get_progress()` — percentage for a UI to display.

# Changelog

All notable changes to the Hyperwisor IoT component will be documented here.
The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/).

## [0.2.0] — 2026-07-03

### Added — HSC v1 (Hyperwisor Secure Channel)
Opt-in cryptographic device authentication so devices can't be impersonated or
hijacked. Existing firmware is unaffected unless it calls `hyperwisor_enable_security()`.

- `hyperwisor_hsc.{c,h}`: on-chip P-256 keypair (mbedTLS) generated at first boot
  and stored in NVS — the private key never leaves the device.
  `hyperwisor_hsc_get_public_key_b64()`, `hyperwisor_hsc_sign()`, and
  `hyperwisor_hsc_register()` (registers the public key via `relay-register-device`).
- WebSocket transport does the HSC challenge–response on connect and only reports
  connected after `auth_ok`.
- `hyperwisor_enable_security()` + `hyperwisor_get_public_key_b64()` in core.
- Point the device at your secured relay via `CONFIG_HYPERWISOR_WS_HOST`.
- `secure_hyperwisor` example.

## [0.1.0] — 2026-05-03

Initial public release. Extracted from the `hyperwisor_s3` reference project
into a standalone, registry-publishable ESP-IDF component.

### Added
- Umbrella header `hyperwisor.h` and Kconfig feature gates
  (`SYSTEM`, `NTP`, `HTTP`, `WIDGET`, `OTA`, `GPIO`, `DISPLAY`).
- WiFi STA + SoftAP fallback (`NIKOLAINDUSTRY_Setup-XXXX`).
- Secure WebSocket transport with cJSON command framing.
- Built-in commands: `SYSTEM`, `DEVICE_STATUS`, `OTA_UPDATE`, `GPIO_MANAGEMENT`.
- Board-port abstraction (`hyperwisor_port_t`) for GPIO whitelist + display.
- Bundled OTA partition CSVs for 4/8/16 MB flash.
- `hello_hyperwisor` example.

### Reliability
- WebSocket client is stopped during HTTPS OTA download to avoid
  TLS-stack contention; a `s_ota_in_progress` flag prevents the core
  task from racing the disconnect.
- OTA download progress is logged locally only (no WS sends mid-download).
- Failures are persisted in NVS (`ota_failed_reason`) and reported to
  the cloud once WebSocket reconnects.

### Targets
- ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C6, ESP32-P4
- IDF `>= 5.5, < 7.0`

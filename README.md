# TokenPulse

A small ESP32-S3 desk gadget that **keeps your Mac awake** and shows your
**Claude Code usage** on an amber-CRT-style dashboard.

- **Keep-awake** is fully on the board: it enumerates as a USB HID keyboard and
  taps `F15` every 30 s, so the Mac never idles to sleep. No host process needed
  for this — just keep the board plugged in.
- **Usage display**: a tiny Go tool hooks into Claude Code's status line, reads
  the 5h / 7-day usage numbers Claude Code already computes, and forwards them to
  the board over a USB serial link. Your terminal status line is left untouched.

```
Claude Code ──stdin──▶ tokenpulse statusline ──USB serial──▶ board (LCD)
                              └──stdout (your status line, unchanged)──▶ terminal
```

## Supported boards

One firmware tree, compile-time board selection:

| Board | Display | Orientation |
|---|---|---|
| **ESP32-S3-LCD-1.47B** (default) | ST7789 SPI, 172×320 | portrait |
| **ESP32-S3-Touch-LCD-4.3C** | RGB parallel, 800×480 | landscape |

Both connect over the ESP32-S3 **native USB** as a composite device:
a **CDC** serial port (receives usage snapshots) + an **HID** keyboard (F15 keep-awake).

## Repo layout

```
host/        Go tool — `tokenpulse` (statusline / setup / daemon-less)
firmware/    ESP-IDF v6.0.1 + LVGL project for both boards
```

## Quick start

### 1. Host tool

Install with Homebrew:

```bash
brew install yuchou87/tap/tokenpulse
```

Or build from source:

```bash
cd host && go build -o tokenpulse .
install tokenpulse /usr/local/bin/        # or: go install (then it's $GOPATH/bin/tokenpulse)
```

Or grab a prebuilt binary from [releases](https://github.com/yuchou87/tokenpulse/releases).

Register it as your Claude Code status line — the tool does it for you (detects
the serial port, wraps your existing status line as passthrough, backs up
`settings.json`):

```bash
tokenpulse setup        # prompts first; --yes to skip, --serial <dev> to override
```

`setup` is idempotent. To register manually instead, set your
`~/.claude/settings.json` `statusLine.command` to `tokenpulse statusline`
(and point `TOKENPULSE_INNER` at your previous status-line command).

Prebuilt host binaries (macOS/Linux) are attached to each
[release](https://github.com/yuchou87/tokenpulse/releases).

### 2. Firmware

```bash
source ~/.espressif/tools/activate_idf_v6.0.1.sh

# ESP32-S3-LCD-1.47B (default)
cd firmware && idf.py -p <PORT> flash monitor

# ESP32-S3-Touch-LCD-4.3C
idf.py -B build_43c -DSDKCONFIG=sdkconfig.43c \
  -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.43c.defaults" -p <PORT> flash monitor
```

Once the app is running it owns the native USB, so re-flashing needs download
mode: **hold BOOT, tap RST, release BOOT**, then flash. Details + gotchas in
[firmware/README.md](firmware/README.md).

A `Makefile` wraps both: `make host` / `make fw-147b` / `make fw-43c` /
`make fw-flash-147b PORT=…` / `make fw-flash-43c PORT=…`.

## How it behaves

- **Push, not poll.** The board updates only when Claude Code refreshes its
  status line (and that payload includes rate limits). There's no background
  polling of Anthropic.
- **Holds the last reading.** When no fresh data arrives, the board keeps showing
  the last real values (it does not blank out). On a cold boot it shows `--`
  until the first snapshot.
- **Fail-safe host.** If the board is unplugged, the serial write fails, or the
  input can't be parsed, your terminal status line still renders normally — the
  serial push is a best-effort side effect.

### Host configuration (environment)

| Variable | Default | Purpose |
|---|---|---|
| `TOKENPULSE_INNER` | `bash ~/.claude/statusline-command.sh` | Your existing status-line command; its stdout is passed through verbatim. |
| `TOKENPULSE_SERIAL` | autodetect (prefers the TokenPulse board, else first `/dev/cu.usbmodem*`) | Serial device of the board. |
| `TOKENPULSE_BAUD` | `115200` | Baud (nominal; irrelevant for USB-CDC). |

### Host subcommands

| Command | What it does |
|---|---|
| `tokenpulse statusline` | The status-line hook: read stdin → push snapshot to the board → pass your status line through to stdout. |
| `tokenpulse setup` | Register into `~/.claude/settings.json` (backup + idempotent). |
| `tokenpulse version` | Print the version. |

## Status & roadmap

- **Done**: USB-wired keep-awake (F15), Claude Code 5h / 7-day usage + session
  cost, both boards, host `setup`.
- **Deferred**: a resident daemon that fetches usage independently of Claude Code
  (so the board updates even when Claude Code is closed); Codex as a second
  provider; Bluetooth.

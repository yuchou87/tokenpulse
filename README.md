# TokenPulse

A small desk gadget that keeps your Mac awake and shows your Claude Code usage
on a 1.47″ amber-CRT-style display.

- **Board** — ESP32-S3 with a 172×320 ST7789 LCD. Connected to the Mac over USB,
  it acts as a composite USB device:
  - a **CDC** serial port that receives usage snapshots, and
  - an **HID** keyboard that taps `F15` every 30 s so the Mac never idles to sleep.
- **Host** — a tiny Go tool that hooks into Claude Code's status line, reads the
  usage numbers Claude Code already computes, and forwards a one-line JSON
  snapshot to the board over serial. It never changes what your status line shows.

```
Claude Code ──stdin──▶ tokenpulse statusline ──serial──▶ board (LCD)
                              │
                              └──stdout (unchanged status line)──▶ terminal
```

## Repo layout

```
host/       Go tool — `tokenpulse statusline`
firmware/   ESP-IDF v6.0.1 + LVGL project for the ESP32-S3 board
```

## Host: build & install

```bash
cd host
go build -o tokenpulse .
# move it onto your PATH, e.g.
install tokenpulse /usr/local/bin/
```

Register it as the Claude Code status-line command in `~/.claude/settings.json`:

```json
{ "statusLine": { "type": "command", "command": "tokenpulse statusline" } }
```

`tokenpulse statusline` passes your existing status line through untouched — so
point it at whatever you ran before via `TOKENPULSE_INNER`.

### Configuration (environment)

| Variable | Default | Purpose |
|---|---|---|
| `TOKENPULSE_INNER` | `bash ~/.claude/statusline-command.sh` | Your existing status-line command; its stdout is passed through verbatim. |
| `TOKENPULSE_SERIAL` | autodetect `/dev/cu.usbmodem*` | Serial device of the board. |
| `TOKENPULSE_BAUD` | `115200` | Baud (nominal; irrelevant for USB-CDC). |

The tool is fail-safe by design: if the board is unplugged, the serial write
fails, or the JSON can't be parsed, your status line still renders normally.

## Firmware: build & flash

```bash
source ~/.espressif/tools/activate_idf_v6.0.1.sh
cd firmware
idf.py set-target esp32s3
idf.py build flash monitor
```

Once the app owns the USB port, re-flashing needs download mode: **hold BOOT,
tap RST**, then run `idf.py flash`. See [firmware/README.md](firmware/README.md).

## Status

MVP: USB-wired, Claude Code 5h / 7-day usage, F15 keep-awake. See the firmware
and host source for details.

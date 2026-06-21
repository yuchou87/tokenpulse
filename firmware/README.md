# TokenPulse Firmware

ESP-IDF v6.0.1 project for ESP32-S3-1.47B (172×320 ST7789 + USB CDC/HID).

## Re-flashing after TinyUSB starts

Once the app is running, TinyUSB takes over the native USB port (it enumerates as
`/dev/cu.usbmodemTP_00011`), so a plain `idf.py flash` can't auto-reset into the
ROM bootloader. Enter download mode manually:

1. **Hold BOOT**, **tap RST**, **release BOOT**.
2. The board re-enumerates as the ROM USB-Serial/JTAG port (`/dev/cu.usbmodem<N>`,
   e.g. `usbmodem1101`) and waits for download.
3. Flash it, pointing at that port:
   `idf.py -p /dev/cu.usbmodem<N> flash`
   (for 4.3C add `-B build_43c`).

### Gotchas (learned during bring-up)

- **After flashing, the board may stay in download mode.** esptool's "Hard
  resetting via RTS" sometimes lands back in download (`boot:0x0 DOWNLOAD`) on
  these native-USB boards. Just **tap RST once (no BOOT)** to boot the app.
- **Don't open the serial port to "check" it.** Opening `/dev/cu.*` (cat,
  screen, `idf.py monitor`) asserts DTR and resets the board. Use
  `ioreg -p IOUSB -l | grep "USB Product Name"` to see whether it enumerated as
  `TokenPulse` (app running) vs `USB JTAG_serial debug unit` (ROM) without
  resetting it.

## 目标板选择

固件支持两块板,默认 1.47B：

| 板 | 屏 | 构建命令 |
|---|---|---|
| ESP32-S3-LCD-1.47B（默认） | ST7789 SPI 172×320 | `idf.py build` |
| ESP32-S3-Touch-LCD-4.3C | RGB 800×480 | `idf.py -B build_43c -DSDKCONFIG=sdkconfig.43c -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.43c.defaults" build` |

也可 `idf.py menuconfig` → TokenPulse → Target board 切换。
4.3C 烧录（首次需带上 defaults 生成正确的 sdkconfig.43c）：`idf.py -B build_43c -DSDKCONFIG=sdkconfig.43c -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.43c.defaults" flash monitor`；app 占用 USB 后重烧同样按住 BOOT 点 RST。

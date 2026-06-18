# TokenPulse Firmware

ESP-IDF v6.0.1 project for ESP32-S3-1.47B (172×320 ST7789 + USB CDC/HID).

## Re-flashing after TinyUSB starts

Once the app is running, TinyUSB takes over the USB port. A plain `idf.py flash` will fail to connect. To re-enter download mode:

1. Hold the **BOOT** button.
2. Tap **RST**.
3. Release **BOOT**.
4. Run `idf.py flash` — esptool will find the device in download mode.

## 目标板选择

固件支持两块板,默认 1.47B：

| 板 | 屏 | 构建命令 |
|---|---|---|
| ESP32-S3-LCD-1.47B（默认） | ST7789 SPI 172×320 | `idf.py build` |
| ESP32-S3-Touch-LCD-4.3C | RGB 800×480 | `idf.py -B build_43c -DSDKCONFIG=sdkconfig.43c -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.43c.defaults" build` |

也可 `idf.py menuconfig` → TokenPulse → Target board 切换。
4.3C 烧录：`idf.py -B build_43c -DSDKCONFIG=sdkconfig.43c flash monitor`；app 占用 USB 后重烧同样按住 BOOT 点 RST。

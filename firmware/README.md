# TokenPulse Firmware

ESP-IDF v6.0.1 project for ESP32-S3-1.47B (172×320 ST7789 + USB CDC/HID).

## Re-flashing after TinyUSB starts

Once the app is running, TinyUSB takes over the USB port. A plain `idf.py flash` will fail to connect. To re-enter download mode:

1. Hold the **BOOT** button.
2. Tap **RST**.
3. Release **BOOT**.
4. Run `idf.py flash` — esptool will find the device in download mode.

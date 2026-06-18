#pragma once

/**
 * keepawake_start — start a periodic 30-second timer that sends F15 (keycode 0x68)
 * via USB HID to prevent the host from entering sleep.
 *
 * Must be called after usb_init().
 * Uses esp_timer; the callback runs in the esp_timer task context which supports
 * FreeRTOS primitives such as vTaskDelay (it is NOT an ISR).
 */
void keepawake_start(void);

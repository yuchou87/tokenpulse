#pragma once

#include <stdint.h>

/**
 * usb_init — install TinyUSB driver (CDC + HID composite)
 *
 * CDC interface receives JSON snapshots from the host.
 * HID interface sends keyboard reports (F15 keep-awake).
 */
void usb_init(void);

/**
 * usb_hid_send_key — send a key-down + key-up report for the given HID keycode.
 *
 * Must be called from a FreeRTOS task (uses vTaskDelay between down/up).
 * Safe to call even if HID is not yet enumerated (logs a warning and returns).
 */
void usb_hid_send_key(uint8_t keycode);

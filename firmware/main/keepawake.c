/*
 * keepawake.c — periodic USB HID F15 to prevent host sleep
 *
 * Starts a 30-second repeating esp_timer.  On each tick it calls
 * usb_hid_send_key(F15_KEYCODE) which issues a key-down + 20 ms delay + key-up.
 *
 * NOTE: esp_timer callbacks run in the esp_timer FreeRTOS task, NOT in an ISR,
 * so calling vTaskDelay (used inside usb_hid_send_key) is safe.
 */

#include "keepawake.h"
#include "usb.h"

#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "keepawake";

#define F15_KEYCODE     0x68u   /* HID Usage: Keyboard F15 */
#define INTERVAL_US     (30LL * 1000 * 1000)   /* 30 seconds */

static void keepawake_cb(void *arg)
{
    (void)arg;
    ESP_LOGD(TAG, "sending F15");
    usb_hid_send_key(F15_KEYCODE);
}

void keepawake_start(void)
{
    const esp_timer_create_args_t args = {
        .callback = keepawake_cb,
        .arg      = NULL,
        .name     = "keepawake_f15",
    };
    esp_timer_handle_t timer;
    ESP_ERROR_CHECK(esp_timer_create(&args, &timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer, INTERVAL_US));
    ESP_LOGI(TAG, "F15 keep-awake started (every 30 s)");
}

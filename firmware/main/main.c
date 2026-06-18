#include "display.h"
#include "esp_lvgl_port.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ui.h"
#include "comm.h"
#include "usb.h"
#include "keepawake.h"

/* Watchdog task: runs on Core1 at 5-second intervals.
 * Checks if comm_last_rx_ms() is older than 60s; if so, sets stale=true
 * and calls ui_update to grey-out all data fields.
 *
 * Cross-core safety: comm_last_rx_ms() returns volatile uint32_t
 * (atomic on ESP32-S3 Xtensa LX7), so no lock needed around that read. */
static void watchdog_task(void *arg)
{
    while (1) {
        uint32_t now_ms = (uint32_t)(esp_timer_get_time() / 1000);
        uint32_t last   = comm_last_rx_ms();
        /* Unsigned subtraction handles uint32_t wrap correctly */
        if ((now_ms - last) > 60000U) {
            ui_state_t st = { .stale = true };
            lvgl_port_lock(0);
            ui_update(&st);
            lvgl_port_unlock();
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void)
{
    /* Display + LVGL */
    esp_lcd_panel_handle_t panel = display_init();
    lv_display_t *disp = display_lvgl_init(panel);

    /* Build static UI widget tree (no live data yet) */
    lvgl_port_lock(0);
    ui_build(disp);
    lvgl_port_unlock();

    /* USB CDC + HID composite — CDC receives JSON snapshots, HID sends F15 */
    usb_init();

    /* Start 30-second F15 keep-awake timer */
    keepawake_start();

    /* Stale-data watchdog: grey-out UI if no snapshot received for >60s.
     * Priority 3, default-core (Core1 on S3, opposite to TinyUSB on Core0). */
    xTaskCreate(watchdog_task, "wd", 4096, NULL, 3, NULL);
}

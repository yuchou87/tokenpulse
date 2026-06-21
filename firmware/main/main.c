#include "display.h"
#include "esp_lvgl_port.h"
#include "ui.h"
#include "usb.h"
#include "keepawake.h"

void app_main(void)
{
    /* Display + LVGL */
    lv_display_t *disp = board_display_init();

    /* Build static UI widget tree (no live data yet) */
    lvgl_port_lock(0);
    ui_build(disp);
    lvgl_port_unlock();
    board_display_backlight_on();

    /* USB CDC + HID composite — CDC receives JSON snapshots, HID sends F15 */
    usb_init();

    /* Start 30-second F15 keep-awake timer */
    keepawake_start();

    /* No stale watchdog: the board holds the last received snapshot until a new
     * one arrives (the UI only updates when comm gets a fresh line). On boot it
     * shows the "--" placeholders until the first snapshot. */
}

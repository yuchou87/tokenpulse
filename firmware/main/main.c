#include "display.h"
#include "esp_lvgl_port.h"
#include "ui.h"
#include "usb.h"
#include "keepawake.h"

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
}

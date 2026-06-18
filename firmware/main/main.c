#include "display.h"
#include "esp_lvgl_port.h"
#include "ui.h"

void app_main(void)
{
    esp_lcd_panel_handle_t panel = display_init();
    lv_display_t *disp = display_lvgl_init(panel);

    lvgl_port_lock(0);
    ui_build(disp);

    ui_state_t demo = {
        .five_pct     = 37,
        .week_pct     = 18,
        .five_reset_s = 8040,   /* 02:14:00 */
        .session_usd  = 0.29,
        .stale        = false,
    };
    ui_update(&demo);

    lvgl_port_unlock();
}

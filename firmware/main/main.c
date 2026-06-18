#include "display.h"
#include "esp_lvgl_port.h"

void app_main(void) {
    esp_lcd_panel_handle_t panel = display_init();
    lv_display_t *disp = display_lvgl_init(panel);

    lvgl_port_lock(0);
    lv_obj_t *l = lv_label_create(lv_display_get_screen_active(disp));
    lv_label_set_text(l, "TOKENPULSE");
    lv_obj_center(l);
    lvgl_port_unlock();
}

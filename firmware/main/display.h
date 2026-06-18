#pragma once
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"
#include "esp_lvgl_port.h"

esp_lcd_panel_handle_t display_init(void);
esp_lcd_panel_io_handle_t display_io(void);
lv_display_t *display_lvgl_init(esp_lcd_panel_handle_t panel);

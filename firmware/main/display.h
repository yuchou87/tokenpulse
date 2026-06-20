#pragma once
#include "esp_lvgl_port.h"

// 建 panel + LVGL,返回 display;背光此时仍关闭。
lv_display_t *board_display_init(void);
// 首帧渲染后打开背光(避免上电花屏)。
void board_display_backlight_on(void);

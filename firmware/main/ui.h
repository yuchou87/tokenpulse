#pragma once
#include "esp_lvgl_port.h"

typedef struct {
    int five_pct;
    int week_pct;
    long five_reset_s;
    double session_usd;
    bool stale;
} ui_state_t;

void ui_build(lv_display_t *disp);
void ui_update(const ui_state_t *st);

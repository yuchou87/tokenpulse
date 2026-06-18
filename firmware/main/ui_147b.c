#include "ui.h"
#include <stdio.h>

#define AMBER 0xFFB000
#define DIM   0xC8870F

static lv_obj_t *lbl_5h_pct, *lbl_reset, *lbl_7d, *bar5, *bar7, *lbl_bottom;

static lv_obj_t *mklbl(lv_obj_t *p, const char *txt, int y, int size, uint32_t color)
{
    (void)size; /* size param reserved for future font switching */
    lv_obj_t *l = lv_label_create(p);
    lv_label_set_text(l, txt);
    lv_obj_set_style_text_color(l, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(l, &lv_font_montserrat_14, 0);
    lv_obj_set_pos(l, 8, y);
    return l;
}

void ui_build(lv_display_t *disp)
{
    lv_obj_t *scr = lv_display_get_screen_active(disp);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0B0A07), 0);

    mklbl(scr, "TOKENPULSE",  6,  14, AMBER);
    mklbl(scr, "CLAUDE CODE", 40, 14, AMBER);
    mklbl(scr, "5H WINDOW",   64, 14, DIM);

    lbl_5h_pct = mklbl(scr, "--%", 82, 28, AMBER);

    bar5 = lv_bar_create(scr);
    lv_obj_set_size(bar5, 156, 12);
    lv_obj_set_pos(bar5, 8, 120);
    lv_obj_set_style_bg_color(bar5, lv_color_hex(0x5A3F08), 0);
    lv_obj_set_style_bg_color(bar5, lv_color_hex(AMBER), LV_PART_INDICATOR);

    lbl_reset = mklbl(scr, "RESET --:--:--", 138, 14, DIM);
    lbl_7d    = mklbl(scr, "7-DAY  --%",     170, 14, DIM);

    bar7 = lv_bar_create(scr);
    lv_obj_set_size(bar7, 156, 8);
    lv_obj_set_pos(bar7, 8, 192);
    lv_obj_set_style_bg_color(bar7, lv_color_hex(0x5A3F08), 0);
    lv_obj_set_style_bg_color(bar7, lv_color_hex(AMBER), LV_PART_INDICATOR);

    lbl_bottom = mklbl(scr, "* RUNNING", 296, 14, AMBER);
}

void ui_update(const ui_state_t *st)
{
    char buf[32];
    uint32_t c = st->stale ? 0x9C6A0C : AMBER;

    snprintf(buf, sizeof buf, "%d%%", st->five_pct);
    lv_label_set_text(lbl_5h_pct, st->stale ? "--%" : buf);
    lv_obj_set_style_text_color(lbl_5h_pct, lv_color_hex(c), 0);
    lv_bar_set_value(bar5, st->five_pct, LV_ANIM_OFF);

    long s = st->five_reset_s;
    snprintf(buf, sizeof buf, "RESET %02ld:%02ld:%02ld", s / 3600, (s % 3600) / 60, s % 60);
    lv_label_set_text(lbl_reset, st->stale ? "RESET --:--:--" : buf);

    snprintf(buf, sizeof buf, "7-DAY  %d%%", st->week_pct);
    lv_label_set_text(lbl_7d, st->stale ? "7-DAY  --%" : buf);
    lv_bar_set_value(bar7, st->week_pct, LV_ANIM_OFF);

    (void)lbl_bottom; /* bottom label stays static in ui_update */
}

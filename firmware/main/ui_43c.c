#include "ui.h"
#include <stdio.h>

#define AMBER 0xFFB000
#define DIM   0xC8870F
#define EMPTY 0x5A3F08
#define STALE 0x9C6A0C
#define BG    0x0B0A07

static lv_obj_t *lbl_5h_pct, *lbl_5h_reset, *bar5;
static lv_obj_t *lbl_7d_pct, *bar7;
static lv_obj_t *lbl_session, *lbl_state;

static lv_obj_t *mklbl(lv_obj_t *p, const char *txt, int x, int y,
                       const lv_font_t *font, uint32_t color)
{
    lv_obj_t *l = lv_label_create(p);
    lv_label_set_text(l, txt);
    lv_obj_set_style_text_color(l, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(l, font, 0);
    lv_obj_set_pos(l, x, y);
    return l;
}

static lv_obj_t *mkbar(lv_obj_t *p, int x, int y, int w, int h)
{
    lv_obj_t *b = lv_bar_create(p);
    lv_obj_set_size(b, w, h);
    lv_obj_set_pos(b, x, y);
    lv_obj_set_style_bg_color(b, lv_color_hex(EMPTY), 0);
    lv_obj_set_style_bg_color(b, lv_color_hex(AMBER), LV_PART_INDICATOR);
    lv_bar_set_range(b, 0, 100);
    return b;
}

void ui_build(lv_display_t *disp)
{
    lv_obj_t *scr = lv_display_get_screen_active(disp);
    lv_obj_set_style_bg_color(scr, lv_color_hex(BG), 0);

    // 顶栏
    mklbl(scr, "TOKENPULSE",  24, 18, &lv_font_montserrat_28, AMBER);
    mklbl(scr, "CLAUDE CODE",  24, 56, &lv_font_montserrat_14, DIM);
    lbl_state = mklbl(scr, "* ONLINE", 640, 24, &lv_font_montserrat_14, AMBER);
    mklbl(scr, "================================================", 24, 78,
          &lv_font_montserrat_14, DIM);

    // 左面板: 5H WINDOW
    mklbl(scr, "5H WINDOW", 40, 110, &lv_font_montserrat_28, DIM);
    lbl_5h_pct = mklbl(scr, "--%", 40, 150, &lv_font_montserrat_48, AMBER);
    bar5 = mkbar(scr, 40, 240, 340, 22);
    lbl_5h_reset = mklbl(scr, "RESET --:--:--", 40, 280, &lv_font_montserrat_28, DIM);

    // 右面板: 7-DAY
    mklbl(scr, "7-DAY", 440, 110, &lv_font_montserrat_28, DIM);
    lbl_7d_pct = mklbl(scr, "--%", 440, 150, &lv_font_montserrat_48, AMBER);
    bar7 = mkbar(scr, 440, 240, 340, 22);

    // 底栏: SESSION 花费 + 分隔
    mklbl(scr, "------------------------------------------------", 24, 380,
          &lv_font_montserrat_14, DIM);
    lbl_session = mklbl(scr, "SESSION $--.--", 40, 410, &lv_font_montserrat_28, AMBER);
    mklbl(scr, "* RUNNING", 640, 420, &lv_font_montserrat_14, AMBER);
}

void ui_update(const ui_state_t *st)
{
    char buf[40];
    uint32_t c = st->stale ? STALE : AMBER;

    // 5H
    snprintf(buf, sizeof buf, "%d%%", st->five_pct);
    lv_label_set_text(lbl_5h_pct, st->stale ? "--%" : buf);
    lv_obj_set_style_text_color(lbl_5h_pct, lv_color_hex(c), 0);
    lv_bar_set_value(bar5, st->stale ? 0 : st->five_pct, LV_ANIM_OFF);

    long s = st->five_reset_s;
    snprintf(buf, sizeof buf, "RESET %02ld:%02ld:%02ld",
             s / 3600, (s % 3600) / 60, s % 60);
    lv_label_set_text(lbl_5h_reset, st->stale ? "RESET --:--:--" : buf);

    // 7D
    snprintf(buf, sizeof buf, "%d%%", st->week_pct);
    lv_label_set_text(lbl_7d_pct, st->stale ? "--%" : buf);
    lv_obj_set_style_text_color(lbl_7d_pct, lv_color_hex(c), 0);
    lv_bar_set_value(bar7, st->stale ? 0 : st->week_pct, LV_ANIM_OFF);

    // SESSION 花费
    if (st->stale) {
        lv_label_set_text(lbl_session, "SESSION $--.--");
    } else {
        snprintf(buf, sizeof buf, "SESSION $%.2f", st->session_usd);
        lv_label_set_text(lbl_session, buf);
    }

    // 状态行: stale 时灰显
    lv_obj_set_style_text_color(lbl_state, lv_color_hex(c), 0);
    lv_label_set_text(lbl_state, st->stale ? "* STALE" : "* ONLINE");
}

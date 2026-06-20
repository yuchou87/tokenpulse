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

// Thin horizontal rule (cleaner + spans exactly, vs a row of '=' characters).
static void mksep(lv_obj_t *p, int x, int y, int w, uint32_t color)
{
    lv_obj_t *o = lv_obj_create(p);
    lv_obj_remove_style_all(o);
    lv_obj_set_size(o, w, 2);
    lv_obj_set_pos(o, x, y);
    lv_obj_set_style_bg_color(o, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
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

    // Header
    mklbl(scr, "TOKENPULSE",  32, 18, &lv_font_montserrat_28, AMBER);
    mklbl(scr, "CLAUDE CODE", 32, 56, &lv_font_montserrat_14, DIM);
    lbl_state = mklbl(scr, "* ONLINE", 648, 26, &lv_font_montserrat_14, AMBER);
    mksep(scr, 32, 88, 736, DIM);

    // Left column — 5H WINDOW (primary metric)
    mklbl(scr, "5H WINDOW", 40, 120, &lv_font_montserrat_28, DIM);
    lbl_5h_pct = mklbl(scr, "--%", 40, 158, &lv_font_montserrat_48, AMBER);
    bar5 = mkbar(scr, 40, 252, 360, 30);
    lbl_5h_reset = mklbl(scr, "RESET --:--:--", 40, 300, &lv_font_montserrat_28, DIM);

    // Right column — 7-DAY (secondary, smaller)
    mklbl(scr, "7-DAY", 440, 120, &lv_font_montserrat_28, DIM);
    lbl_7d_pct = mklbl(scr, "--%", 440, 160, &lv_font_montserrat_28, AMBER);
    bar7 = mkbar(scr, 440, 210, 320, 18);

    // Footer — session cost
    mksep(scr, 32, 372, 736, DIM);
    lbl_session = mklbl(scr, "SESSION $--.--", 40, 398, &lv_font_montserrat_28, AMBER);
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

    // Session cost
    if (st->stale) {
        lv_label_set_text(lbl_session, "SESSION $--.--");
    } else {
        snprintf(buf, sizeof buf, "SESSION $%.2f", st->session_usd);
        lv_label_set_text(lbl_session, buf);
    }

    // Status (single indicator; greyed when stale)
    lv_obj_set_style_text_color(lbl_state, lv_color_hex(c), 0);
    lv_label_set_text(lbl_state, st->stale ? "* STALE" : "* ONLINE");
}

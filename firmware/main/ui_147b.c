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

// Thin horizontal rule spanning the column.
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

// Portrait 172x320 — mirrors the 4.3C dashboard: 5H hero, thicker bars,
// drawn separators, session line, single ONLINE/STALE status.
void ui_build(lv_display_t *disp)
{
    lv_obj_t *scr = lv_display_get_screen_active(disp);
    lv_obj_set_style_bg_color(scr, lv_color_hex(BG), 0);

    // Header
    mklbl(scr, "TOKENPULSE",  8,  8, &lv_font_montserrat_14, AMBER);
    mklbl(scr, "CLAUDE CODE", 8, 26, &lv_font_montserrat_14, DIM);
    mksep(scr, 8, 48, 156, DIM);

    // 5H WINDOW (hero)
    mklbl(scr, "5H WINDOW", 8, 58, &lv_font_montserrat_14, DIM);
    lbl_5h_pct = mklbl(scr, "--%", 8, 78, &lv_font_montserrat_28, AMBER);
    bar5 = mkbar(scr, 8, 124, 156, 18);
    lbl_5h_reset = mklbl(scr, "RESET --:--:--", 8, 150, &lv_font_montserrat_14, DIM);
    mksep(scr, 8, 176, 156, DIM);

    // 7-DAY (secondary, inline)
    mklbl(scr, "7-DAY", 8, 188, &lv_font_montserrat_14, DIM);
    lbl_7d_pct = mklbl(scr, "--%", 110, 188, &lv_font_montserrat_14, AMBER);
    bar7 = mkbar(scr, 8, 214, 156, 14);
    mksep(scr, 8, 238, 156, DIM);

    // Footer
    lbl_session = mklbl(scr, "SESSION $--.--", 8, 250, &lv_font_montserrat_14, AMBER);
    lbl_state = mklbl(scr, "* ONLINE", 8, 290, &lv_font_montserrat_14, AMBER);
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

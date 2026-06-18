/*
 * display_43c.c — ESP32-S3-Touch-LCD-4.3C display driver
 *
 * API corrections vs plan:
 * - Uses old I2C driver (i2c_param_config/i2c_driver_install) because
 *   espressif/esp32_io_expander v1.x expects i2c_port_t, not i2c_master_bus_handle_t.
 * - esp_lcd_rgb_panel_config_t has no bits_per_pixel in IDF v6.0.1;
 *   uses in_color_format/out_color_format = LCD_COLOR_FMT_RGB565 instead.
 * - Component is espressif/esp32_io_expander (not esp_io_expander_ch422g).
 */
#include "display.h"
#include "board.h"
#include "driver/i2c.h"
#include "esp_io_expander_ch422g.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lvgl_port.h"

static esp_io_expander_handle_t g_exp = NULL;

lv_display_t *board_display_init(void)
{
    // 1. I2C init (legacy driver, required by esp32_io_expander v1.x)
    i2c_config_t i2c_cfg = {
        .mode             = I2C_MODE_MASTER,
        .sda_io_num       = I2C_SDA,
        .scl_io_num       = I2C_SCL,
        .sda_pullup_en    = GPIO_PULLUP_ENABLE,
        .scl_pullup_en    = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_HZ,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_cfg));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));

    // 2. CH422G IO expander: EXIO2 = backlight, keep off initially
    ESP_ERROR_CHECK(esp_io_expander_new_i2c_ch422g(
        I2C_NUM_0, ESP_IO_EXPANDER_I2C_CH422G_ADDRESS, &g_exp));
    ESP_ERROR_CHECK(esp_io_expander_set_dir(g_exp, CH422G_BL_PIN, IO_EXPANDER_OUTPUT));
    ESP_ERROR_CHECK(esp_io_expander_set_level(g_exp, CH422G_BL_PIN, 0));

    // 3. RGB panel (IDF v6.0.1: use in_color_format/out_color_format, no bits_per_pixel)
    esp_lcd_rgb_panel_config_t rgb = {
        .clk_src           = LCD_CLK_SRC_DEFAULT,
        .timings = {
            .pclk_hz           = LCD_PCLK_HZ,
            .h_res             = LCD_H_RES,
            .v_res             = LCD_V_RES,
            .hsync_pulse_width = LCD_HSYNC_PULSE,
            .hsync_back_porch  = LCD_HSYNC_BACK,
            .hsync_front_porch = LCD_HSYNC_FRONT,
            .vsync_pulse_width = LCD_VSYNC_PULSE,
            .vsync_back_porch  = LCD_VSYNC_BACK,
            .vsync_front_porch = LCD_VSYNC_FRONT,
            .flags.pclk_active_neg = 1,
        },
        .data_width        = 16,
        .in_color_format   = LCD_COLOR_FMT_RGB565,
        .out_color_format  = LCD_COLOR_FMT_RGB565,
        .hsync_gpio_num    = LCD_HSYNC,
        .vsync_gpio_num    = LCD_VSYNC,
        .de_gpio_num       = LCD_DE,
        .pclk_gpio_num     = LCD_PCLK,
        .disp_gpio_num     = -1,
        .data_gpio_nums    = LCD_DATA_GPIOS,
        .num_fbs           = 2,
        .bounce_buffer_size_px = LCD_H_RES * 10,
        .flags             = { .fb_in_psram = 1 },
    };
    esp_lcd_panel_handle_t panel = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&rgb, &panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel));

    // 4. LVGL port + RGB display (handles vsync / anti-tearing internally)
    const lvgl_port_cfg_t pc = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&pc));
    const lvgl_port_display_cfg_t dc = {
        .panel_handle  = panel,
        .buffer_size   = LCD_H_RES * LCD_V_RES,
        .double_buffer = true,
        .hres          = LCD_H_RES,
        .vres          = LCD_V_RES,
        .color_format  = LV_COLOR_FORMAT_RGB565,
        .flags         = { .buff_spiram = true },
    };
    const lvgl_port_display_rgb_cfg_t rgbc = {
        .flags = { .bb_mode = true, .avoid_tearing = true },
    };
    return lvgl_port_add_disp_rgb(&dc, &rgbc);
}

void board_display_backlight_on(void)
{
    if (g_exp) {
        esp_io_expander_set_level(g_exp, CH422G_BL_PIN, 1);
    }
}

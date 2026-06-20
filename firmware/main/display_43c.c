/*
 * display_43c.c — ESP32-S3-Touch-LCD-4.3C display driver
 *
 * IO expander note:
 * This board's "CH422G" footprint is actually a custom MCU-based IO expander
 * (schematic U10 exposes SWDIO + PWM + ADC pins — a real CH422G has none of
 * these). It speaks a simple register-pointer protocol at I2C 0x24:
 *   reg 0x02 = direction mask (1=output per bit), reg 0x03 = output latch byte.
 * The Espressif esp_io_expander_ch422g driver uses the real-CH422G multi-address
 * protocol (0x23/0x38/...) and NACKs on this board -> abort. So we talk to the
 * expander directly (verified working per Waveshare's reference io_extension).
 *
 * Other IDF v6.0.1 notes:
 * - esp_lcd_rgb_panel_config_t has no bits_per_pixel; use in/out_color_format.
 */
#include "display.h"
#include "board.h"
#include "driver/i2c.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lvgl_port.h"

#define IOEXP_ADDR       0x24   // I2C slave of the IO expander
#define IOEXP_REG_MODE   0x02   // 8-bit direction mask, 1 = output
#define IOEXP_REG_OUTPUT 0x03   // 8-bit output latch (write-only -> keep a shadow)
#define IOEXP_BL_BIT     2      // EXIO2 = LCD backlight (high = on)

// Shadow of the output latch. 0xF7 is Waveshare's proven default (PA/bit3 low,
// rest high); we additionally clear the backlight bit so the screen stays dark
// until the first frame is rendered.
static uint8_t s_ioexp_out = 0xF7 & ~(1 << IOEXP_BL_BIT);

static esp_err_t ioexp_write(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return i2c_master_write_to_device(I2C_NUM_0, IOEXP_ADDR, buf, sizeof buf,
                                      pdMS_TO_TICKS(100));
}

lv_display_t *board_display_init(void)
{
    // 1. I2C (legacy driver)
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

    // 2. IO expander: all pins output, backlight off (until first frame)
    ESP_ERROR_CHECK(ioexp_write(IOEXP_REG_MODE, 0xFF));
    ESP_ERROR_CHECK(ioexp_write(IOEXP_REG_OUTPUT, s_ioexp_out));

    // 3. RGB panel (IDF v6.0.1: in/out_color_format, no bits_per_pixel)
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
    s_ioexp_out |= (1 << IOEXP_BL_BIT);
    ioexp_write(IOEXP_REG_OUTPUT, s_ioexp_out);
}

#pragma once
// ESP32-S3-Touch-LCD-4.3C — RGB565 800x480 + CH422G(I2C) backlight
#define I2C_SDA      8
#define I2C_SCL      9
#define I2C_HZ       400000

#define LCD_HSYNC    46
#define LCD_VSYNC    3
#define LCD_DE       5
#define LCD_PCLK     7
// D0..D15 = B0-4, G0-5, R0-4
#define LCD_DATA_GPIOS {14,38,18,17,10, 39,0,45,48,47,21, 1,2,42,41,40}

#define LCD_H_RES    800
#define LCD_V_RES    480
#define LCD_PCLK_HZ  (16 * 1000 * 1000)
#define LCD_HSYNC_PULSE 8
#define LCD_HSYNC_BACK  8
#define LCD_HSYNC_FRONT 4
#define LCD_VSYNC_PULSE 8
#define LCD_VSYNC_BACK  8
#define LCD_VSYNC_FRONT 4

#define BOARD_H_RES  LCD_H_RES
#define BOARD_V_RES  LCD_V_RES

// CH422G EXIO2 = backlight (high=on)
#define CH422G_BL_PIN  IO_EXPANDER_PIN_NUM_2

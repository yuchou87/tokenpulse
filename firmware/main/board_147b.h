#pragma once
// ESP32-S3-LCD-1.47B — ST7789 SPI (pins from board schematic)
#define LCD_CS    42
#define LCD_SCL   40   // SPI CLK
#define LCD_SDA   45   // SPI MOSI (DIN)
#define LCD_RST   39
#define LCD_DC    41
#define LCD_BL    46   // backlight, high=on
#define LCD_H_RES 172
#define LCD_V_RES 320
#define LCD_X_GAP 34
#define LCD_Y_GAP 0
#define BOARD_H_RES LCD_H_RES
#define BOARD_V_RES LCD_V_RES

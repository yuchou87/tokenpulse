#pragma once
// ESP32-S3-LCD-1.47B — ST7789 SPI
#define LCD_CS    21
#define LCD_SCL   38   // SPI CLK
#define LCD_SDA   39   // SPI MOSI
#define LCD_RST   40
#define LCD_DC    45
#define LCD_BL    46   // 背光,高=亮(8050)
#define LCD_H_RES 172
#define LCD_V_RES 320
#define LCD_X_GAP 34
#define LCD_Y_GAP 0
#define BOARD_H_RES LCD_H_RES
#define BOARD_V_RES LCD_V_RES

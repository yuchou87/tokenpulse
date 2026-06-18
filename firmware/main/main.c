#include "display.h"
#include "board.h"
#include "esp_lcd_panel_ops.h"
#include "esp_heap_caps.h"
#include <string.h>
void app_main(void) {
    esp_lcd_panel_handle_t panel = display_init();
    size_t n = LCD_H_RES * LCD_V_RES;
    uint16_t *fb = heap_caps_malloc(n * 2, MALLOC_CAP_DMA | MALLOC_CAP_SPIRAM);
    for (size_t i = 0; i < n; i++) fb[i] = 0xF800; // amber? 这是纯红,先验证刷得动
    esp_lcd_panel_draw_bitmap(panel, 0, 0, LCD_H_RES, LCD_V_RES, fb);
}

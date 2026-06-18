idf_component_register(SRCS "main.c" "display.c" "ui.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_lcd driver esp_driver_gpio esp_driver_spi)

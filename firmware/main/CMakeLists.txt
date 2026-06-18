idf_component_register(SRCS "main.c" "display.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_lcd driver esp_driver_gpio esp_driver_spi)

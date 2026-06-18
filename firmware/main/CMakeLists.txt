idf_component_register(SRCS "main.c" "display.c" "ui.c" "comm.c" "usb.c" "keepawake.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_lcd driver esp_driver_gpio esp_driver_spi
                              espressif__esp_tinyusb espressif__cjson esp_timer)

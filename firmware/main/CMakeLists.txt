set(BOARD_SRCS)
set(BOARD_REQUIRES)
if(CONFIG_TP_BOARD_43C)
    list(APPEND BOARD_SRCS "display_43c.c" "ui_43c.c")
    # legacy i2c (i2c_driver_install) for the IO expander lives in `driver` (already required)
else()
    list(APPEND BOARD_SRCS "display_147b.c" "ui_147b.c")
    list(APPEND BOARD_REQUIRES esp_driver_spi)
endif()

idf_component_register(SRCS "main.c" "comm.c" "usb.c" "keepawake.c" ${BOARD_SRCS}
                    INCLUDE_DIRS "."
                    REQUIRES esp_lcd driver esp_driver_gpio
                             espressif__esp_tinyusb espressif__cjson esp_timer
                             ${BOARD_REQUIRES})

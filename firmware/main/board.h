#pragma once
#if defined(CONFIG_TP_BOARD_43C)
#  include "board_43c.h"
#  define BOARD_NAME "ESP32-S3-Touch-LCD-4.3C"
#else
#  include "board_147b.h"
#  define BOARD_NAME "ESP32-S3-LCD-1.47B"
#endif

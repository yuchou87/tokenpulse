/*
 * Local stub for esp_expander_utils.h — replaces the managed component's
 * version to avoid pulling in esp-lib-utils C++ headers that fail to compile
 * under IDF v6.0.1 strict C++ mode. The C port files (esp_io_expander*.c)
 * only use standard IDF macros (ESP_RETURN_ON_ERROR, ESP_LOGI, etc.)
 * and do not need any macros from esp_lib_utils.
 */
#pragma once

#pragma once
#include <stddef.h>
#include <stdint.h>

/**
 * comm — USB CDC RX line handler
 *
 * comm_handle_line: parse one JSON line -> fill ui_state_t -> call ui_update
 * comm_last_rx_ms:  timestamp (esp_timer ms) of last successful parse
 */
void     comm_handle_line(const char *line, size_t len);
/* Returns ms timestamp (uint32_t, atomic on S3) of last successful parse.
 * Wraps at ~49 days; fine for 60-second stale detection. */
uint32_t comm_last_rx_ms(void);

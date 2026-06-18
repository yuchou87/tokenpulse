#include "comm.h"
#include "ui.h"
#include "cJSON.h"
#include "esp_lvgl_port.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "comm";
static int64_t g_last_rx_ms;

/**
 * Parse one newline-terminated JSON snapshot and update UI.
 *
 * Schema: {"ts":...,"state":"...","providers":[{"id":"...",
 *   "five_pct":N,"five_reset_s":N,"week_pct":N,"session_usd":F}]}
 */
void comm_handle_line(const char *line, size_t len)
{
    cJSON *root = cJSON_ParseWithLength(line, len);
    if (!root) {
        ESP_LOGW(TAG, "JSON parse failed");
        return;
    }

    cJSON *provs = cJSON_GetObjectItem(root, "providers");
    cJSON *p = (provs && cJSON_IsArray(provs)) ? cJSON_GetArrayItem(provs, 0) : NULL;

    if (p) {
        cJSON *j_five  = cJSON_GetObjectItem(p, "five_pct");
        cJSON *j_reset = cJSON_GetObjectItem(p, "five_reset_s");
        cJSON *j_week  = cJSON_GetObjectItem(p, "week_pct");

        if (j_five && j_reset && j_week) {
            ui_state_t st = {
                .five_pct     = j_five->valueint,
                .five_reset_s = (long)j_reset->valuedouble,
                .week_pct     = j_week->valueint,
                .session_usd  = 0.0,
                .stale        = false,
            };

            cJSON *j_usd = cJSON_GetObjectItem(p, "session_usd");
            if (j_usd) {
                st.session_usd = j_usd->valuedouble;
            }

            g_last_rx_ms = esp_timer_get_time() / 1000;
            ESP_LOGI(TAG, "rx: 5h=%d%% week=%d%% reset=%lds",
                     st.five_pct, st.week_pct, st.five_reset_s);

            lvgl_port_lock(0);
            ui_update(&st);
            lvgl_port_unlock();
        } else {
            ESP_LOGW(TAG, "providers[0] missing required fields");
        }
    } else {
        ESP_LOGW(TAG, "no providers array in snapshot");
    }

    cJSON_Delete(root);
}

int64_t comm_last_rx_ms(void)
{
    return g_last_rx_ms;
}

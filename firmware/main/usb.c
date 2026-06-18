#include "usb.h"
#include "comm.h"
#include "tinyusb.h"
#include "tusb_cdc_acm.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "usb";

/* Line accumulator — collect bytes until '\n', then call comm_handle_line */
static char  s_buf[512];
static size_t s_blen;

static void cdc_rx_cb(int itf, cdcacm_event_t *event)
{
    (void)event; /* event->type == CDC_EVENT_RX */

    uint8_t tmp[64];
    size_t  got = 0;

    /* Drain the RX FIFO in chunks */
    while (true) {
        esp_err_t ret = tinyusb_cdcacm_read((tinyusb_cdcacm_itf_t)itf,
                                             tmp, sizeof(tmp), &got);
        if (ret != ESP_OK || got == 0) break;

        for (size_t i = 0; i < got; i++) {
            char c = (char)tmp[i];
            if (c == '\n') {
                if (s_blen > 0) {
                    s_buf[s_blen] = '\0';
                    comm_handle_line(s_buf, s_blen);
                    s_blen = 0;
                }
            } else if (s_blen < sizeof(s_buf) - 1) {
                s_buf[s_blen++] = c;
            }
            /* silently drop if buffer full — will recover on next '\n' */
        }
    }
}

void usb_init(void)
{
    ESP_LOGI(TAG, "installing TinyUSB driver (CDC only)");

    /* Install TinyUSB core. NULL fields → use Kconfig/default descriptors */
    const tinyusb_config_t cfg = {
        .device_descriptor        = NULL,
        .string_descriptor        = NULL,
        .string_descriptor_count  = 0,
        .external_phy             = false,
        .configuration_descriptor = NULL,
    };
    ESP_ERROR_CHECK(tinyusb_driver_install(&cfg));

    /* Register CDC ACM interface 0 with RX callback */
    tinyusb_config_cdcacm_t acm = {
        .usb_dev                   = TINYUSB_USBDEV_0,
        .cdc_port                  = TINYUSB_CDC_ACM_0,
        .callback_rx               = &cdc_rx_cb,
        .callback_rx_wanted_char   = NULL,
        .callback_line_state_changed = NULL,
        .callback_line_coding_changed = NULL,
    };
    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm));

    ESP_LOGI(TAG, "USB CDC ready");
}

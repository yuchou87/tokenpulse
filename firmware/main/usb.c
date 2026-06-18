/*
 * usb.c — TinyUSB composite CDC + HID keyboard device
 *
 * CDC (interface 0+1): receives JSON snapshots from host
 * HID (interface 2):   sends F15 key-presses for keep-awake
 *
 * The esp_tinyusb component auto-generates configuration descriptors only for
 * CDC/MSC/NCM/Vendor through Kconfig macros; HID is NOT included in the
 * auto-generated descriptor.  We therefore supply a full custom configuration
 * descriptor and implement the three mandatory TinyUSB HID device callbacks.
 */

#include "usb.h"
#include "comm.h"

#include "tinyusb.h"
#include "tusb_cdc_acm.h"
#include "class/hid/hid_device.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "usb";

/* -----------------------------------------------------------------------
 * HID report descriptor — standard boot-protocol keyboard
 * ----------------------------------------------------------------------- */
static const uint8_t s_hid_report_desc[] = {
    TUD_HID_REPORT_DESC_KEYBOARD()
};

/* -----------------------------------------------------------------------
 * Custom CDC + HID composite configuration descriptor
 *
 * Interface layout:
 *   0   CDC control  (notification EP 0x81)
 *   1   CDC data     (bulk EP 0x02 OUT, 0x82 IN)
 *   2   HID keyboard (interrupt EP 0x83 IN, 10 ms poll)
 *
 * Endpoint numbers must not overlap; EP 0x80/0x00 is EP0 (reserved).
 * ----------------------------------------------------------------------- */
enum {
    ITF_NUM_CDC = 0,
    ITF_NUM_CDC_DATA,
    ITF_NUM_HID,
    ITF_NUM_TOTAL
};

enum {
    EPNUM_CDC_NOTIF = 0x81,
    EPNUM_CDC_OUT   = 0x02,
    EPNUM_CDC_IN    = 0x82,
    EPNUM_HID_IN    = 0x83,
};

#define CONFIG_DESC_TOTAL_LEN \
    (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_HID_DESC_LEN)

static const uint8_t s_cfg_desc[] = {
    /* Configuration */
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0,
                          CONFIG_DESC_TOTAL_LEN,
                          TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    /* CDC (uses Interface Association Descriptor internally) */
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC,      /* interface number */
                       4,                /* string index (4 = CDC) */
                       EPNUM_CDC_NOTIF,  /* notification EP (IN) */
                       8,                /* notification EP size */
                       EPNUM_CDC_OUT,    /* data EP OUT */
                       EPNUM_CDC_IN,     /* data EP IN */
                       64),              /* bulk EP size */

    /* HID keyboard, input-only, non-boot */
    TUD_HID_DESCRIPTOR(ITF_NUM_HID,
                       5,                /* string index (5 = HID) */
                       HID_ITF_PROTOCOL_NONE,
                       sizeof(s_hid_report_desc),
                       EPNUM_HID_IN,
                       8,                /* interrupt EP size */
                       10),              /* polling interval ms */
};

/* -----------------------------------------------------------------------
 * String descriptors — must stay in sync with string index constants above
 * ----------------------------------------------------------------------- */
static const char *s_str_desc[] = {
    /* 0 */ (char[]){0x09, 0x04},  /* language: English */
    /* 1 */ "Espressif",
    /* 2 */ "TokenPulse",
    /* 3 */ "TP-0001",
    /* 4 */ "TokenPulse CDC",
    /* 5 */ "TokenPulse HID",
    NULL
};

/* -----------------------------------------------------------------------
 * CDC line accumulator
 * ----------------------------------------------------------------------- */
static char   s_buf[512];
static size_t s_blen;

static void cdc_rx_cb(int itf, cdcacm_event_t *event)
{
    (void)event;

    uint8_t tmp[64];
    size_t  got = 0;

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
        }
    }
}

/* -----------------------------------------------------------------------
 * TinyUSB HID device callbacks (mandatory)
 * ----------------------------------------------------------------------- */

/* Called when host requests the HID report descriptor */
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    (void)instance;
    return s_hid_report_desc;
}

/* Called when host wants to GET_REPORT (read current state).
 * Return an empty (all-zeros) report. */
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                                hid_report_type_t report_type,
                                uint8_t *buffer, uint16_t reqlen)
{
    (void)instance; (void)report_id; (void)report_type;
    memset(buffer, 0, reqlen);
    return reqlen;
}

/* Called when host sends SET_REPORT (e.g. LED state for NumLock).
 * Nothing to do for a send-only keyboard. */
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                            hid_report_type_t report_type,
                            uint8_t const *buffer, uint16_t bufsize)
{
    (void)instance; (void)report_id; (void)report_type;
    (void)buffer;   (void)bufsize;
}

/* -----------------------------------------------------------------------
 * Public API
 * ----------------------------------------------------------------------- */

void usb_hid_send_key(uint8_t keycode)
{
    if (!tud_hid_ready()) {
        ESP_LOGW(TAG, "HID not ready, skipping key 0x%02x", keycode);
        return;
    }
    /* Key down */
    uint8_t kc[6] = { keycode, 0, 0, 0, 0, 0 };
    tud_hid_keyboard_report(0, 0, kc);
    vTaskDelay(pdMS_TO_TICKS(20));
    /* Key up */
    tud_hid_keyboard_report(0, 0, NULL);
}

void usb_init(void)
{
    ESP_LOGI(TAG, "installing TinyUSB driver (CDC + HID composite)");

    const tinyusb_config_t cfg = {
        .device_descriptor        = NULL,   /* use built-in device descriptor */
        .string_descriptor        = s_str_desc,
        .string_descriptor_count  = 6,      /* matches s_str_desc array, excluding NULL */
        .external_phy             = false,
        .configuration_descriptor = s_cfg_desc,
    };
    ESP_ERROR_CHECK(tinyusb_driver_install(&cfg));

    /* Register CDC ACM interface 0 with RX callback */
    tinyusb_config_cdcacm_t acm = {
        .usb_dev                      = TINYUSB_USBDEV_0,
        .cdc_port                     = TINYUSB_CDC_ACM_0,
        .callback_rx                  = &cdc_rx_cb,
        .callback_rx_wanted_char      = NULL,
        .callback_line_state_changed  = NULL,
        .callback_line_coding_changed = NULL,
    };
    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm));

    ESP_LOGI(TAG, "USB CDC + HID composite ready");
}

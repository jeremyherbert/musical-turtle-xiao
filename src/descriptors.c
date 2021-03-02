/* SPDX-License-Identifier: MIT */

#include "tusb.h"

#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define USB_PID           (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                           _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )

static tusb_desc_device_t const desc_device = {
        .bLength            = sizeof(tusb_desc_device_t),
        .bDescriptorType    = TUSB_DESC_DEVICE,
        .bcdUSB             = 0x0200,

        // the following three are needed for a USB composite device
        .bDeviceClass       = TUSB_CLASS_MISC,
        .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
        .bDeviceProtocol    = MISC_PROTOCOL_IAD,

        .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

        .idVendor           = 0xf055,
        .idProduct          = USB_PID,
        .bcdDevice          = 0x0100,

        .iManufacturer      = 0x01,
        .iProduct           = 0x02,
        .iSerialNumber      = 0x03,

        .bNumConfigurations = 0x01
};

uint8_t const * tud_descriptor_device_cb(void) {
    return (uint8_t const *) &desc_device;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
    ITF_NUM_CDC = 0,
    ITF_NUM_CDC_DATA,
    ITF_NUM_MIDI,
    ITF_NUM_MIDI_STREAMING,
    ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN    (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_MIDI_DESC_LEN)

static uint8_t const desc_configuration[] = {
        // Interface count, string index, total length, attribute, power in mA
        TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 500),

        // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
        TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, 0x81, 8, 0x02, 0x82, 64),

        // Interface number, string index, EP Out & EP In address, EP size
        TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 5, 0x03, 0x83, 64)
};

uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
    (void) index; // for multiple configurations
    return desc_configuration;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

static char serial[33] = "00112233445566778899AABBCCDDEEFF";

static char const* string_desc_arr [] = {
        (const char[]) { 0x09, 0x04 }, // 0: is supported language is English (0x0409)
        "Herbert Engineering",                     // 1: Manufacturer
        "musical-turtle-xiao",              // 2: Product
        serial,                      // 3: Serials, should use chip ID
        "musical-turtle CDC",                 // 4: CDC Interface
        "musical-turtle MIDI",                 // 5: MIDI Interface
};

static uint16_t string_descriptor_buf[64];
static bool serial_readout_done = false;

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {

    (void)langid; // ignore arg

    uint8_t chr_count;

    if (!serial_readout_done) {
        uint32_t unique_id[4];
        unique_id[0] = *((uint32_t*)0x0080A00C);
        unique_id[1] = *((uint32_t*)0x0080A040);
        unique_id[2] = *((uint32_t*)0x0080A044);
        unique_id[3] = *((uint32_t*)0x0080A048);

        sprintf(serial, "%08lX%08lX%08lX%08lX", unique_id[0], unique_id[1], unique_id[2], unique_id[3]);

        serial_readout_done = true;
    }

    if ( index == 0)
    {
        memcpy(&string_descriptor_buf[1], string_desc_arr[0], 2);
        chr_count = 1;
    }else
    {
        // Convert ASCII string into UTF-16

        if ( !(index < sizeof(string_desc_arr)/sizeof(string_desc_arr[0])) ) return NULL;

        const char* str = string_desc_arr[index];

        // Cap at max char
        chr_count = strlen(str);
        if ( chr_count > 63 ) chr_count = 63;

        for(uint8_t i=0; i<chr_count; i++)
        {
            string_descriptor_buf[1+i] = str[i];
        }
    }

    // first byte is length (including header), second byte is string type
    string_descriptor_buf[0] = (TUSB_DESC_STRING << 8 ) | (2*chr_count + 2);

    return string_descriptor_buf;
}
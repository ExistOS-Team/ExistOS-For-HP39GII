/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#include <inttypes.h>
#include <stdlib.h>
#include "tusb.h"
#include "ServiceUSBDevice.h"

/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define USB_PID           (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                           _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,

    // Use Interface Association Descriptor (IAD) for CDC
    // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,

    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = 0xCafe,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const * tud_descriptor_device_cb(void)
{
  return (uint8_t const *) &desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

enum
{
  REPORT_ID_KEYBOARD = 1,
  REPORT_ID_MOUSE,
  REPORT_ID_CONSUMER_CONTROL,
  REPORT_ID_GAMEPAD,
  REPORT_ID_COUNT
};

uint8_t const desc_hid_report[] =
{
  TUD_HID_REPORT_DESC_KEYBOARD( HID_REPORT_ID(REPORT_ID_KEYBOARD         )),
  TUD_HID_REPORT_DESC_MOUSE   ( HID_REPORT_ID(REPORT_ID_MOUSE            ))
};
// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete

uint8_t const *tud_hid_descriptor_report_cb() {
  return desc_hid_report;
}
//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+




#if CFG_TUSB_MCU == OPT_MCU_LPC175X_6X || CFG_TUSB_MCU == OPT_MCU_LPC177X_8X || CFG_TUSB_MCU == OPT_MCU_LPC40XX
  // LPC 17xx and 40xx endpoint type (bulk/interrupt/iso) are fixed by its number
  // 0 control, 1 In, 2 Bulk, 3 Iso, 4 In, 5 Bulk etc ...
  #define EPNUM_CDC_NOTIF   0x81
  #define EPNUM_CDC_OUT     0x02
  #define EPNUM_CDC_IN      0x82

  #define EPNUM_MSC_OUT     0x05
  #define EPNUM_MSC_IN      0x85

#elif CFG_TUSB_MCU == OPT_MCU_SAMG
  // SAMG doesn't support a same endpoint number with different direction IN and OUT
  //    e.g EP1 OUT & EP1 IN cannot exist together
  #define EPNUM_CDC_NOTIF   0x81
  #define EPNUM_CDC_OUT     0x02
  #define EPNUM_CDC_IN      0x83

  #define EPNUM_MSC_OUT     0x04
  #define EPNUM_MSC_IN      0x85

#else
 
  #define EPNUM_CDC_NOTIF   0x81
  #define EPNUM_CDC_OUT     0x02
  #define EPNUM_CDC_IN      0x82

  #define EPNUM_MSC_OUT     0x03
  #define EPNUM_MSC_IN      0x83

  #define EPNUM_HID         0x81

#endif


// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
  (void) index; // for multiple configurations

  int itf_num = 0;
  int itf_desc_num = sizeof(usbd_enabled_itfs) / sizeof(usbd_enabled_itfs[0]);
  int config_len = TUD_CONFIG_DESC_LEN;
  size_t desc_size = 0;
  bool usb_hs = (tud_speed_get() == TUSB_SPEED_HIGH)? true:false;
  uint8_t itf_idx_cdc,itf_idx_msc,itf_idx_hid;
  for (int i = 0; i < itf_desc_num; ++i) {
    switch (usbd_enabled_itfs[i]) {
    case ITF_NUM_CDC:
      itf_idx_cdc = i;
      break;
    case ITF_NUM_MSC:
      itf_idx_msc = i;
      break;
    case ITF_NUM_HID:
      itf_idx_hid = i;
      break;
    default:
      break;
    }
  }

  uint8_t cdc_desc[] = {TUD_CDC_DESCRIPTOR(itf_idx_cdc, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, usb_hs? 512:64)};
  uint8_t msc_desc[] = {TUD_MSC_DESCRIPTOR(itf_idx_msc, 5, EPNUM_MSC_OUT, EPNUM_MSC_IN, usb_hs? 512:64)};
  uint8_t hid_desc[] = {TUD_HID_DESCRIPTOR(itf_idx_hid, 6, HID_PROTOCOL_NONE,sizeof(desc_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 5)};

  for (size_t i = 0; i < itf_desc_num; i++) {
    switch (usbd_enabled_itfs[i]) {
    case ITF_NUM_CDC:
      itf_num += 2;
      desc_size += sizeof(cdc_desc);
      config_len += TUD_CDC_DESC_LEN;
      break;
    case ITF_NUM_MSC:
      itf_num++;
      desc_size += sizeof(msc_desc);
      config_len += TUD_MSC_DESC_LEN;
      break;
    case ITF_NUM_HID:
      itf_num++;
      desc_size += sizeof(hid_desc);
      config_len += TUD_HID_DESC_LEN;
      break;
    default:
      break;
    }
  }
  uint8_t config_desc[] = {TUD_CONFIG_DESCRIPTOR(1, itf_num, 0, config_len, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100)};
  desc_size += sizeof(config_desc);

  uint8_t* desc = (uint8_t*)malloc(desc_size);  //GCC YES
  size_t desc_ptr = 0;
  memcpy(desc,config_desc,sizeof(config_desc));
  desc_ptr += sizeof(config_desc);
  for (int i = 0; i < itf_desc_num; ++i){
     switch (usbd_enabled_itfs[i]) {
    case ITF_NUM_CDC:
      memcpy(&desc[desc_ptr], cdc_desc, sizeof(cdc_desc));
      desc_ptr += sizeof(cdc_desc);
      break;
    case ITF_NUM_MSC:
      memcpy(&desc[desc_ptr], msc_desc, sizeof(msc_desc));
      desc_ptr += sizeof(msc_desc);
      break;
    case ITF_NUM_HID:
      memcpy(&desc[desc_ptr], hid_desc, sizeof(hid_desc));
      desc_ptr += sizeof(hid_desc);
      break;
    default:
      break;
    }
  }

  return desc;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// array of pointer to string descriptors
char const *string_desc_arr[] = {
    (const char[]){0x09, 0x04}, // 0: is supported language is English (0x0409)
    "Hewlett-Packard",          // 1: Manufacturer
    "HP 39GII Calc",            // 2: Product
    "39GII",                    // 3: Serials, should use chip ID
    "Console CDC",                 // 4: CDC Interface
    "USB MSC",      // 5: MSC Interface
    "USB HID Keyboard Mouse" // 6: HID Interface

};

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  (void) langid;

  uint8_t chr_count;

  if ( index == 0)
  {
    memcpy(&_desc_str[1], string_desc_arr[0], 2);
    chr_count = 1;
  }else
  {
    // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

    if ( !(index < sizeof(string_desc_arr)/sizeof(string_desc_arr[0])) ) return NULL;

    const char* str = string_desc_arr[index];

    // Cap at max char
    chr_count = strlen(str);
    if ( chr_count > 31 ) chr_count = 31;

    // Convert ASCII string into UTF-16
    for(uint8_t i=0; i<chr_count; i++)
    {
      _desc_str[1+i] = str[i];
    }
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (TUSB_DESC_STRING << 8 ) | (2*chr_count + 2);

  return _desc_str;
}

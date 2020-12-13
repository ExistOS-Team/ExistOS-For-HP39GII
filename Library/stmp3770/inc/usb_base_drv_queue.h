
#ifndef _USB_BASE_DRV_QUEUE_H
#define _USB_BASE_DRV_QUEUE_H
#include "usb_ch9.h"
#include <stdbool.h>

#define USB_BASE_DRV_QUEUE_LENGTH	512






void usb_base_drv_queue_init();
void usb_base_drv_queue_ctrl_req_join(struct usb_ctrlrequest *ctrl_req);
void usb_base_drv_queue_exec();


#endif
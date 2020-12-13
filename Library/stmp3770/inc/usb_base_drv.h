
#ifndef _USB_BASE_DRV_H
#define _USB_BASE_DRV_H
#include "usb_ch9.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void usb_drv_init();
void usb_exec_ctrl_req(struct usb_ctrlrequest *ctrl_req);



#endif
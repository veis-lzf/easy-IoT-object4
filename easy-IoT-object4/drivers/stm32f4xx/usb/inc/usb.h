#ifndef __USB__H__
#define __USB__H__

#include "usbh_conf.h"
#include "usbh_usr.h"
#include "usb_conf.h"

//host
#include "usbh_core.h"
#include "usbh_def.h"
#include "usbh_hcs.h"
#include "usbh_ioreq.h"
#include "usbh_stdreq.h"

//otg
#include "usb_bsp.h"
#include "usb_core.h"
#include "usb_dcd.h"
#include "usb_dcd_int.h"
#include "usb_defines.h"
#include "usb_hcd.h"
#include "usb_hcd_int.h"
#include "usb_otg.h"
#include "usb_regs.h"

extern struct usb_device usb_dev;
extern USB_OTG_CORE_HANDLE *p_usb_dev;

#endif

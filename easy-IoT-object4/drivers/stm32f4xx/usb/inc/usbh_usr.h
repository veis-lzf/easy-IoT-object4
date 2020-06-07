/**
 ******************************************************************************
 * @file    usbh_usr.h
 * @author  MCD Application Team
 * @version V1.0.0
 * @date   22/07/2011
 * @brief   Header file for usbh_usr.c
 ******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USH_USR_H__
#define __USH_USR_H__

void init_usb_struct(USB_OTG_CORE_HANDLE *pdev, USBH_HOST *phost);
int init_usb(void);

#define usb_sndctrlpipe(dev, address) 	(0)
#define usb_rcvctrlpipe(dev, address) 	(1)

#define usb_intpipe(X,Y)  		(0 + 2)
#define usb_rcvintpipe(X,Y)  	(0 + 2)
#define usb_sndbulkpipe(X,Y) 	(1 + 2)
#define usb_rcvbulkpipe(X,Y)  	(2 + 2)

#define usb_sndisocpipe(X,Y)  	(3 + 2)
#define usb_rcvisocpipe(X,Y)  	(4 + 2)


int usb_control_msg(unsigned int pipe, uint8_t request, uint8_t requesttype, uint16_t value, uint16_t index, void *data, uint16_t size, int timeout);
void usb_fill_bulk_urb(struct urb *urb, struct usb_device *dev, unsigned int pipe, void *transfer_buffer, int buffer_length, void *complete_fn, void *context);
int usb_submit_urb(struct urb *urb, gfp_t mem_flags);
int usb_enumeration(USB_OTG_CORE_HANDLE *p_dev, USBH_HOST *p_host);

void usb_fill_bulk_urb(struct urb *urb, struct usb_device *dev, unsigned int pipe, void *transfer_buffer, int buffer_length, void *complete_fn, void *context);
void usb_fill_int_urb(struct urb *urb, struct usb_device *dev, unsigned int pipe, void *transfer_buffer, int buffer_length, usb_complete_t complete_fn, void *context, int interval);
void usb_kill_urb(struct urb *urb);
void reactive_urb(struct urb *urb);

uint8_t  USBH_FindInterface(USBH_HOST *phost, uint8_t Class, uint8_t SubClass, uint8_t Protocol);
#endif /*__USH_USR_H__*/

 /******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

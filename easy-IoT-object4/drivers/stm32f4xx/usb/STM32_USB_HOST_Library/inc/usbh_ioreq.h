/**
 ******************************************************************************
 * @file    usbh_ioreq.h
 * @author  MCD Application Team
 * @version V2.0.0
 * @date    22-July-2011
 * @brief   Header file for usbh_ioreq.c
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

/* Define to prevent recursive  ----------------------------------------------*/
#ifndef __USBH_IOREQ_H
#define __USBH_IOREQ_H

/* Includes ------------------------------------------------------------------*/
#include "usb_conf.h"
#include "usbh_core.h"
#include "usbh_def.h"


/** @addtogroup USBH_LIB
 * @{
 */

/** @addtogroup USBH_LIB_CORE
 * @{
 */

/** @defgroup USBH_IOREQ
 * @brief This file is the header file for usbh_ioreq.c
 * @{
 */


/** @defgroup USBH_IOREQ_Exported_Defines
 * @{
 */
#define USBH_SETUP_PKT_SIZE   	8
#define USBH_EP0_EP_NUM       		0
#define USBH_MAX_PACKET_SIZE  	0x40
/**
 * @}
 */


/** @defgroup USBH_IOREQ_Exported_Types
 * @{
 */
/**
 * @}
 */


/** @defgroup USBH_IOREQ_Exported_Macros
 * @{
 */
/**
 * @}
 */

/** @defgroup USBH_IOREQ_Exported_Variables
 * @{
 */
/**
 * @}
 */

/** @defgroup USBH_IOREQ_Exported_FunctionsPrototype
 * @{
 */
USBH_Status USBH_CtlSendSetup(USB_OTG_CORE_HANDLE *pdev, uint8_t *buff, uint8_t hc_num);

USBH_Status USBH_CtlSendData(USB_OTG_CORE_HANDLE *pdev, uint8_t *buff, uint8_t length, uint8_t hc_num);

USBH_Status USBH_CtlReceiveData(USB_OTG_CORE_HANDLE *pdev, uint8_t *buff, uint8_t length, uint8_t hc_num);

USBH_Status USBH_BulkReceiveData(struct urb *urb, USB_OTG_CORE_HANDLE *pdev, uint8_t *buff, uint16_t length, uint8_t hc_num);

USBH_Status USBH_BulkSendData(struct urb *urb, USB_OTG_CORE_HANDLE *pdev, uint8_t *buff, uint16_t length, uint8_t hc_num);

USBH_Status USBH_InterruptReceiveData(struct urb *urb, USB_OTG_CORE_HANDLE *pdev, uint8_t *buff, uint8_t length, uint8_t hc_num);

USBH_Status USBH_InterruptSendData(struct urb *urb, USB_OTG_CORE_HANDLE *pdev, uint8_t *buff, uint8_t length, uint8_t hc_num);

USBH_Status USBH_IsocSendData(struct urb *urb, USB_OTG_CORE_HANDLE *pdev, uint8_t *buff, uint32_t length, uint8_t hc_num);

USBH_Status USBH_IsocReceiveData(struct urb *urb, USB_OTG_CORE_HANDLE *pdev, uint8_t *buff, uint32_t length, uint8_t hc_num);

USBH_Status USBH_CtlReq(USB_OTG_CORE_HANDLE *pdev, USBH_HOST *phost, uint8_t *buff, uint16_t length);

//int usb_sndbulkpipe(void *pUsb_Dev, int uEndpointAddress);

void usb_fill_bulk_urb(struct urb *urb, struct usb_device *dev, unsigned int pipe, void *transfer_buffer, int buffer_length, void *complete_fn, void *context);
int usb_submit_urb(struct urb *urb, gfp_t mem_flags);

/**
 * @}
 */

#endif /* __USBH_IOREQ_H */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

 /******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

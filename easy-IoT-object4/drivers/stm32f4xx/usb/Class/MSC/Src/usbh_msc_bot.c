/**
 ******************************************************************************
 * @file    usbh_msc_bot.c 
 * @author  MCD Application Team
 * @version V2.1.0
 * @date    19-March-2012
 * @brief   This file includes the mass storage related functions
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#define DEBUG
#include "drivers.h"
#include "app.h"
#include "api.h"
#include "usb_regs.h"
#include "usb_defines.h"
#include "usb_core.h"
#include "usbh_def.h"
#include "usbh_conf.h"
#include "usb_dcd.h"
#include "usb_bsp.h"

#include "usbh_msc_core.h"
#include "usbh_msc_scsi.h"
#include "usbh_msc_bot.h"
#include "usbh_ioreq.h"
#include "usbh_def.h"
#include "usb_hcd_int.h"


/** @addtogroup USBH_LIB
 * @{
 */

/** @addtogroup USBH_CLASS
 * @{
 */

/** @addtogroup USBH_MSC_CLASS
 * @{
 */

/** @defgroup USBH_MSC_BOT 
 * @brief    This file includes the mass storage related functions
 * @{
 */


/** @defgroup USBH_MSC_BOT_Private_TypesDefinitions
 * @{
 */
/**
 * @}
 */

/** @defgroup USBH_MSC_BOT_Private_Defines
 * @{
 */
/**
 * @}
 */

/** @defgroup USBH_MSC_BOT_Private_Macros
 * @{
 */
/**
 * @}
 */


/** @defgroup USBH_MSC_BOT_Private_Variables
 * @{
 */

HostCBWPkt_TypeDef USBH_MSC_CBWData /*  __attribute__((aligned(4)))*/;

HostCSWPkt_TypeDef USBH_MSC_CSWData /*__attribute__((aligned(4))) */;


/**
 * @}
 */


/** @defgroup USBH_MSC_BOT_Private_FunctionPrototypes
 * @{
 */
/**
 * @}
 */


/**
 * @}
 */
struct urb *bulk_out_urb = NULL;
struct urb *bulk_in_urb = NULL;

/** @defgroup USBH_MSC_BOT_Private_Functions
 * @{
 */

/**
 * @brief  USBH_MSC_Init 
 *         Initializes the mass storage parameters
 * @param  None
 * @retval None
 */
int USBH_MSC_Init(USB_OTG_CORE_HANDLE *pdev)
{
    USBH_MSC_CBWData.field.CBWSignature = USBH_MSC_BOT_CBW_SIGNATURE;
    USBH_MSC_CBWData.field.CBWTag = USBH_MSC_BOT_CBW_TAG;
    USBH_MSC_CBWData.field.CBWLUN = 0; /*Only one LUN is supported*/

    bulk_out_urb = (struct urb*)usb_alloc_urb(0, GFP_ATOMIC);
    if (!bulk_out_urb)
        goto err;
    bulk_in_urb = (struct urb*)usb_alloc_urb(0, GFP_ATOMIC);
    if (!bulk_in_urb)
        goto err;

    bulk_out_urb->wait_complete = init_event();
    if (!bulk_out_urb->wait_complete)
        goto err;
    bulk_in_urb->wait_complete = init_event();
    if (!bulk_in_urb->wait_complete)
        goto err;
   
    return 0;
err: 
		if (bulk_out_urb && bulk_out_urb->wait_complete)
        del_event(bulk_out_urb->wait_complete);
    if (bulk_in_urb && bulk_in_urb->wait_complete)
        del_event(bulk_in_urb->wait_complete);

    if (bulk_out_urb)
        mem_free(bulk_out_urb);
    if (bulk_in_urb)
        mem_free(bulk_in_urb);

    bulk_out_urb = 0;
    bulk_in_urb = 0;
    return  - 1;
}


void USBH_MSC_DeInit(USB_OTG_CORE_HANDLE *pdev)
{
    if (bulk_out_urb && bulk_out_urb->wait_complete)
        del_event(bulk_out_urb->wait_complete);
    if (bulk_in_urb && bulk_in_urb->wait_complete)
        del_event(bulk_in_urb->wait_complete);

    if (bulk_out_urb)
        mem_free(bulk_out_urb);
    if (bulk_in_urb)
        mem_free(bulk_in_urb);

    bulk_out_urb = 0;
    bulk_in_urb = 0;
}

/**
 * @brief  USBH_MSC_BOT_Abort 
 *         This function manages the different Error handling for STALL
 * @param  direction : IN / OUT 
 * @retval None
 */
USBH_Status USBH_MSC_BOT_Abort(USB_OTG_CORE_HANDLE *pdev, USBH_HOST *phost, uint8_t direction)
{
    USBH_Status status;

    status = USBH_BUSY;

    assert(bulk_in_urb && bulk_out_urb);

    switch (direction)
    {
        case USBH_MSC_DIR_IN:
        /* send ClrFeture on Bulk IN endpoint */
        status = USBH_ClrFeature(pdev, phost, pdev->host.hc[bulk_in_urb->pipe].ep_num, bulk_in_urb->pipe);

        break;

        case USBH_MSC_DIR_OUT:
        /*send ClrFeature on Bulk OUT endpoint */
        status = USBH_ClrFeature(pdev, phost, pdev->host.hc[bulk_out_urb->pipe].ep_num, bulk_out_urb->pipe);
        break;

        default:
        break;
    }

    return status;
}

/**
 * @brief  USBH_MSC_DecodeCSW
 *         This function decodes the CSW received by the device and updates the
 *         same to upper layer.
 * @param  None
 * @retval On success USBH_MSC_OK, on failure USBH_MSC_FAIL
 * @notes
 *     Refer to USB Mass-Storage Class : BOT (www.usb.org)
 *    6.3.1 Valid CSW Conditions :
 *     The host shall consider the CSW valid when:
 *     1. dCSWSignature is equal to 53425355h
 *     2. the CSW is 13 (Dh) bytes in length,
 *     3. dCSWTag matches the dCBWTag from the corresponding CBW.
 */

uint8_t USBH_MSC_DecodeCSW(USB_OTG_CORE_HANDLE *pdev)
{
    uint8_t status;
    uint32_t dataXferCount = 0;
    status = USBH_MSC_FAIL;

    if (HCD_IsDeviceConnected(pdev))
    {
        /*Checking if the transfer length is diffrent than 13*/
        dataXferCount = HCD_GetXferCnt(pdev, bulk_in_urb->pipe);

        if (dataXferCount != USBH_MSC_CSW_LENGTH)
        {
            /*(4) Hi > Dn (Host expects to receive data from the device,
            Device intends to transfer no data)
            (5) Hi > Di (Host expects to receive data from the device,
            Device intends to send data to the host)
            (9) Ho > Dn (Host expects to send data to the device,
            Device intends to transfer no data)
            (11) Ho > Do  (Host expects to send data to the device,
            Device intends to receive data from the host)*/


            status = USBH_MSC_PHASE_ERROR;
        }
        else
        {
            /* CSW length is Correct */

            /* Check validity of the CSW Signature and CSWStatus */
            if (USBH_MSC_CSWData.field.CSWSignature == USBH_MSC_BOT_CSW_SIGNATURE)
            {
                /* Check Condition 1. dCSWSignature is equal to 53425355h */

                if (USBH_MSC_CSWData.field.CSWTag == USBH_MSC_CBWData.field.CBWTag)
                {
                    /* Check Condition 3. dCSWTag matches the dCBWTag from the 
                    corresponding CBW */

                    if (USBH_MSC_CSWData.field.CSWStatus == USBH_MSC_OK)
                    {
                        /* Refer to USB Mass-Storage Class : BOT (www.usb.org) 

                        Hn Host expects no data transfers
                        Hi Host expects to receive data from the device
                        Ho Host expects to send data to the device

                        Dn Device intends to transfer no data
                        Di Device intends to send data to the host
                        Do Device intends to receive data from the host

                        Section 6.7 
                        (1) Hn = Dn (Host expects no data transfers,
                        Device intends to transfer no data)
                        (6) Hi = Di (Host expects to receive data from the device,
                        Device intends to send data to the host)
                        (12) Ho = Do (Host expects to send data to the device, 
                        Device intends to receive data from the host)

                         */

                        status = USBH_MSC_OK;
                    }
                    else if (USBH_MSC_CSWData.field.CSWStatus == USBH_MSC_FAIL)
                    {
                        status = USBH_MSC_FAIL;
                    }

                    else if (USBH_MSC_CSWData.field.CSWStatus == USBH_MSC_PHASE_ERROR)
                    {
                        /* Refer to USB Mass-Storage Class : BOT (www.usb.org) 
                        Section 6.7 
                        (2) Hn < Di ( Host expects no data transfers, 
                        Device intends to send data to the host)
                        (3) Hn < Do ( Host expects no data transfers, 
                        Device intends to receive data from the host)
                        (7) Hi < Di ( Host expects to receive data from the device, 
                        Device intends to send data to the host)
                        (8) Hi <> Do ( Host expects to receive data from the device, 
                        Device intends to receive data from the host)
                        (10) Ho <> Di (Host expects to send data to the device,
                        Di Device intends to send data to the host)
                        (13) Ho < Do (Host expects to send data to the device, 
                        Device intends to receive data from the host)
                         */

                        status = USBH_MSC_PHASE_ERROR;
                    }
                } /* CSW Tag Matching is Checked  */
            } /* CSW Signature Correct Checking */
            else
            {
                /* If the CSW Signature is not valid, We sall return the Phase Error to
                Upper Layers for Reset Recovery */

                status = USBH_MSC_PHASE_ERROR;
            }
        } /* CSW Length Check*/
    }

    return status;
}


/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

 /************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

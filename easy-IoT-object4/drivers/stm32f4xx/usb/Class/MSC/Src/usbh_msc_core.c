/**
 ******************************************************************************
 * @file    usbh_msc_core.c
 * @author  MCD Application Team
 * @version V2.1.0
 * @date    19-March-2012
 * @brief   This file implements the MSC class driver functions
 *          ===================================================================      
 *                                MSC Class  Description
 *          =================================================================== 
 *           This module manages the MSC class V1.0 following the "Universal 
 *           Serial Bus Mass Storage Class (MSC) Bulk-Only Transport (BOT) Version 1.0
 *           Sep. 31, 1999".
 *           This driver implements the following aspects of the specification:
 *             - Bulk-Only Transport protocol
 *             - Subclass : SCSI transparent command set (ref. SCSI Primary Commands - 3 (SPC-3))
 *      
 *  @endverbatim
 *
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
#include "usbh_core.h"


/** @addtogroup USBH_LIB
 * @{
 */

/** @addtogroup USBH_CLASS
 * @{
 */

/** @addtogroup USBH_MSC_CLASS
 * @{
 */

/** @defgroup USBH_MSC_CORE 
 * @brief    This file includes the mass storage related functions
 * @{
 */


/** @defgroup USBH_MSC_CORE_Private_TypesDefinitions
 * @{
 */
/**
 * @}
 */

/** @defgroup USBH_MSC_CORE_Private_Defines
 * @{
 */
#define USBH_MSC_ERROR_RETRY_LIMIT 10
/**
 * @}
 */

/** @defgroup USBH_MSC_CORE_Private_Macros
 * @{
 */
/**
 * @}
 */
extern struct urb *bulk_out_urb;
extern struct urb *bulk_in_urb;

/**
 * @}
 */


/** @defgroup USBH_MSC_CORE_Private_FunctionPrototypes
 * @{
 */
static USBH_Status USBH_MSC_BOTReset(USB_OTG_CORE_HANDLE *pdev, USBH_HOST *phost);
static uint8_t USBH_MSC_GETMaxLUN(USB_OTG_CORE_HANDLE *pdev, USBH_HOST *phost);

/**
 * @}
 */


/** @defgroup USBH_MSC_CORE_Exported_Variables
 * @{
 */

/**
 * @}
 */


/** @defgroup USBH_MSC_CORE_Private_Functions
 * @{
 */

/**
 * @brief  USBH_MSC_InterfaceDeInit 
 *         De-Initialize interface by freeing host channels allocated to interface
 * @param  pdev: Selected device
 * @param  hdev: Selected device property
 * @retval None
 */
void USBH_MSC_InterfaceDeInit(USB_OTG_CORE_HANDLE *pdev, void *phost)
{
#if 0
    if (MSC_Machine.hc_num_out)
    {
        USB_OTG_HC_Halt(pdev, MSC_Machine.hc_num_out);
        USBH_Free_Channel(pdev, MSC_Machine.hc_num_out);
        MSC_Machine.hc_num_out = 0; /* Reset the Channel as Free */
    }

    if (MSC_Machine.hc_num_in)
    {
        USB_OTG_HC_Halt(pdev, MSC_Machine.hc_num_in);
        USBH_Free_Channel(pdev, MSC_Machine.hc_num_in);
        MSC_Machine.hc_num_in = 0; /* Reset the Channel as Free */
    }
#endif 
}

/*
 * @brief  初始化MSC设备,每次枚举完都会调用一次
 *
 * 
 * 
 */
int init_msc(USB_OTG_CORE_HANDLE *pdev, void *phost)
{
    int i;
    uint8_t maxLun;
    USBH_Status status;
    uint8_t mscStatus = USBH_MSC_BUSY;

    if (HCD_IsDeviceConnected(pdev))
    {
        for(i = 0; i < HC_MAX; i++)
	{
	        if (pdev->host.channel[i] &HC_USED)
	        {

	            if (pdev->host.hc[i].ep_type == USB_OTG_EP_BULK)
	            {
	                if (pdev->host.hc[i].ep_is_in)
	                {
			    if(bulk_in_urb){
	                   	bulk_in_urb->pipe = i;
			    }
	                }else{
	                    if(bulk_out_urb){
	                    	bulk_out_urb->pipe = i;
	                    }
	                }
	            } 


	        }

	 }

	if(!bulk_in_urb->pipe || !bulk_out_urb->pipe)	//pipe不可能等于0,0被控制端点用掉了
		return -1;
	 
        status = USBH_MSC_BOTReset(pdev, phost);
        if (status != USBH_OK)
        {
            p_err_fun;
            return  - 1;
        }

        maxLun = USBH_MSC_GETMaxLUN(pdev, phost);
        p_dbg("maxLun:%d", maxLun);
        if (maxLun > 0)
        //不支持多个
        {
            p_err_fun;
            return  - 1;
        }

        mscStatus = USBH_MSC_TestUnitReady(pdev);
        if (mscStatus == USBH_MSC_OK)
        {
            mscStatus = USBH_MSC_ReadCapacity10(pdev);
            if (mscStatus == USBH_MSC_OK)
            {
                mscStatus = USBH_MSC_ModeSense6(pdev);
                if (mscStatus == USBH_MSC_OK)
                {

                    return 0;
                }
            }
        }

    }

    return  - 1;
}

/**
 * @brief  USBH_MSC_BOTReset
 *         This request is used to reset the mass storage device and its 
 *         associated interface. This class-specific request shall ready the 
 *         device for the next CBW from the host.
 * @param  pdev: Selected device
 * @retval USBH_Status : Status of class request handled.
 */
static USBH_Status USBH_MSC_BOTReset(USB_OTG_CORE_HANDLE *pdev, USBH_HOST *phost)
{

    phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_TYPE_CLASS |  \
        USB_REQ_RECIPIENT_INTERFACE;

    phost->Control.setup.b.bRequest = USB_REQ_BOT_RESET;
    phost->Control.setup.b.wValue.w = 0;
    phost->Control.setup.b.wIndex.w = 0;
    phost->Control.setup.b.wLength.w = 0;

    return USBH_CtlReq(pdev, phost, 0, 0);
}


/**
 * @brief  USBH_MSC_GETMaxLUN
 *         This request is used to reset the mass storage device and its 
 *         associated interface. This class-specific request shall ready the 
 *         device for the next CBW from the host.
 * @param  pdev: Selected device
 * @retval USBH_Status : USB ctl xfer status
 */
static uint8_t USBH_MSC_GETMaxLUN(USB_OTG_CORE_HANDLE *pdev, USBH_HOST *phost)
{
    uint8_t max_lum = 0;
    USBH_Status ret;

    phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_TYPE_CLASS |  \
        USB_REQ_RECIPIENT_INTERFACE;

    phost->Control.setup.b.bRequest = USB_REQ_GET_MAX_LUN;
    phost->Control.setup.b.wValue.w = 0;
    phost->Control.setup.b.wIndex.w = 0;
    phost->Control.setup.b.wLength.w = 1;

    ret = USBH_CtlReq(pdev, phost, &max_lum, 1);
    if (ret != USBH_OK)
    {
        USBH_ClrFeature(pdev, phost, 0x00, phost->Control.hc_num_out);
    }
    return max_lum;
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

 /************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

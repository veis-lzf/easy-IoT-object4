/**
 ******************************************************************************
 * @file    usbh_core.c
 * @author  MCD Application Team
 * @version V2.0.0
 * @date    22-July-2011
 * @brief   This file implements the functions for the core state machine process
 *          the enumeration and the control transfer process
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
/* Includes ------------------------------------------------------------------*/
//#define DEBUG
#include "drivers.h"
#include "app.h"
#include "api.h"
#include "usb_regs.h"
#include "usb_defines.h"
#include "usb_core.h"
#include "usbh_def.h"
#include "usbh_conf.h"
#include "usbh_core.h"
#include "usb_bsp.h"
#include "usb_dcd.h"

#include "usbh_usr.h"
#include "usbh_hcs.h"
#include "usbh_stdreq.h"
#include "usbh_ioreq.h"

USB_OTG_CORE_HANDLE USB_OTG_Core; //need align

USBH_HOST USB_Host;

/** @addtogroup USBH_LIB
 * @{
 */

/** @addtogroup USBH_LIB_CORE
 * @{
 */

/** @defgroup USBH_CORE
 * @brief TThis file handles the basic enumaration when a device is connected
 *          to the host.
 * @{
 */

/** @defgroup USBH_CORE_Private_TypesDefinitions
 * @{
 */
void USBH_Disconnect(void *pdev);
void USBH_Connect(void *pdev);
/*
USB_OTG_hPort_TypeDef USBH_DeviceConnStatus_cb =
{
USBH_Disconnect, USBH_Connect, 0, 0, 0, 0
};*/
/**
 * @}
 */


/** @defgroup USBH_CORE_Private_Defines
 * @{
 */
/**
 * @}
 */


/** @defgroup USBH_CORE_Private_Macros
 * @{
 */
/**
 * @}
 */


/** @defgroup USBH_CORE_Private_Variables
 * @{
 */
/**
 * @}
 */


/** @defgroup USBH_CORE_Private_FunctionPrototypes
 * @{
 */
USBH_Status USBH_HandleControl(USB_OTG_CORE_HANDLE *pdev, USBH_HOST *phost);

/**
 * @}
 */


/** @defgroup USBH_CORE_Private_Functions
 * @{
 */


/**
 * @brief  USBH_Connect
 *         USB Connect callback function from the Interrupt.
 * @param  selected device
 * @retval none
 */
void USBH_Connect(void *pdev)
{
	//USB_OTG_CORE_HANDLE *ppdev = pdev;
	//  ppdev->host.port_cb->ConnStatus = 1;
	//  ppdev->host.port_cb->ConnHandled = 0;
}

/**
 * @brief  USBH_Disconnect
 *         USB Disconnect callback function from the Interrupt.
 * @param  selected device
 * @retval none
 */

void USBH_Disconnect(void *pdev)
{

	//USB_OTG_CORE_HANDLE *ppdev = pdev;

	/* Make device Not connected flag true */
	//  ppdev->host.port_cb->DisconnStatus = 1;
	//  ppdev->host.port_cb->DisconnHandled = 0;
}

/**
 * @brief  USBH_Init
 *         Host hardware and stack initializations
 * @param  class_cb: Class callback structure address
 * @param  usr_cb: User callback structure address
 * @retval None
 */

void USBH_Init(USB_OTG_CORE_HANDLE *pdev, USB_OTG_CORE_ID_TypeDef coreID, USBH_HOST *phost, USBH_Class_cb_TypeDef *class_cb, USBH_Usr_cb_TypeDef *usr_cb)
{
	init_usb_struct(pdev, phost);

	USB_OTG_BSP_Init(pdev);

	/* configure GPIO pin used for switching VBUS power */
	USB_OTG_BSP_ConfigVBUS(0);


	/* Host de-initializations */
	USBH_DeInit(pdev, phost);


	pdev->crl_urb_wait = init_event();
	pdev->bulkout_urb_wait = init_event();
	pdev->ctrl_urb_lock = mutex_init("");
	pdev->bulk_out_urb_lock = mutex_init("");
	pdev->int_out_urb_lock = mutex_init("");
	pdev->iso_out_urb_lock = mutex_init("");

	phost->Control.setup_urb_mutex = mutex_init("");
	/* Start the USB OTG core */
	HCD_Init(pdev, coreID);

	/* Enable Interrupts */
	USB_OTG_BSP_EnableInterrupt(pdev);

}

/**
 * @brief  USBH_DeInit
 *         Re-Initialize Host
 * @param  None
 * @retval status: USBH_Status
 */
USBH_Status USBH_DeInit(USB_OTG_CORE_HANDLE *pdev, USBH_HOST *phost)
{
	/* Software Init */
	phost->Control.ep0size = USB_OTG_MAX_EP0_SIZE;

	phost->device_prop.address = USBH_DEVICE_ADDRESS_DEFAULT;
	phost->device_prop.speed = HPRT0_PRTSPD_FULL_SPEED;

	USBH_Free_Channel(pdev, phost->Control.hc_num_in);
	USBH_Free_Channel(pdev, phost->Control.hc_num_out);
	return USBH_OK;
}

/**
 * @brief  USBH_Process
 *         USB Host core main state machine process
 * @param  None
 * @retval None
 */
#if 0
void USBH_Process(USB_OTG_CORE_HANDLE *pdev, USBH_HOST *phost)
{
	volatile USBH_Status status = USBH_FAIL;

	switch (phost->gState)
	{
		case HOST_ISSUE_CORE_RESET:

			if (HCD_ResetPort(pdev) == 0)
			{
				phost->gState = HOST_IDLE;
			}
			break;

		case HOST_IDLE:

			if (HCD_IsDeviceConnected(pdev))
			{
				/* Wait for USB Connect Interrupt void USBH_ISR_Connected(void) */
				USBH_DeAllocate_AllChannel(pdev);
				phost->gState = HOST_DEV_ATTACHED;
			}
			break;

		case HOST_DEV_ATTACHED:

			//    phost->usr_cb->DeviceAttached();
			pdev->host.port_cb->DisconnStatus = 0;
			pdev->host.port_cb->ConnHandled = 1;

			phost->Control.hc_num_out = USBH_Alloc_Channel(pdev, 0x00);
			phost->Control.hc_num_in = USBH_Alloc_Channel(pdev, 0x80);

			/* Reset USB Device */
			if (HCD_ResetPort(pdev) == 0)
			{
				phost->usr_cb->ResetDevice();
				/*  Wait for USB USBH_ISR_PrtEnDisableChange()
				Host is Now ready to start the Enumeration
				 */

				phost->device_prop.speed = HCD_GetCurrentSpeed(pdev);

				phost->gState = HOST_ENUMERATION;
				phost->usr_cb->DeviceSpeedDetected(phost->device_prop.speed);

				/* Open Control pipes */
				USBH_Open_Channel(pdev, phost->Control.hc_num_in, phost->device_prop.address, phost->device_prop.speed, EP_TYPE_CTRL, phost->Control.ep0size);

				/* Open Control pipes */
				USBH_Open_Channel(pdev, phost->Control.hc_num_out, phost->device_prop.address, phost->device_prop.speed, EP_TYPE_CTRL, phost->Control.ep0size);
			}
			break;

		case HOST_ENUMERATION:
			/* Check for enumeration status */
			if (USBH_HandleEnum(pdev, phost) == USBH_OK)
			{
				/* The function shall return USBH_OK when full enumeration is complete */

				/* user callback for end of device basic enumeration */
				phost->usr_cb->EnumerationDone();

				phost->gState = HOST_USR_INPUT;
			}
			break;

		case HOST_USR_INPUT:
			/*The function should return user response true to move to class state */
			if (phost->usr_cb->UserInput() == USBH_USR_RESP_OK)
			{
				if ((phost->class_cb->Init(pdev, phost)) \
					 == USBH_OK)
				{
					phost->gState = HOST_CLASS_REQUEST;
				}
			}
			break;

		case HOST_CLASS_REQUEST:
			/* process class standard contol requests state machine */
			status = phost->class_cb->Requests(pdev, phost);

			if (status == USBH_OK)
			{
				phost->gState = HOST_CLASS;
			}

			else
			{
				USBH_ErrorHandle(phost, status);
			}


			break;
		case HOST_CLASS:
			/* process class state machine */
			status = phost->class_cb->Machine(pdev, phost);
			USBH_ErrorHandle(phost, status);
			break;

		case HOST_CTRL_XFER:
			/* process control transfer state machine */
			USBH_HandleControl(pdev, phost);
			break;

		case HOST_SUSPENDED:
			break;

		case HOST_ERROR_STATE:
			/* Re-Initilaize Host for new Enumeration */
			USBH_DeInit(pdev, phost);
			phost->usr_cb->DeInit();
			phost->class_cb->DeInit(pdev, &phost->device_prop);
			break;

		default:
			break;
	}

	/* check device disconnection event */
	if (!(HCD_IsDeviceConnected(pdev)) && (pdev->host.port_cb->DisconnHandled == 0))
	{
		/* Manage User disconnect operations*/
		phost->usr_cb->DeviceDisconnected();

		pdev->host.port_cb->DisconnHandled = 1; /* Handle to avoid the Re
		-entry*/

		/* Re-Initilaize Host for new Enumeration */
		USBH_DeInit(pdev, phost);
		phost->usr_cb->DeInit();
		phost->class_cb->DeInit(pdev, &phost->device_prop);
	}
}

#endif 



/**
 * @brief  USBH_HandleEnum
 *         This function includes the complete enumeration process
 * @param  pdev: Selected device
 * @retval USBH_Status
 */
USBH_Status USBH_HandleEnum(USB_OTG_CORE_HANDLE *pdev, USBH_HOST *phost)
{
	USBH_Status Status = USBH_FAIL;
	uint8_t Local_Buffer[64];
	p_dbg_enter;

	Status = USBH_Get_DevDesc(pdev, phost, 8);
	if (Status != USBH_OK)
	{
		return Status;
	}

	phost->Control.ep0size = phost->device_prop.Dev_Desc.bMaxPacketSize;
	/* Issue Reset  */
	HCD_ResetPort(pdev);
	//    phost->EnumState = ENUM_GET_FULL_DEV_DESC;
	/* modify control channels configuration for MaxPacket size */
	USBH_Modify_Channel(pdev, phost->Control.hc_num_out, 0, 0, 0, phost->Control.ep0size);

	USBH_Modify_Channel(pdev, phost->Control.hc_num_in, 0, 0, 0, phost->Control.ep0size);

	if (USBH_Get_DevDesc(pdev, phost, USB_DEVICE_DESC_SIZE) \
		 != USBH_OK)
	{
		return Status;
	}


	if (USBH_SetAddress(pdev, phost, USBH_DEVICE_ADDRESS) != USBH_OK)
	{
		return Status;
	}

	phost->device_prop.address = USBH_DEVICE_ADDRESS;

	/* modify control channels to update device address */
	USBH_Modify_Channel(pdev, phost->Control.hc_num_in, phost->device_prop.address, 0, 0, 0);

	USBH_Modify_Channel(pdev, phost->Control.hc_num_out, phost->device_prop.address, 0, 0, 0);

	if (USBH_Get_CfgDesc(pdev, phost, USB_CONFIGURATION_DESC_SIZE) != USBH_OK)
	{
		return Status;
	}

	if (USBH_Get_CfgDesc(pdev, phost, phost->device_prop.Cfg_Desc.wTotalLength) != USBH_OK)
	{
		return Status;
	}

	if (phost->device_prop.Dev_Desc.iManufacturer != 0)
	{
		/* Check that Manufacturer String is available */

		if (USBH_Get_StringDesc(pdev, phost, phost->device_prop.Dev_Desc.iManufacturer, Local_Buffer, 0xff) != USBH_OK){

		}
	}

	if (phost->device_prop.Dev_Desc.iProduct != 0)
	{
		/* Check that Product string is available */
		if (USBH_Get_StringDesc(pdev, phost, phost->device_prop.Dev_Desc.iProduct, Local_Buffer, 0xff) == USBH_OK){

		}
	}

	if (phost->device_prop.Dev_Desc.iSerialNumber != 0)
	{
		/* Check that Serial number string is available */
		if (USBH_Get_StringDesc(pdev, phost, phost->device_prop.Dev_Desc.iSerialNumber, Local_Buffer, 0xff) == USBH_OK){

		}
	}

	if (USBH_SetCfg(pdev, phost, phost->device_prop.Cfg_Desc.bConfigurationValue) == USBH_OK){

	}
	return USBH_OK;
}

usb_complete_t Complete;
EXTERN_MONITOR_ITEM(bulk_in_complite);
EXTERN_MONITOR_ITEM(int_in_complite);
EXTERN_MONITOR_ITEM(iso_in_complite);
EXTERN_MONITOR_ITEM(bulk_out_complite);
EXTERN_MONITOR_ITEM(int_out_complite);
EXTERN_MONITOR_ITEM(iso_out_complite);
void wake_up_urb(USB_OTG_CORE_HANDLE *pdev, int num)
{

	if (pdev->host.hc[num].ep_type == USB_OTG_EP_CONTROL)
	{

		wake_up(USB_OTG_Core.crl_urb_wait);
	}
	else if (pdev->urb[num] && pdev->urb[num]->complete)
	{
		if (pdev->host.URB_State[num] == URB_DONE)
			pdev->urb[num]->status = 0;
		else
			pdev->urb[num]->status = pdev->host.URB_State[num];//? if  == URB_IDLE
		pdev->urb[num]->actual_length = pdev->host.hc[num].xfer_count;

		Complete = (usb_complete_t)pdev->urb[num]->complete;
		Complete((void*)pdev->urb[num]);
		
		//pdev->urb[num] = 0;

		if (!pdev->host.hc[num].ep_is_in){

			if(pdev->host.hc[num].ep_type == USB_OTG_EP_BULK){
				
				INC_MONITOR_ITEM_VALUE(bulk_out_complite);
				//mutex_unlock(pdev->bulk_out_urb_lock);
			}else if(pdev->host.hc[num].ep_type == USB_OTG_EP_INT){
				
				INC_MONITOR_ITEM_VALUE(int_out_complite);
				//mutex_unlock(pdev->int_out_urb_lock);
			}else if(pdev->host.hc[num].ep_type == USB_OTG_EP_ISOC){
				
				INC_MONITOR_ITEM_VALUE(iso_out_complite);
				//mutex_unlock(pdev->iso_out_urb_lock);
			}
			wake_up(pdev->bulkout_urb_wait);
			//mutex_unlock(pdev->bulk_out_urb_lock);
		}else{

			if(pdev->host.hc[num].ep_type == USB_OTG_EP_BULK){
				
				INC_MONITOR_ITEM_VALUE(bulk_in_complite);
			}else if(pdev->host.hc[num].ep_type == USB_OTG_EP_INT){
				
				INC_MONITOR_ITEM_VALUE(int_in_complite);
			}else if(pdev->host.hc[num].ep_type == USB_OTG_EP_ISOC){
				
				INC_MONITOR_ITEM_VALUE(iso_in_complite);
			}
		}
	}
	else
		p_err("*****\n");
}


URB_STATE usb_start_wait_urb(USB_OTG_CORE_HANDLE *pdev, int hc_num, int timeout)
{
	int ret;
	URB_STATE status;
	
	ret = wait_event_timeout(pdev->crl_urb_wait, timeout);
	if (ret  != 0)
	{
		USB_OTG_HCTSIZn_TypeDef hctsiz;
		USB_OTG_HNPTXSTS_TypeDef hnptxsts;
		hctsiz.d32 = USB_OTG_READ_REG32(&pdev->regs.HC_REGS[hc_num]->HCTSIZ);
		hnptxsts.d32 = USB_OTG_READ_REG32(&pdev->regs.GREGS->HNPTXSTS);

		p_err("hctsiz.xfersize:%d, hctsiz.pktcnt:%d, hnptxsts.fspc:%d, hnptxsts.qspc:%d", hctsiz.b.xfersize, hctsiz.b.pktcnt, hnptxsts.b.nptxfspcavail, hnptxsts.b.nptxqspcavail 

		);

		hctsiz.d32 = USB_OTG_READ_REG32(&pdev->regs.HC_REGS[3]->HCTSIZ);

		p_err("hctsiz.xfersize:%d, hctsiz.pktcnt:%d, %d, %d, %08x", hctsiz.b.xfersize, hctsiz.b.pktcnt, pdev->host.hc[3].packet_count, pdev->host.hc[3].xfer_len, ret);

		p_err("usb_start_wait_urb timeout:%d\n", hc_num);


	}
	status = HCD_GetURB_State(pdev, hc_num);
	if (status != URB_DONE)
		p_err("usb_start_wait_urb ret err:%d\n", status);
	return status;
}

URB_STATE USBH_HandleControl_Setup(USB_OTG_CORE_HANDLE *pdev, USBH_HOST *phost, int timeout)
{
	USBH_Status state;
	state = USBH_CtlSendSetup(pdev, phost->Control.setup.d8, phost->Control.hc_num_out);

	if (state != USBH_OK)
	{
		p_err("USBH_HandleControl_Setup err:%d\n", state);
		return URB_ERROR;
	}

	return URB_DONE;
}

URB_STATE USBH_HandleControl_Data_in(USB_OTG_CORE_HANDLE *pdev, USBH_HOST *phost, int timeout)
{
	USBH_Status state;
	state = USBH_CtlReceiveData(pdev, phost->Control.buff, phost->Control.length, phost->Control.hc_num_in);

	if (state != USBH_OK)
	{
		p_err("USBH_HandleControl_Data_in err:%d\n", state);
		return URB_ERROR;
	}

	return URB_DONE;
}

URB_STATE USBH_HandleControl_Data_out(USB_OTG_CORE_HANDLE *pdev, USBH_HOST *phost, int timeout)
{
	USBH_Status state;

	pdev->host.hc[phost->Control.hc_num_out].toggle_out ^= 1;

	state = USBH_CtlSendData(pdev, phost->Control.buff, phost->Control.length, phost->Control.hc_num_out);

	if (state != USBH_OK)
	{
		p_err("USBH_HandleControl_Data_out err:%d\n", state);
		return URB_ERROR;
	}

	return URB_DONE;
}

URB_STATE USBH_HandleControl_Status_in(USB_OTG_CORE_HANDLE *pdev, USBH_HOST *phost, int timeout)
{
	USBH_Status state;

	state = USBH_CtlReceiveData(pdev, 0, 0, phost->Control.hc_num_in);

	if (state != USBH_OK)
	{
		p_err("USBH_HandleControl_Status_in err:%d\n", state);
		return URB_ERROR;
	}

	return URB_DONE;
}

URB_STATE USBH_HandleControl_Status_out(USB_OTG_CORE_HANDLE *pdev, USBH_HOST *phost, int timeout)
{
	USBH_Status state;
	pdev->host.hc[phost->Control.hc_num_out].toggle_out ^= 1;
	state = USBH_CtlSendData(pdev, 0, 0, phost->Control.hc_num_out);

	if (state != USBH_OK)
	{
		p_err("USBH_HandleControl_Status_out err:%d\n", state);
		return URB_ERROR;
	}

	return URB_DONE;
}

/**
 * @brief  USBH_HandleControl
 *         Handles the USB control transfer state machine
 * @param  pdev: Selected device
 * @retval Status
 */
#define WAIT_URB_TIMEOUT	3000

USBH_Status USBH_HandleControl(USB_OTG_CORE_HANDLE *pdev, USBH_HOST *phost)
{
	uint8_t direction;
	static uint16_t timeout = 0;
	URB_STATE URB_Status = URB_IDLE;

	phost->Control.status = CTRL_START;

	URB_Status = USBH_HandleControl_Setup(pdev, phost, WAIT_URB_TIMEOUT);
	if (URB_Status != URB_DONE)
		return USBH_FAIL;

	direction = (phost->Control.setup.b.bmRequestType &USB_REQ_DIR_MASK);

	/* check if there is a data stage */
	if (phost->Control.setup.b.wLength.w != 0)
	{
		timeout = DATA_STAGE_TIMEOUT;
		if (direction == USB_D2H)
		{
			URB_Status = USBH_HandleControl_Data_in(pdev, phost, timeout);
			if (URB_DONE != URB_Status)
			{
				return USBH_FAIL;
			}

			URB_Status = USBH_HandleControl_Status_out(pdev, phost, timeout);
			if (URB_DONE != URB_Status)
			{
				return USBH_FAIL;
			}
		}
		else
		{
			/* Data Direction is OUT */
			URB_Status = USBH_HandleControl_Data_out(pdev, phost, timeout);
			if (URB_DONE != URB_Status)
			{
				return USBH_FAIL;
			}

			URB_Status = USBH_HandleControl_Status_in(pdev, phost, timeout);
			if (URB_DONE != URB_Status)
			{
				return USBH_FAIL;
			}
		}
	}
	/* No DATA stage */
	else
	{
		timeout = NODATA_STAGE_TIMEOUT;

		/* If there is No Data Transfer Stage */
		if (direction == USB_D2H)
		{
			/* Data Direction is IN */
			URB_Status = USBH_HandleControl_Status_out(pdev, phost, timeout);
			if (URB_DONE != URB_Status)
			{
				return USBH_FAIL;
			}
		}
		else
		{
			/* Data Direction is OUT */
			URB_Status = USBH_HandleControl_Status_in(pdev, phost, timeout);
			if (URB_DONE != URB_Status)
			{
				return USBH_FAIL;
			}
		}
	}
	return USBH_OK;
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

 /******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

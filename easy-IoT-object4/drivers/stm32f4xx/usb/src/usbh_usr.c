/**
 ******************************************************************************
 * @file    usbh_usr.c
 * @author  MCD Application Team
 * @version V1.0.0
 * @date    22/07/2011
 * @brief   This file includes the usb host user callbacks
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
#define DEBUG
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

#include "usbh_ioreq.h"
#include "usbh_hcs.h"
#include "usbh_msc_bot.h"
#include "usbh_msc_core.h"
#include "ff.h"
struct usb_device usb_dev;
struct usb_host_config usb_config;
struct usb_interface usb_intfc[USBH_MAX_NUM_INTERFACES];
struct usb_host_interface host_intfc[USBH_MAX_NUM_INTERFACES];

extern struct urb *bulk_out_urb;
extern struct urb *bulk_in_urb;

void init_usb_struct(USB_OTG_CORE_HANDLE *pdev, USBH_HOST *phost)
{
	int i, j;
	memset(&usb_dev, 0, sizeof(struct usb_device));
	memset(&usb_config, 0, sizeof(struct usb_host_config));
	memset(&usb_intfc, 0, sizeof(struct usb_interface) *USBH_MAX_NUM_INTERFACES);
	memset(&host_intfc, 0, sizeof(struct usb_host_interface));

	usb_dev.descriptor = &phost->device_prop.Dev_Desc;
	usb_dev.config = &usb_config;

	usb_config.desc = &phost->device_prop.Cfg_Desc;
	pdev->dev = &usb_dev;

	for (i = 0; i < USBH_MAX_NUM_INTERFACES; i++)
	{
		usb_config.interface[i] = &usb_intfc[i];

		usb_intfc[i].altsetting = usb_intfc[i].cur_altsetting = &host_intfc[i];

		host_intfc[i].desc = &phost->device_prop.Itf_Desc[i];

		host_intfc[i].endpoint = usb_dev.ep;


	} for (j = 0; j < USBH_MAX_NUM_ENDPOINTS; j++)
	{
		usb_dev.ep[j].desc = &phost->device_prop.Ep_Desc[j];
	}
	//sys_cfg.usb_dev = &usb_dev;
}

extern USB_OTG_CORE_HANDLE USB_OTG_Core;
extern USBH_HOST USB_Host;

USB_OTG_CORE_HANDLE *p_usb_dev = &USB_OTG_Core;


DECLARE_MONITOR_ITEM("control cnt", control_cnt);
int usb_control_msg(unsigned int pipe, u8 request, u8 requesttype, u16 value, u16 index, void *data, u16 size, int timeout)
{
	int ret;
	mutex_lock(USB_OTG_Core.ctrl_urb_lock);
	USB_Host.Control.setup.b.bmRequestType = requesttype;
	USB_Host.Control.setup.b.bRequest = request;
	USB_Host.Control.setup.b.wValue.w = value;
	USB_Host.Control.setup.b.wIndex.w = index;
	USB_Host.Control.setup.b.wLength.w = size;

	INC_MONITOR_ITEM_VALUE(control_cnt);
	ret = USBH_CtlReq(&USB_OTG_Core, &USB_Host, data, size);
	if (ret != USBH_OK)
		ret =  - 1;
	else
		ret = 0;
	mutex_unlock(USB_OTG_Core.ctrl_urb_lock);
	return ret;
}

void usb_fill_bulk_urb(struct urb *urb, struct usb_device *dev, unsigned int pipe, void *transfer_buffer, int buffer_length, void *complete_fn, void *context)
{
	urb->dev = dev;
	urb->pipe = pipe;
	urb->transfer_buffer = transfer_buffer;
	urb->transfer_buffer_length = buffer_length;
	urb->complete = complete_fn;
	urb->context = context;
} 

void usb_fill_int_urb(struct urb *urb, struct usb_device *dev, unsigned int pipe, void *transfer_buffer, int buffer_length, usb_complete_t complete_fn, void *context, int interval)
{
	//if(urb->context)
	//	kfree_skb(urb->context);
	
	urb->dev = dev;
	urb->pipe = pipe;
	urb->transfer_buffer = transfer_buffer;
	urb->transfer_buffer_length = buffer_length;
	urb->complete = complete_fn;
	urb->context = context;
	//if (dev->speed == USB_SPEED_HIGH)
	//	urb->interval = 1 << (interval - 1);
	//else
	urb->interval = interval;
	urb->start_frame =  - 1;
} 

DECLARE_MONITOR_ITEM("bulk out cnt", bulk_out_cnt);
DECLARE_MONITOR_ITEM("bulk out complite", bulk_out_complite);
DECLARE_MONITOR_ITEM("bulk in cnt", bulk_in_cnt);
DECLARE_MONITOR_ITEM("bulk in complite", bulk_in_complite);

DECLARE_MONITOR_ITEM("int out cnt", int_out_cnt);
DECLARE_MONITOR_ITEM("int out complite", int_out_complite);
DECLARE_MONITOR_ITEM("int in cnt", int_in_cnt);

DECLARE_MONITOR_ITEM("int in complite", int_in_complite);

DECLARE_MONITOR_ITEM("iso out cnt", iso_out_cnt);
DECLARE_MONITOR_ITEM("iso out complite", iso_out_complite);
DECLARE_MONITOR_ITEM("iso in cnt", iso_in_cnt);
DECLARE_MONITOR_ITEM("iso in complite", iso_in_complite);

int usb_submit_urb(struct urb *urb, gfp_t mem_flags)
{
	struct usb_device *dev;
	USB_OTG_HC *hc;
	USBH_Status stat;

	if (!urb /* || urb->hcpriv*/ || !urb->complete)
		return  - EINVAL;

	urb->actual_length = 0;
	
	dev = urb->dev;
	if ((!dev))
		return  - ENODEV;

	hc = &USB_OTG_Core.host.hc[urb->pipe];

	if(hc->ep_type == USB_OTG_EP_CONTROL)
	{
		p_err_fun;
		return  - EINVAL;
	}
	if(hc->ep_type == USB_OTG_EP_BULK)
	{
		if (!hc->ep_is_in)
		{
			INC_MONITOR_ITEM_VALUE(bulk_out_cnt);
			stat = USBH_BulkSendData(urb, &USB_OTG_Core, urb->transfer_buffer, urb->transfer_buffer_length, urb->pipe);
		} 
		else
		{
			INC_MONITOR_ITEM_VALUE(bulk_in_cnt);
			stat = USBH_BulkReceiveData(urb, &USB_OTG_Core, urb->transfer_buffer, urb->transfer_buffer_length, urb->pipe);
		}
		return stat;
	}
	if(hc->ep_type == USB_OTG_EP_INT)
	{
		if (!hc->ep_is_in)
		{
			INC_MONITOR_ITEM_VALUE(int_out_cnt);
			stat = USBH_InterruptSendData(urb, &USB_OTG_Core, urb->transfer_buffer, urb->transfer_buffer_length, urb->pipe);
		} 
		else
		{
			INC_MONITOR_ITEM_VALUE(int_in_cnt);
			stat = USBH_InterruptReceiveData(urb, &USB_OTG_Core, urb->transfer_buffer, urb->transfer_buffer_length, urb->pipe);
		}
		return stat;
	}
	if(hc->ep_type == USB_OTG_EP_ISOC)
	{
		if (!hc->ep_is_in)
		{
			INC_MONITOR_ITEM_VALUE(iso_out_cnt);
			stat = USBH_IsocSendData(urb, &USB_OTG_Core, urb->transfer_buffer, urb->transfer_buffer_length, urb->pipe);
		} 
		else
		{
			INC_MONITOR_ITEM_VALUE(iso_in_cnt);
			stat = USBH_IsocReceiveData(urb, &USB_OTG_Core, urb->transfer_buffer, urb->transfer_buffer_length, urb->pipe);
		}
		return stat;
	}
	return stat;
}

void usb_kill_urb(struct urb *urb)
{
	p_usb_dev->host.HC_Status[urb->pipe] = HC_HALTED;
	p_usb_dev->regs.HC_REGS[urb->pipe]->HCGINTMSK |= USB_OTG_HCINTMSK_CHHM;
	USB_OTG_HC_Halt(p_usb_dev, urb->pipe);
} 

#define USB_OTG_HCCHAR_CHDIS                   ((uint32_t)0x40000000)            /*!< Channel disable */
#define USB_OTG_HCCHAR_CHENA                   ((uint32_t)0x80000000)            /*!< Channel enable */
void reactive_urb(struct urb *urb)
{
	p_usb_dev->regs.HC_REGS[urb->pipe]->HCCHAR &= ~USB_OTG_HCCHAR_CHDIS;
	p_usb_dev->regs.HC_REGS[urb->pipe]->HCCHAR |= USB_OTG_HCCHAR_CHENA;    
}

/*
 * @brief  USB枚举
 * 
 * 
 */
int usb_enumeration(USB_OTG_CORE_HANDLE *p_dev, USBH_HOST *p_host)
{
	int ret =  - 1;
	int i = 0, j, ep_cnt = 0;
	p_dbg_enter;
    	if (HCD_ResetPort(p_dev) != 0)
	{
		p_err("HCD_ResetPort err\n");
		goto end;
	}
	sleep(200);
	if (!HCD_IsDeviceConnected(p_dev))
	{
		p_err("device not conneted\n");
		return -1;;
	}

	USBH_DeAllocate_AllChannel(p_dev);

	p_host->Control.hc_num_out = USBH_Alloc_Channel(p_dev, 0x00);

	p_host->Control.hc_num_in = USBH_Alloc_Channel(p_dev, 0x80);

	/* Reset USB Device */
	if (HCD_ResetPort(p_dev) == 0)
	{
		sleep(500);
		p_host->device_prop.speed = HCD_GetCurrentSpeed(p_dev);
		p_dbg("speed %d\n", p_host->device_prop.speed);

		/* Open Control pipes */
		USBH_Open_Channel(p_dev, p_host->Control.hc_num_in, p_host->device_prop.address, p_host->device_prop.speed, EP_TYPE_CTRL, p_host->Control.ep0size);

		/* Open Control pipes */
		USBH_Open_Channel(p_dev, p_host->Control.hc_num_out, p_host->device_prop.address, p_host->device_prop.speed, EP_TYPE_CTRL, p_host->Control.ep0size);
	}
	else
	{
		p_err("HCD_ResetPort err\n");
		goto end;
	}
	if (USBH_HandleEnum(p_dev, p_host) != USBH_OK)
	{
		p_err("USBH_HandleEnum faild\n");
		goto end;
	}

	ep_cnt = 0;
	for(j = 0; j < p_host->device_prop.Cfg_Desc.bNumInterfaces; j++)
	{
		for (i = 0; i < p_host->device_prop.Itf_Desc[j].bNumEndpoints; i++)
		{
			int hc_num;
			hc_num = USBH_Alloc_Channel(p_dev, p_host->device_prop.Ep_Desc[ep_cnt].bEndpointAddress);

			/* Open channel for IN endpoint */
			USBH_Open_Channel(p_dev, hc_num, p_host->device_prop.address, p_host->device_prop.speed, p_host->device_prop.Ep_Desc[ep_cnt].bmAttributes, p_host->device_prop.Ep_Desc[ep_cnt].wMaxPacketSize);
			ep_cnt++;
		}
	}

	ret = ERR_OK;
end: 
	if (ret != ERR_OK)
		p_err("usb_enumeration faild\n");
	return ret;
}

/**
  * @brief  USBH_FindInterface 
  *         Find the interface index for a specific class.
  * @param  phost: Host Handle
  * @param  Class: Class code
  * @param  SubClass: SubClass code
  * @param  Protocol: Protocol code
  * @retval interface index in the configuration structure
  * @note : (1)interface index 0xFF means interface index not found
  */

uint8_t  USBH_FindInterface(USBH_HOST *phost, uint8_t Class, uint8_t SubClass, uint8_t Protocol)
{
  int i;

  for(i =0; i < phost->device_prop.Cfg_Desc.bNumInterfaces; i++)
  {
  	if(phost->device_prop.Itf_Desc[i].bInterfaceClass == Class && 
		phost->device_prop.Itf_Desc[i].bInterfaceSubClass == SubClass/* && 
		phost->device_prop.Itf_Desc[i].bInterfaceProtocol == Protocol*/)
		return i;
  }

  return 0xFF;
}

/*
 * @brief  轮询定时
 * 每隔500ms轮询一次USB设备,检测到则进行枚举
 * 如果是MSC设备则进行相关的初始化
 * 
 */
void usb_polling_timer(void *arg)
{
	int ret;
	static bool saved_usb_stat = 0;
	USB_OTG_CORE_HANDLE *p_dev = (USB_OTG_CORE_HANDLE *)arg;

	if(HCD_IsDeviceConnected(p_dev) && !saved_usb_stat)
	{
		p_dbg("usb connect");
		saved_usb_stat = 1;
		USB_OTG_BSP_DisableInterrupt(p_dev);
		//memset(&USB_Host, 0, sizeof(USB_Host));
		USBH_DeInit(p_dev, &USB_Host);
		
		/* Start the USB OTG core */
		HCD_Init(p_dev, USB_OTG_HS_CORE_ID);

		/* Enable Interrupts */
		USB_OTG_BSP_EnableInterrupt(p_dev);
		
		ret = usb_enumeration(p_dev, &USB_Host);
		if(!ret)
		{
			//针对msc设备
			if(USBH_FindInterface(&USB_Host, MSC_CLASS, MSC_SCSI_SUB_CLASS, 0) != 0XFF)
			{
				ret = init_msc(&USB_OTG_Core, &USB_Host);
				if(!ret)
				{
					p_dbg("Udisk init OK, try to mount the file system");
					test_mount_fs();
				}
			}else{
				p_err("msc device not find");
			}
		}
	}

	if(!HCD_IsDeviceConnected(p_dev) && saved_usb_stat)
	{
		extern FATFS USBDISKFatFs;
		p_dbg("usb disconnect");
		
		USBDISKFatFs.fs_type = 0;
		saved_usb_stat = 0;
		test_unmount_fs();
	}
}

/*
 * @brief  初始化USB主机和MSC
 * 用的是HS,高速usb的全速模式
 * 这里还不对设备进行枚举,等检测到设备插上的时候再枚举
 */
int init_usb()
{
	USBH_Init(&USB_OTG_Core, USB_OTG_HS_CORE_ID, &USB_Host, 0, 0);
	
	USBH_MSC_Init(&USB_OTG_Core);
	
	USB_OTG_Core.polling_timer = timer_setup(500, 1, usb_polling_timer, &USB_OTG_Core);
	if(!USB_OTG_Core.polling_timer)
		return -1;

	add_timer(USB_OTG_Core.polling_timer);

	return 0;
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF
       FILE****/

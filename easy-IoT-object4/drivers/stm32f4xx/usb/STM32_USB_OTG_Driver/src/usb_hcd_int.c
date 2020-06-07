/**
 ******************************************************************************
 * @file    usb_hcd_int.c
 * @author  MCD Application Team
 * @version V2.0.0
 * @date    22-July-2011
 * @brief   Host driver interrupt subroutines
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
#include "usbh_hcs.h"
#include "usb_bsp.h"
#include "usb_dcd.h"

#include "usbh_ioreq.h"

#if defined   (__CC_ARM) /*!< ARM Compiler */
#pragma O0
#elif defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma O0
#elif defined (__GNUC__) /*!< GNU Compiler */
#pragma GCC optimize ("O0")
#elif defined  (__TASKING__) /*!< TASKING Compiler */
#pragma optimize=0

#endif /* __CC_ARM */

/** @addtogroup USB_OTG_DRIVER
 * @{
 */

/** @defgroup USB_HCD_INT
 * @brief This file contains the interrupt subroutines for the Host mode.
 * @{
 */


/** @defgroup USB_HCD_INT_Private_Defines
 * @{
 */
/**
 * @}
 */


/** @defgroup USB_HCD_INT_Private_TypesDefinitions
 * @{
 */
/**
 * @}
 */



/** @defgroup USB_HCD_INT_Private_Macros
 * @{
 */
/**
 * @}
 */


/** @defgroup USB_HCD_INT_Private_Variables
 * @{
 */
/**
 * @}
 */


/** @defgroup USB_HCD_INT_Private_FunctionPrototypes
 * @{
 */

static uint32_t USB_OTG_USBH_handle_sof_ISR(USB_OTG_CORE_HANDLE *pdev);
static uint32_t USB_OTG_USBH_handle_port_ISR(USB_OTG_CORE_HANDLE *pdev);
static uint32_t USB_OTG_USBH_handle_hc_ISR(USB_OTG_CORE_HANDLE *pdev);
static uint32_t USB_OTG_USBH_handle_hc_n_In_ISR(USB_OTG_CORE_HANDLE *pdev, uint32_t num);
static uint32_t USB_OTG_USBH_handle_hc_n_Out_ISR(USB_OTG_CORE_HANDLE *pdev, uint32_t num);
static uint32_t USB_OTG_USBH_handle_rx_qlvl_ISR(USB_OTG_CORE_HANDLE *pdev);
static uint32_t USB_OTG_USBH_handle_nptxfempty_ISR(USB_OTG_CORE_HANDLE *pdev);
static uint32_t USB_OTG_USBH_handle_ptxfempty_ISR(USB_OTG_CORE_HANDLE *pdev);
static uint32_t USB_OTG_USBH_handle_Disconnect_ISR(USB_OTG_CORE_HANDLE *pdev);
static uint32_t USB_OTG_USBH_handle_IncompletePeriodicXfer_ISR(USB_OTG_CORE_HANDLE *pdev);

/**
 * @}
 */


/** @defgroup USB_HCD_INT_Private_Functions
 * @{
 */

/**
 * @brief  HOST_Handle_ISR
 *         This function handles all USB Host Interrupts
 * @param  pdev: Selected device
 * @retval status
 */
extern USB_OTG_CORE_HANDLE USB_OTG_Core;
DECLARE_MONITOR_ITEM("USB ISR", usb_isr);


void USBH_OTG_ISR_Handler()
{
	USB_OTG_GINTSTS_TypeDef gintsts;
	uint32_t retval = 0;
	USB_OTG_CORE_HANDLE *pdev = &USB_OTG_Core;
    	INC_MONITOR_ITEM_VALUE(usb_isr);
	enter_interrupt();

	gintsts.d32 = USB_OTG_READ_REG32(&pdev->regs.GREGS->GINTSTS);
	gintsts.d32 &= USB_OTG_READ_REG32(&pdev->regs.GREGS->GINTMSK);
	
	if (!gintsts.d32)
	{
		exit_interrupt(0);
		return ;
	}

	if (gintsts.b.rxstsqlvl)
	{
		gintsts.b.rxstsqlvl = 0;
		USB_OTG_USBH_handle_rx_qlvl_ISR(pdev);
	}

	if (gintsts.b.nptxfempty)
	{
		gintsts.b.nptxfempty = 0;
		USB_OTG_USBH_handle_nptxfempty_ISR(pdev);
	}

	if (gintsts.b.hcintr)
	{
		
		gintsts.b.hcintr = 0;
		retval = USB_OTG_USBH_handle_hc_ISR(pdev);
	}

	if (!gintsts.d32)
		goto end;

	if (gintsts.b.sofintr)
	{
		USB_OTG_USBH_handle_sof_ISR(pdev);
	}

	if (gintsts.b.ptxfempty)
	{
		USB_OTG_USBH_handle_ptxfempty_ISR(pdev);
	}

	if (gintsts.b.portintr)
	{
		USB_OTG_USBH_handle_port_ISR(pdev);
	}

	if (gintsts.b.disconnect)
	{
		USB_OTG_USBH_handle_Disconnect_ISR(pdev);

	}

	if (gintsts.b.incomplisoout)
	{
		USB_OTG_USBH_handle_IncompletePeriodicXfer_ISR(pdev);
	}

	if (gintsts.b.datafsusp){

	}

end: 
	
	exit_interrupt(retval);
}

/**
 * @brief  USB_OTG_USBH_handle_hc_ISR
 *         This function indicates that one or more host channels has a pending
 * @param  pdev: Selected device
 * @retval status
 */
extern USBH_HOST USB_Host;

static uint32_t __inline USB_OTG_USBH_handle_hc_ISR(USB_OTG_CORE_HANDLE *pdev)
{
	USB_OTG_HAINT_TypeDef haint;
	USB_OTG_HCCHAR_TypeDef hcchar;
	uint32_t i = 0;
	uint32_t retval = 0, need_wake_up = 0;

	/* Clear appropriate bits in HCINTn to clear the interrupt bit in
	 * GINTSTS */
	while((haint.d32 = USB_OTG_READ_REG32(&pdev->regs.HREGS->HAINT)) != 0)
	{
		for (i = 0; i < 8; i++)
		{
			if (haint.b.chint &(1 << i))
			{
				hcchar.d32 = USB_OTG_READ_REG32(&pdev->regs.HC_REGS[i]->HCCHAR);
				if (hcchar.b.epdir)
					retval = USB_OTG_USBH_handle_hc_n_In_ISR(pdev, i);
				else
					retval = USB_OTG_USBH_handle_hc_n_Out_ISR(pdev, i);
				if (retval)
				{
					need_wake_up = 1;
					wake_up_urb(pdev, i);
				}
			}
		}
	}
	
	return need_wake_up;
}

/**
 * @brief  USB_OTG_otg_hcd_handle_sof_intr
 *         Handles the start-of-frame interrupt in host mode.
 * @param  pdev: Selected device
 * @retval status
 */
static uint32_t USB_OTG_USBH_handle_sof_ISR(USB_OTG_CORE_HANDLE *pdev)
{
	USB_OTG_GINTSTS_TypeDef gintsts;


	gintsts.d32 = 0;
	/* Clear interrupt */
	gintsts.b.sofintr = 1;
	USB_OTG_WRITE_REG32(&pdev->regs.GREGS->GINTSTS, gintsts.d32);

	return 1;
}

/**
 * @brief  USB_OTG_USBH_handle_Disconnect_ISR
 *         Handles disconnect event.
 * @param  pdev: Selected device
 * @retval status
 */
static uint32_t USB_OTG_USBH_handle_Disconnect_ISR(USB_OTG_CORE_HANDLE *pdev)
{
	USB_OTG_GINTSTS_TypeDef gintsts;

	pdev->host.ConnSts = 0;
	gintsts.d32 = 0;

	//  pdev->host.port_cb->Disconnect(pdev);

	/* Clear interrupt */
	gintsts.b.disconnect = 1;
	USB_OTG_WRITE_REG32(&pdev->regs.GREGS->GINTSTS, gintsts.d32);

	return 1;
}

/**
 * @brief  USB_OTG_USBH_handle_nptxfempty_ISR
 *         Handles non periodic tx fifo empty.
 * @param  pdev: Selected device
 * @retval status
 */
 DECLARE_MONITOR_ITEM("nptxfempty", nptxfempty);
static uint32_t USB_OTG_USBH_handle_nptxfempty_ISR(USB_OTG_CORE_HANDLE *pdev)
{
	int free_space;
	USB_OTG_HNPTXSTS_TypeDef hnptxsts;
	uint32_t write_len;
	char *write_buff;

	hnptxsts.d32 = USB_OTG_READ_REG32(&pdev->regs.GREGS->HNPTXSTS);
	hnptxsts.b.chnum = pdev->cur_bulk_out_ch;

	write_len = (pdev->host.hc[hnptxsts.b.chnum].xfer_len + 3) / 4;

	free_space = hnptxsts.b.nptxfspcavail/*- 16*/;

	INC_MONITOR_ITEM_VALUE(nptxfempty);

	if (free_space < write_len)
	{
		write_len = free_space * 4;
		write_buff = (char*)pdev->host.hc[hnptxsts.b.chnum].xfer_buff;
		pdev->host.hc[hnptxsts.b.chnum].xfer_len -= write_len;
		pdev->host.hc[hnptxsts.b.chnum].xfer_buff += write_len;
	}
	else
	{
		USB_OTG_GINTMSK_TypeDef intmsk;
		intmsk.d32 = 0;
		intmsk.b.nptxfempty = 1;
		USB_OTG_MODIFY_REG32(&pdev->regs.GREGS->GINTMSK, intmsk.d32, 0);

		write_buff = (char*)pdev->host.hc[hnptxsts.b.chnum].xfer_buff;
		write_len = write_len * 4;
		pdev->host.hc[hnptxsts.b.chnum].xfer_len = 0;
		pdev->bulk_out_irq_pending = 0;
	}


	USB_OTG_WritePacket(pdev, (uint8_t*)write_buff, hnptxsts.b.chnum, write_len);

	if (pdev->host.hc[hnptxsts.b.chnum].xfer_len < 0)
	{
		pdev->host.hc[hnptxsts.b.chnum].xfer_len = 0;
	}

	return 1;
}


static uint32_t USB_OTG_USBH_handle_ptxfempty_ISR(USB_OTG_CORE_HANDLE *pdev)
{
	int free_space;
	USB_OTG_HPTXSTS_TypeDef hptxsts;
	uint32_t write_len;
	char *write_buff;

	hptxsts.d32 = USB_OTG_READ_REG32(&pdev->regs.HREGS->HPTXSTS);
	hptxsts.b.chnum = pdev->cur_bulk_out_ch;

	write_len = (pdev->host.hc[hptxsts.b.chnum].xfer_len + 3) / 4;

	free_space = hptxsts.b.ptxfspcavail/* - 16*/;
	if (hptxsts.b.ptxfspcavail < write_len)
	{
		write_len = free_space * 4;
		write_buff = (char*)pdev->host.hc[hptxsts.b.chnum].xfer_buff;
		pdev->host.hc[hptxsts.b.chnum].xfer_len -= write_len;
		pdev->host.hc[hptxsts.b.chnum].xfer_buff += write_len;
	}
	else
	{
		USB_OTG_GINTMSK_TypeDef intmsk;
		intmsk.d32 = 0;
		intmsk.b.ptxfempty = 1;
		USB_OTG_MODIFY_REG32(&pdev->regs.GREGS->GINTMSK, intmsk.d32, 0);

		write_buff = (char*)pdev->host.hc[hptxsts.b.chnum].xfer_buff;
		write_len = pdev->host.hc[hptxsts.b.chnum].xfer_len;

		pdev->host.hc[hptxsts.b.chnum].xfer_len = 0;
		pdev->host.hc[hptxsts.b.chnum].xfer_buff += write_len;
		pdev->int_out_irq_pending = 0;
	}

	USB_OTG_WritePacket(pdev, (uint8_t*)write_buff, hptxsts.b.chnum, write_len);

	if (pdev->host.hc[hptxsts.b.chnum].xfer_len < 0)
	{
		pdev->host.hc[hptxsts.b.chnum].xfer_len = 0;
	}

	return 1;
}



/**
 * @brief  USB_OTG_USBH_handle_port_ISR
 *         This function determines which interrupt conditions have occurred
 * @param  pdev: Selected device
 * @retval status
 */
static uint32_t USB_OTG_USBH_handle_port_ISR(USB_OTG_CORE_HANDLE *pdev)
{
	USB_OTG_HPRT0_TypeDef hprt0;
	USB_OTG_HPRT0_TypeDef hprt0_dup;
	USB_OTG_HCFG_TypeDef hcfg;
	uint32_t do_reset = 0;
	uint32_t retval = 0;

	hcfg.d32 = 0;
	hprt0.d32 = 0;
	hprt0_dup.d32 = 0;

	hprt0.d32 = USB_OTG_READ_REG32(pdev->regs.HPRT0);
	hprt0_dup.d32 = USB_OTG_READ_REG32(pdev->regs.HPRT0);

	/* Clear the interrupt bits in GINTSTS */

	hprt0_dup.b.prtena = 0;
	hprt0_dup.b.prtconndet = 0;
	hprt0_dup.b.prtenchng = 0;
	hprt0_dup.b.prtovrcurrchng = 0;

	/* Port Connect Detected */
	if (hprt0.b.prtconndet)
	{
		//    pdev->host.port_cb->Connect(pdev);
		hprt0_dup.b.prtconndet = 1;
		do_reset = 1;
		retval |= 1;
	}

	/* Port Enable Changed */
	if (hprt0.b.prtenchng)
	{
		hprt0_dup.b.prtenchng = 1;
		if (hprt0.b.prtena == 1)
		{
			pdev->host.ConnSts = 1;

			if ((hprt0.b.prtspd == HPRT0_PRTSPD_LOW_SPEED) || (hprt0.b.prtspd == HPRT0_PRTSPD_FULL_SPEED))
			{

				hcfg.d32 = USB_OTG_READ_REG32(&pdev->regs.HREGS->HCFG);

				if (hprt0.b.prtspd == HPRT0_PRTSPD_LOW_SPEED)
				{
					USB_OTG_WRITE_REG32(&pdev->regs.HREGS->HFIR, 6000);
					if (hcfg.b.fslspclksel != HCFG_6_MHZ)
					{
						if (pdev->cfg.coreID == USB_OTG_FS_CORE_ID)
						{
							USB_OTG_InitFSLSPClkSel(pdev, HCFG_6_MHZ);
						}
						do_reset = 1;
					}
				}
				else
				{

					USB_OTG_WRITE_REG32(&pdev->regs.HREGS->HFIR, 48000);
					if (hcfg.b.fslspclksel != HCFG_48_MHZ)
					{
						USB_OTG_InitFSLSPClkSel(pdev, HCFG_48_MHZ);
						do_reset = 1;
					}
				}
			}
			else
			{
				do_reset = 1;
			}
		}
	}
	/* Overcurrent Change Interrupt */
	if (hprt0.b.prtovrcurrchng)
	{
		hprt0_dup.b.prtovrcurrchng = 1;
		retval |= 1;
	}
	if (do_reset)
	{
		USB_OTG_ResetPort(pdev);

	}
	/* Clear Port Interrupts */
	USB_OTG_WRITE_REG32(pdev->regs.HPRT0, hprt0_dup.d32);

	return retval;
}

/**
 * @brief  USB_OTG_USBH_handle_hc_n_Out_ISR
 *         Handles interrupt for a specific Host Channel
 * @param  pdev: Selected device
 * @param  hc_num: Channel number
 * @retval status
 */
#define __HAL_HCD_MASK_HALT_HC_INT()             hcreg->HCGINTMSK &= ~USB_OTG_HCINTMSK_CHHM 
#define __HAL_HCD_UNMASK_HALT_HC_INT()           hcreg->HCGINTMSK |= USB_OTG_HCINTMSK_CHHM 


DECLARE_MONITOR_ITEM("bulk out comp", bulk_out_comp);
uint32_t USB_OTG_USBH_handle_hc_n_Out_ISR(USB_OTG_CORE_HANDLE *pdev, uint32_t num)
{
	USB_OTG_HC_REGS *hcreg;
	USB_OTG_HCCHAR_TypeDef hcchar;
	int ret = 0;
	hcreg = pdev->regs.HC_REGS[num];
	
	hcchar.d32 = USB_OTG_READ_REG32(&hcreg->HCCHAR);

	if (hcreg->HCINT & USB_OTG_HCINT_XFRC)
	{
		hcreg->HCINT = USB_OTG_HCINT_XFRC;
		
		pdev->host.ErrCnt[num] = 0;
		__HAL_HCD_UNMASK_HALT_HC_INT();
		USB_OTG_HC_Halt(pdev, num);
		pdev->host.HC_Status[num] = HC_XFRC;

		if(hcchar.b.eptype == EP_TYPE_BULK)
			INC_MONITOR_VALUE(bulk_out_comp);
		
	}
	else if (hcreg->HCINT & USB_OTG_HCINT_AHBERR)
	{
		hcreg->HCINT = USB_OTG_HCINT_AHBERR;
	}
	else if (hcreg->HCINT & USB_OTG_HCINT_FRMOR)
	{
		hcreg->HCINT = USB_OTG_HCINT_FRMOR;
		__HAL_HCD_UNMASK_HALT_HC_INT();
		USB_OTG_HC_Halt(pdev, num);
	}
	/*
	else if (hcreg->HCINT & USB_OTG_HCINT_ACK)
	{
		hcreg->HCINT = USB_OTG_HCINT_ACK;

		 if(pdev->host.hc[num].do_ping == 1)
	    {
	      pdev->host.HC_Status[num] = HC_NYET;  
			__HAL_HCD_UNMASK_HALT_HC_INT();		  
	      USB_OTG_HC_Halt(pdev, num);
	      pdev->host.URB_State[num]  = URB_NOTREADY;
	    }
	}*/
	else if (hcreg->HCINT & USB_OTG_HCINT_STALL)
	{
		p_err_fun;
		hcreg->HCINT = USB_OTG_HCINT_STALL;
		__HAL_HCD_UNMASK_HALT_HC_INT();
		USB_OTG_HC_Halt(pdev, num);
		pdev->host.HC_Status[num] = HC_STALL;
	}
	else if (hcreg->HCINT & USB_OTG_HCINT_NAK)
	{p_err_fun;
		hcreg->HCINT = USB_OTG_HCINT_NAK;
		pdev->host.ErrCnt[num] = 0;
		__HAL_HCD_UNMASK_HALT_HC_INT();
		USB_OTG_HC_Halt(pdev, num);
		pdev->host.HC_Status[num] = HC_NAK;
	}
	else if (hcreg->HCINT & USB_OTG_HCINT_TXERR)
	{p_err_fun;
		hcreg->HCINT = USB_OTG_HCINT_TXERR;
		__HAL_HCD_UNMASK_HALT_HC_INT();
		USB_OTG_HC_Halt(pdev, num);
		pdev->host.HC_Status[num] = HC_XACTERR;
	}
	else if (hcreg->HCINT & USB_OTG_HCINT_NYET)
	{p_err_fun;
		hcreg->HCINT = USB_OTG_HCINT_NYET;
		pdev->host.ErrCnt[num] = 0;
		__HAL_HCD_UNMASK_HALT_HC_INT();
		USB_OTG_HC_Halt(pdev, num);
		pdev->host.HC_Status[num] = HC_NYET;
	}
	else if (hcreg->HCINT & USB_OTG_HCINT_DTERR)
	{p_err_fun;
		hcreg->HCINT = USB_OTG_HCINT_NAK;
		hcreg->HCINT = USB_OTG_HCINT_DTERR;
		__HAL_HCD_UNMASK_HALT_HC_INT();
		USB_OTG_HC_Halt(pdev, num);
		pdev->host.HC_Status[num] = HC_DATATGLERR;
	}
	else if (hcreg->HCINT & USB_OTG_HCINT_CHH)
	{
		hcreg->HCINT = USB_OTG_HCINT_CHH;
		ret = 1;
		__HAL_HCD_MASK_HALT_HC_INT();
	
		if (pdev->host.HC_Status[num] == HC_XFRC)
		{
			pdev->host.URB_State[num] = URB_DONE;
			
			if (hcchar.b.eptype == EP_TYPE_BULK)
			{
				if (pdev->host.hc[num].packet_count &1) 
					pdev->host.hc[num].toggle_out ^= 1;
			}
			
		}
		else if (pdev->host.HC_Status[num] == HC_NAK)
		{
			pdev->host.URB_State[num] = URB_NOTREADY;
		}
		else if (pdev->host.HC_Status[num] == HC_NYET)
		{
			/*
			if (pdev->host.hc[num].do_ping == 1)
			{
				USB_OTG_HC_DoPing(pdev, num);
			}*/
			pdev->host.hc[num].do_ping = 0;
			pdev->host.URB_State[num] = URB_NOTREADY;
		}
		else if (pdev->host.HC_Status[num] == HC_STALL)
		{
			pdev->host.URB_State[num] = URB_STALL;
		}
		else if ((pdev->host.HC_Status[num] == HC_XACTERR) || (pdev->host.HC_Status[num] == HC_DATATGLERR))
		{
			if (pdev->host.ErrCnt[num] > 3)
			{
				pdev->host.URB_State[num] = URB_ERROR;
				pdev->host.ErrCnt[num] = 0;
			}else{
				pdev->host.URB_State[num] = URB_NOTREADY;
			}
			hcchar.b.chen = 1;
			hcchar.b.chdis = 0;
			USB_OTG_WRITE_REG32(&pdev->regs.HC_REGS[num]->HCCHAR, hcchar.d32);
		}
		else if (pdev->host.HC_Status[num] == HC_HALTED)
		{
			pdev->host.URB_State[num] = URB_ERROR;
			pdev->host.ErrCnt[num] = 0;
		}
		else
		{
			ret = 0;
			p_err("unkown out chhltd:%d\n", pdev->host.HC_Status[num]);
		}

	}

	return ret;
}

//#endif 


/**
 * @brief  USB_OTG_USBH_handle_hc_n_In_ISR
 *         Handles interrupt for a specific Host Channel
 * @param  pdev: Selected device
 * @param  hc_num: Channel number
 * @retval status
 */


uint32_t USB_OTG_USBH_handle_hc_n_In_ISR(USB_OTG_CORE_HANDLE *pdev, uint32_t num)
{
	USB_OTG_HCCHAR_TypeDef hcchar;
	USB_OTG_HCTSIZn_TypeDef hctsiz;
	USB_OTG_HC_REGS *hcreg;
	int ret = 0;
	
	hcreg = pdev->regs.HC_REGS[num];
	
	hcchar.d32 = USB_OTG_READ_REG32(&pdev->regs.HC_REGS[num]->HCCHAR);

	 if(hcreg->HCINT & USB_OTG_HCINT_XFRC)
	{
		hcreg->HCINT = USB_OTG_HCINT_XFRC;

		pdev->host.HC_Status[num] = HC_XFRC;
		pdev->host.ErrCnt[num] = 0;

		if (hcchar.b.eptype == EP_TYPE_BULK)
		{
			hcreg->HCINT = USB_OTG_HCINT_NAK;
			if (pdev->cfg.dma_enable == 1)
			{
				hctsiz.d32 = USB_OTG_READ_REG32(&pdev->regs.HC_REGS[num]->HCTSIZ);
				pdev->host.XferCnt[num] = pdev->host.hc[num].xfer_len - hctsiz.b.xfersize;

				pdev->host.hc[num].packet_count = (pdev->host.XferCnt[num] + pdev->host.hc[num].max_packet - 1) / pdev->host.hc[num].max_packet;
			}
			if (pdev->host.hc[num].packet_count &1) 
				pdev->host.hc[num].toggle_in ^= 1;
			__HAL_HCD_UNMASK_HALT_HC_INT();
			USB_OTG_HC_Halt(pdev, num);
			pdev->urb[num]->complete_flag = FALSE;
		}
		else if (hcchar.b.eptype == EP_TYPE_CTRL)
		{
			hcreg->HCINT = USB_OTG_HCINT_NAK;
			pdev->host.hc[num].toggle_in ^= 1;
			__HAL_HCD_UNMASK_HALT_HC_INT();
			USB_OTG_HC_Halt(pdev, num);
		}
		else if (hcchar.b.eptype == EP_TYPE_INTR)
		{
			hcchar.b.oddfrm = 1;
			USB_OTG_WRITE_REG32(&pdev->regs.HC_REGS[num]->HCCHAR, hcchar.d32);
			pdev->host.URB_State[num] = URB_DONE;
			ret = 1;
			pdev->host.hc[num].toggle_in ^= 1;
		}
		
	}
	else  if(hcreg->HCINT & USB_OTG_HCINT_NAK)
	{
		hcreg->HCINT = USB_OTG_HCINT_NAK;

		pdev->host.HC_Status[num] = HC_NAK;

		if (hcchar.b.eptype == EP_TYPE_CTRL) 
		{
			hcchar.b.chen = 1;
			hcchar.b.chdis = 0;
			USB_OTG_WRITE_REG32(&pdev->regs.HC_REGS[num]->HCCHAR, hcchar.d32);
		}else if(hcchar.b.eptype == EP_TYPE_BULK)
		{
			//在1ms定时器里面查询，否则会占用太多cpu
			//pdev->urb[num]->complete_flag = TRUE;
			hcchar.b.chen = 1;
			hcchar.b.chdis = 0;
			USB_OTG_WRITE_REG32(&pdev->regs.HC_REGS[num]->HCCHAR, hcchar.d32);
		}
		else if(hcchar.b.eptype == EP_TYPE_INTR)
		{
			__HAL_HCD_UNMASK_HALT_HC_INT();
			USB_OTG_HC_Halt(pdev, num);
		}
	}
	else if(hcreg->HCINT & USB_OTG_HCINT_AHBERR)
	{
		hcreg->HCINT = USB_OTG_HCINT_AHBERR;
	}
	/*
	else if(hcreg->HCINT & USB_OTG_HCINT_ACK)
	{
		hcreg->HCINT = USB_OTG_HCINT_ACK;
	}*/
	else if(hcreg->HCINT & USB_OTG_HCINT_STALL)
	{p_err_fun;
		hcreg->HCINT = USB_OTG_HCINT_NAK;
		hcreg->HCINT = USB_OTG_HCINT_STALL;
		pdev->host.HC_Status[num] = HC_STALL;
		 /* NOTE: When there is a 'stall', reset also nak,
		else, the pdev->host.HC_Status = HC_STALL
		will be overwritten by 'nak' in code below */
		__HAL_HCD_UNMASK_HALT_HC_INT();
		USB_OTG_HC_Halt(pdev, num);

	}
	else if(hcreg->HCINT & USB_OTG_HCINT_DTERR)
	{p_err_fun;
		hcreg->HCINT = USB_OTG_HCINT_NAK;
		hcreg->HCINT = USB_OTG_HCINT_DTERR;
		
		pdev->host.HC_Status[num] = HC_DATATGLERR;
		__HAL_HCD_UNMASK_HALT_HC_INT();
		USB_OTG_HC_Halt(pdev, num);
	}
	else if(hcreg->HCINT & USB_OTG_HCINT_FRMOR)
	{p_err_fun;
		hcreg->HCINT = USB_OTG_HCINT_FRMOR;
		__HAL_HCD_UNMASK_HALT_HC_INT();
		USB_OTG_HC_Halt(pdev, num);
	}
	else if(hcreg->HCINT & USB_OTG_HCINT_TXERR)
	{p_err_fun;
		hcreg->HCINT = USB_OTG_HCINT_TXERR;
		pdev->host.ErrCnt[num]++;
		pdev->host.HC_Status[num] = HC_XACTERR;
		__HAL_HCD_UNMASK_HALT_HC_INT();
		USB_OTG_HC_Halt(pdev, num);
	}

	else if(hcreg->HCINT & USB_OTG_HCINT_CHH)
	{
		hcreg->HCINT = USB_OTG_HCINT_CHH;
		__HAL_HCD_MASK_HALT_HC_INT();
		ret = 1;
	
		if (pdev->host.HC_Status[num] == HC_XFRC)
		{
			pdev->host.URB_State[num] = URB_DONE;
		}
		else if (pdev->host.HC_Status[num] == HC_NAK)
		//mark nak作为正常的接收
		{
			pdev->host.URB_State[num] = URB_DONE;
			//p_err_fun;
			
		}
		else if (pdev->host.HC_Status[num] == HC_STALL)
		{
			pdev->host.URB_State[num] = URB_STALL;
		}

		else if ((pdev->host.HC_Status[num] == HC_XACTERR) || (pdev->host.HC_Status[num] == HC_DATATGLERR))
		{
			 if(pdev->host.ErrCnt[num]++ > 3)
		      {      
					pdev->host.ErrCnt[num] = 0;
					pdev->host.URB_State[num] = URB_ERROR;
			 }else{

					pdev->host.URB_State[num] = URB_NOTREADY;
			 }
			hcchar.b.chen = 1;
			hcchar.b.chdis = 0;
			USB_OTG_WRITE_REG32(&pdev->regs.HC_REGS[num]->HCCHAR, hcchar.d32);

		}
		else if (pdev->host.HC_Status[num] == HC_HALTED)
		{
			pdev->host.URB_State[num] = URB_ERROR;
			pdev->host.ErrCnt[num] = 0;
			
		}
		else
		{
			ret = 0;
			p_err("unkown hltd:%d\n", pdev->host.HC_Status[num]);
		}
	}

	return ret;

}


/**
 * @brief  USB_OTG_USBH_handle_rx_qlvl_ISR
 *         Handles the Rx Status Queue Level Interrupt
 * @param  pdev: Selected device
 * @retval status
 */
static uint32_t USB_OTG_USBH_handle_rx_qlvl_ISR(USB_OTG_CORE_HANDLE *pdev)
{
	USB_OTG_GRXFSTS_TypeDef grxsts;
	USB_OTG_GINTMSK_TypeDef intmsk;
	USB_OTG_HCTSIZn_TypeDef hctsiz;
	USB_OTG_HCCHAR_TypeDef hcchar;
	__IO uint8_t channelnum = 0;

	/* Disable the Rx Status Queue Level interrupt */
	intmsk.d32 = 0;
	intmsk.b.rxstsqlvl = 1;
	USB_OTG_MODIFY_REG32(&pdev->regs.GREGS->GINTMSK, intmsk.d32, 0);

	grxsts.d32 = USB_OTG_READ_REG32(&pdev->regs.GREGS->GRXSTSP);
	channelnum = grxsts.b.chnum;
	hcchar.d32 = USB_OTG_READ_REG32(&pdev->regs.HC_REGS[channelnum]->HCCHAR);
	
	switch (grxsts.b.pktsts)
	{
		case GRXSTS_PKTSTS_IN:
			/* Read the data into the host buffer. */
			if ((grxsts.b.bcnt > 0) && (pdev->host.hc[channelnum].xfer_buff != (void*)0))
			{

				USB_OTG_ReadPacket(pdev, pdev->host.hc[channelnum].xfer_buff, channelnum, grxsts.b.bcnt);

				/*manage multiple Xfer */
				pdev->host.hc[channelnum].xfer_buff += grxsts.b.bcnt;
				pdev->host.hc[channelnum].xfer_count += grxsts.b.bcnt;
				pdev->host.hc[channelnum].packet_count += 1;

				pdev->host.XferCnt[channelnum] = pdev->host.hc[channelnum].xfer_count;

				hctsiz.d32 = USB_OTG_READ_REG32(&pdev->regs.HC_REGS[channelnum]->HCTSIZ);

				if (hctsiz.b.pktcnt > 0)
				{
					/* re-activate the channel when more packets are expected */
					hcchar.b.chen = 1;
					hcchar.b.chdis = 0;
					USB_OTG_WRITE_REG32(&pdev->regs.HC_REGS[channelnum]->HCCHAR, hcchar.d32);
					//pdev->host.hc[channelnum].toggle_in ^= 1;
				}
			}
			break;

		case GRXSTS_PKTSTS_IN_XFER_COMP:

		case GRXSTS_PKTSTS_DATA_TOGGLE_ERR:
		case GRXSTS_PKTSTS_CH_HALTED:
		default:
			break;
	}

	/* Enable the Rx Status Queue Level interrupt */
	intmsk.b.rxstsqlvl = 1;
	USB_OTG_MODIFY_REG32(&pdev->regs.GREGS->GINTMSK, 0, intmsk.d32);
	return 1;
}



/**
 * @brief  USB_OTG_USBH_handle_IncompletePeriodicXfer_ISR
 *         Handles the incomplete Periodic transfer Interrupt
 * @param  pdev: Selected device
 * @retval status
 */
static uint32_t USB_OTG_USBH_handle_IncompletePeriodicXfer_ISR(USB_OTG_CORE_HANDLE *pdev)
{

	USB_OTG_GINTSTS_TypeDef gintsts;
	USB_OTG_HCCHAR_TypeDef hcchar;




	hcchar.d32 = USB_OTG_READ_REG32(&pdev->regs.HC_REGS[0]->HCCHAR);
	hcchar.b.chen = 1;
	hcchar.b.chdis = 1;
	USB_OTG_WRITE_REG32(&pdev->regs.HC_REGS[0]->HCCHAR, hcchar.d32);

	gintsts.d32 = 0;
	/* Clear interrupt */
	gintsts.b.incomplisoout = 1;
	USB_OTG_WRITE_REG32(&pdev->regs.GREGS->GINTSTS, gintsts.d32);

	return 1;
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

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF
        FILE****/

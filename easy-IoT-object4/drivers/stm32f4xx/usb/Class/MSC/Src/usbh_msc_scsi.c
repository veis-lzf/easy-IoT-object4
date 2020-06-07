/**
 ******************************************************************************
 * @file    usbh_msc_scsi.c 
 * @author  MCD Application Team
 * @version V2.1.0
 * @date    19-March-2012
 * @brief   This file implements the SCSI commands
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

#include "usb_bsp.h"

#include "usbh_msc_core.h"
#include "usbh_msc_scsi.h"
#include "usbh_msc_bot.h"
#include "usbh_ioreq.h"
#include "usbh_def.h"

#define URB_RW_PKG_SIZE 	64
/** @addtogroup USBH_LIB
 * @{
 */

/** @addtogroup USBH_CLASS
 * @{
 */

/** @addtogroup USBH_MSC_CLASS
 * @{
 */

/** @defgroup USBH_MSC_SCSI 
 * @brief    This file includes the mass storage related functions
 * @{
 */


/** @defgroup USBH_MSC_SCSI_Private_TypesDefinitions
 * @{
 */

MassStorageParameter_TypeDef USBH_MSC_Param;
/**
 * @}
 */

/** @defgroup USBH_MSC_SCSI_Private_Defines
 * @{
 */
/**
 * @}
 */

/** @defgroup USBH_MSC_SCSI_Private_Macros
 * @{
 */
/**
 * @}
 */


/** @defgroup USBH_MSC_SCSI_Private_Variables
 * @{
 */
extern struct urb *bulk_out_urb;
extern struct urb *bulk_in_urb;
extern struct usb_device usb_dev;

/**
 * @}
 */


/** @defgroup USBH_MSC_SCSI_Private_FunctionPrototypes
 * @{
 */
/**
 * @}
 */


/** @defgroup USBH_MSC_SCSI_Exported_Variables
 * @{
 */

/**
 * @}
 */


/** @defgroup USBH_MSC_SCSI_Private_Functions
 * @{
 */
static void usb_bulk_complete(struct urb *urb)
{
    wake_up(urb->wait_complete);
} 

int do_submit_rw(USB_OTG_CORE_HANDLE *pdev, struct urb *p_urb, uint8_t *buff, int size)
{
    int ret;
again:
    usb_fill_bulk_urb(p_urb, &usb_dev, p_urb->pipe, buff, size, usb_bulk_complete, 0);

    clear_wait_event(bulk_out_urb->wait_complete);
    usb_submit_urb(p_urb, 0);
    ret = wait_event_timeout(p_urb->wait_complete, 5000);
    if (ret != 0)
    {
        p_err_fun;
	//USBH_ClrFeature(pdev, &USB_Host, pdev->host.hc[p_urb->pipe].ep_num, p_urb->pipe);
    }
	if(p_urb->status == URB_NOTREADY)
		goto again;
	
	return ret;
}
/*
 * @brief  MSC命令执行,包括发送CBW,读写数据,读取CSW
 * 注意,这里对原始代码改动比较大,使其能支持OS环境,
 * 尚未做出错处理,请参考官方文档,主要是对STALL错误等的处理
 * 
 * 
 */
int USBH_MSC_SUBMIT_RW(USB_OTG_CORE_HANDLE *pdev, HostCBWPkt_TypeDef *cbw, uint8_t *buff, int len, bool read)
{
    int ret, i, pkgs;
    struct urb *p_urb;

    if (!bulk_out_urb || !bulk_in_urb)
        return  - 1;

		ret = do_submit_rw(pdev, bulk_out_urb, cbw->CBWArray, USBH_MSC_BOT_CBW_PACKET_LENGTH);
    if (ret != 0)
    {
        p_err_fun;
				return -1;
    } 

    if (len)
    {
        pkgs = len / URB_RW_PKG_SIZE;
        if (read)
            p_urb = bulk_in_urb;
        else
            p_urb = bulk_out_urb;

        for (i = 0; i < pkgs; i++)
        {
					  ret = do_submit_rw(pdev, p_urb, buff, URB_RW_PKG_SIZE);
            if (ret != 0)
            {
                p_err_fun;
								return -1;
            }
						buff += URB_RW_PKG_SIZE;
            len -= URB_RW_PKG_SIZE;
        }

        if (len)
        {
						ret = do_submit_rw(pdev, p_urb, buff, len);
            if (ret != 0)
            {
                p_err_fun;
								return -1;
            }
        }
    }

		ret = do_submit_rw(pdev, bulk_in_urb, USBH_MSC_CSWData.CSWArray, USBH_MSC_CSW_MAX_LENGTH);
    if (ret != 0)
    {
        p_err_fun;
				return -1;
    }

    ret = USBH_MSC_DecodeCSW(pdev);
    if (ret)
        p_err_fun;

    return ret;
}


/**
 * @brief  USBH_MSC_TestUnitReady 
 *         Issues 'Test unit ready' command to the device. Once the response  
 *         received, it updates the status to upper layer.
 * @param  None
 * @retval Status
 */
uint8_t USBH_MSC_TestUnitReady(USB_OTG_CORE_HANDLE *pdev)
{
    uint8_t index;
    int ret;

    /*Prepare the CBW and relevent field*/
    USBH_MSC_CBWData.field.CBWTransferLength = 0; /* No Data Transfer */
    USBH_MSC_CBWData.field.CBWFlags = USB_EP_DIR_OUT;
    USBH_MSC_CBWData.field.CBWLength = CBW_LENGTH_TEST_UNIT_READY;

    for (index = CBW_CB_LENGTH; index != 0; index--)
    {
        USBH_MSC_CBWData.field.CBWCB[index] = 0x00;
    }

    USBH_MSC_CBWData.field.CBWCB[0] = OPCODE_TEST_UNIT_READY;

    ret = USBH_MSC_SUBMIT_RW(pdev, &USBH_MSC_CBWData, 0, 0, 1);

    return ret;
}


/**
 * @brief  USBH_MSC_ReadCapacity10  
 *         Issue the read capacity command to the device. Once the response 
 *         received, it updates the status to upper layer
 * @param  None
 * @retval Status
 */
uint8_t USBH_MSC_ReadCapacity10(USB_OTG_CORE_HANDLE *pdev)
{
    uint8_t index;
    int ret;
    uint8_t buff[XFER_LEN_READ_CAPACITY10];

    /*Prepare the CBW and relevent field*/
    USBH_MSC_CBWData.field.CBWTransferLength = XFER_LEN_READ_CAPACITY10;
    USBH_MSC_CBWData.field.CBWFlags = USB_EP_DIR_IN;
    USBH_MSC_CBWData.field.CBWLength = CBW_LENGTH;


    for (index = CBW_CB_LENGTH; index != 0; index--)
    {
        USBH_MSC_CBWData.field.CBWCB[index] = 0x00;
    }

    USBH_MSC_CBWData.field.CBWCB[0] = OPCODE_READ_CAPACITY10;

    ret = USBH_MSC_SUBMIT_RW(pdev, &USBH_MSC_CBWData, buff, XFER_LEN_READ_CAPACITY10, 1);

    (((uint8_t*) &USBH_MSC_Param.MSCapacity)[3]) = buff[0];
    (((uint8_t*) &USBH_MSC_Param.MSCapacity)[2]) = buff[1];
    (((uint8_t*) &USBH_MSC_Param.MSCapacity)[1]) = buff[2];
    (((uint8_t*) &USBH_MSC_Param.MSCapacity)[0]) = buff[3];

    /*assign the page length*/
    (((uint8_t*) &USBH_MSC_Param.MSPageLength)[1]) = buff[6];
    (((uint8_t*) &USBH_MSC_Param.MSPageLength)[0]) = buff[7];

    p_dbg("%d, %d", USBH_MSC_Param.MSCapacity, USBH_MSC_Param.MSPageLength);

    return ret;
}


/**
 * @brief  USBH_MSC_ModeSense6  
 *         Issue the Mode Sense6 Command to the device. This function is used 
 *          for reading the WriteProtect Status of the Mass-Storage device. 
 * @param  None
 * @retval Status
 */
uint8_t USBH_MSC_ModeSense6(USB_OTG_CORE_HANDLE *pdev)
{
    uint8_t index;
    int ret;
    uint8_t buff[XFER_LEN_MODE_SENSE6];

    USBH_MSC_CBWData.field.CBWTransferLength = XFER_LEN_MODE_SENSE6;
    USBH_MSC_CBWData.field.CBWFlags = USB_EP_DIR_IN;
    USBH_MSC_CBWData.field.CBWLength = CBW_LENGTH;

    for (index = CBW_CB_LENGTH; index != 0; index--)
    {
        USBH_MSC_CBWData.field.CBWCB[index] = 0x00;
    }

    USBH_MSC_CBWData.field.CBWCB[0] = OPCODE_MODE_SENSE6;
    USBH_MSC_CBWData.field.CBWCB[2] = MODE_SENSE_PAGE_CONTROL_FIELD |  \
        MODE_SENSE_PAGE_CODE;

    USBH_MSC_CBWData.field.CBWCB[4] = XFER_LEN_MODE_SENSE6;

    ret = USBH_MSC_SUBMIT_RW(pdev, &USBH_MSC_CBWData, buff, XFER_LEN_MODE_SENSE6, 1);

    if (buff[2] &MASK_MODE_SENSE_WRITE_PROTECT)
    {
        USBH_MSC_Param.MSWriteProtect = DISK_WRITE_PROTECTED;
    }
    else
    {
        USBH_MSC_Param.MSWriteProtect = 0;
    }

    return ret;
}

/**
 * @brief  USBH_MSC_RequestSense  
 *         Issues the Request Sense command to the device. Once the response 
 *         received, it updates the status to upper layer
 * @param  None
 * @retval Status
 */
uint8_t USBH_MSC_RequestSense(USB_OTG_CORE_HANDLE *pdev)
{
    uint8_t buff[ALLOCATION_LENGTH_REQUEST_SENSE];
    uint8_t index;
    int ret;

    USBH_MSC_CBWData.field.CBWTransferLength =  \
        ALLOCATION_LENGTH_REQUEST_SENSE;
    USBH_MSC_CBWData.field.CBWFlags = USB_EP_DIR_IN;
    USBH_MSC_CBWData.field.CBWLength = CBW_LENGTH;


    for (index = CBW_CB_LENGTH; index != 0; index--)
    {
        USBH_MSC_CBWData.field.CBWCB[index] = 0x00;
    }

    USBH_MSC_CBWData.field.CBWCB[0] = OPCODE_REQUEST_SENSE;
    USBH_MSC_CBWData.field.CBWCB[1] = DESC_REQUEST_SENSE;
    USBH_MSC_CBWData.field.CBWCB[4] = ALLOCATION_LENGTH_REQUEST_SENSE;

    ret = USBH_MSC_SUBMIT_RW(pdev, &USBH_MSC_CBWData, buff, ALLOCATION_LENGTH_REQUEST_SENSE, 1);

    (((uint8_t*) &USBH_MSC_Param.MSSenseKey)[3]) = buff[0];
    (((uint8_t*) &USBH_MSC_Param.MSSenseKey)[2]) = buff[1];
    (((uint8_t*) &USBH_MSC_Param.MSSenseKey)[1]) = buff[2];
    (((uint8_t*) &USBH_MSC_Param.MSSenseKey)[0]) = buff[3];

    return ret;
}


/**
 * @brief  USBH_MSC_Write10 
 *         Issue the write command to the device. Once the response received, 
 *         it updates the status to upper layer
 * @param  dataBuffer : DataBuffer contains the data to write
 * @param  address : Address to which the data will be written
 * @param  nbOfbytes : NbOfbytes to be written
 * @retval Status
 */
uint8_t USBH_MSC_Write10(USB_OTG_CORE_HANDLE *pdev, uint8_t *dataBuffer, uint32_t address, uint32_t nbOfbytes)
{
    uint8_t index;
    int ret;
    uint16_t nbOfPages;


    USBH_MSC_CBWData.field.CBWTransferLength = nbOfbytes;
    USBH_MSC_CBWData.field.CBWFlags = USB_EP_DIR_OUT;
    USBH_MSC_CBWData.field.CBWLength = CBW_LENGTH;


    for (index = CBW_CB_LENGTH; index != 0; index--)
    {
        USBH_MSC_CBWData.field.CBWCB[index] = 0x00;
    }

    USBH_MSC_CBWData.field.CBWCB[0] = OPCODE_WRITE10;

    /*logical block address*/
    USBH_MSC_CBWData.field.CBWCB[2] = (((uint8_t*) &address)[3]);
    USBH_MSC_CBWData.field.CBWCB[3] = (((uint8_t*) &address)[2]);
    USBH_MSC_CBWData.field.CBWCB[4] = (((uint8_t*) &address)[1]);
    USBH_MSC_CBWData.field.CBWCB[5] = (((uint8_t*) &address)[0]);

    /*USBH_MSC_PAGE_LENGTH = 512*/
    nbOfPages = nbOfbytes / USBH_MSC_PAGE_LENGTH;

    /*Tranfer length */
    USBH_MSC_CBWData.field.CBWCB[7] = (((uint8_t*) &nbOfPages)[1]);
    USBH_MSC_CBWData.field.CBWCB[8] = (((uint8_t*) &nbOfPages)[0]);

    ret = USBH_MSC_SUBMIT_RW(pdev, &USBH_MSC_CBWData, dataBuffer, nbOfbytes, 0);

    return ret;
}

/**
 * @brief  USBH_MSC_Read10 
 *         Issue the read command to the device. Once the response received, 
 *         it updates the status to upper layer
 * @param  dataBuffer : DataBuffer will contain the data to be read
 * @param  address : Address from which the data will be read
 * @param  nbOfbytes : NbOfbytes to be read
 * @retval Status
 */
uint8_t USBH_MSC_Read10(USB_OTG_CORE_HANDLE *pdev, uint8_t *dataBuffer, uint32_t address, uint32_t nbOfbytes)
{
    uint8_t index;
    int ret;
    uint16_t nbOfPages;

    USBH_MSC_CBWData.field.CBWTransferLength = nbOfbytes;
    USBH_MSC_CBWData.field.CBWFlags = USB_EP_DIR_IN;
    USBH_MSC_CBWData.field.CBWLength = CBW_LENGTH;

    for (index = CBW_CB_LENGTH; index != 0; index--)
    {
        USBH_MSC_CBWData.field.CBWCB[index] = 0x00;
    }

    USBH_MSC_CBWData.field.CBWCB[0] = OPCODE_READ10;

    /*logical block address*/

    USBH_MSC_CBWData.field.CBWCB[2] = (((uint8_t*) &address)[3]);
    USBH_MSC_CBWData.field.CBWCB[3] = (((uint8_t*) &address)[2]);
    USBH_MSC_CBWData.field.CBWCB[4] = (((uint8_t*) &address)[1]);
    USBH_MSC_CBWData.field.CBWCB[5] = (((uint8_t*) &address)[0]);

    /*USBH_MSC_PAGE_LENGTH = 512*/
    nbOfPages = nbOfbytes / USBH_MSC_PAGE_LENGTH;

    /*Tranfer length */
    USBH_MSC_CBWData.field.CBWCB[7] = (((uint8_t*) &nbOfPages)[1]);
    USBH_MSC_CBWData.field.CBWCB[8] = (((uint8_t*) &nbOfPages)[0]);

    ret = USBH_MSC_SUBMIT_RW(pdev, &USBH_MSC_CBWData, dataBuffer, nbOfbytes, 1);


    return ret;
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

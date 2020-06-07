/**
 ******************************************************************************
 * @file    usbh_msc_scsi.h
 * @author  MCD Application Team
 * @version V2.1.0
 * @date    19-March-2012
 * @brief   Header file for usbh_msc_scsi.c
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

/* Define to prevent recursive  ----------------------------------------------*/
#ifndef __USBH_MSC_FATFS_H__
#define __USBH_MSC_FATFS_H__

DSTATUS USB_disk_initialize(BYTE drv /* Physical drive number (0) */
);



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS USB_disk_status(BYTE drv /* Physical drive number (0) */
);


/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT USB_disk_read(BYTE drv,  /* Physical drive number (0) */
BYTE *buff,  /* Pointer to the data buffer to store read data */
DWORD sector,  /* Start sector number (LBA) */
BYTE count /* Sector count (1..255) */
);


/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
DRESULT USB_disk_write(BYTE drv,  /* Physical drive number (0) */
const BYTE *buff,  /* Pointer to the data to be written */
DWORD sector,  /* Start sector number (LBA) */
BYTE count /* Sector count (1..255) */
);
#endif /* _READONLY == 0 */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL != 0
DRESULT USB_disk_ioctl(BYTE drv,  /* Physical drive number (0) */
BYTE ctrl,  /* Control code */
void *buff /* Buffer to send/receive control data */
);
#endif 
/**
 * @}
 */

DWORD get_fattime(void);

#endif //__USBH_MSC_SCSI_H__


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

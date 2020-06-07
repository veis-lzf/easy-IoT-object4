/** @file moal_sdio_mmc.c
 *
 *  @brief This file contains SDIO MMC IF (interface) module
 *  related functions.
 * 
 * Copyright (C) 2008-2011, Marvell International Ltd. 
 *
 * This software file (the "File") is distributed by Marvell International 
 * Ltd. under the terms of the GNU General Public License Version 2, June 1991 
 * (the "License").  You may use, redistribute and/or modify this File in 
 * accordance with the terms and conditions of the License, a copy of which 
 * is available by writing to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA or on the
 * worldwide web at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 *
 * THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE 
 * ARE EXPRESSLY DISCLAIMED.  The License provides additional details about 
 * this warranty disclaimer.
 *
 */
/****************************************************
Change log:
	02/25/09: Initial creation -
		  This file supports SDIO MMC only
****************************************************/

//#include <linux/firmware.h>

#include "drivers.h"
#include "app.h"
#include "api.h"


#include "cfg80211.h"
#include "defs.h"
#include "type.h"
#include "types.h"

#include "moal_sdio.h"



/********************************************************
		Local Variables
********************************************************/

/********************************************************
		Global Variables
********************************************************/

#ifdef SDIO_SUSPEND_RESUME
/** PM keep power */
extern int pm_keep_power;
#endif


/** WLAN IDs */
/*
static const struct sdio_device_id wlan_ids[] = {
    {SDIO_DEVICE(MARVELL_VENDOR_ID, SD_DEVICE_ID_8797)},
    {SDIO_DEVICE(MARVELL_VENDOR_ID, SD_DEVICE_ID_8782)},
    {},
};
*/
//MODULE_DEVICE_TABLE(sdio, wlan_ids);

int woal_sdio_probe(struct sdio_func *func);
void woal_sdio_remove(struct sdio_func *func);

#ifdef SDIO_SUSPEND_RESUME
#ifdef MMC_PM_KEEP_POWER
int woal_sdio_suspend(struct device *dev);
int woal_sdio_resume(struct device *dev);

static struct dev_pm_ops wlan_sdio_pm_ops = {
    .suspend = woal_sdio_suspend,
    .resume = woal_sdio_resume,
};
#endif
#endif

#if 0
static struct sdio_driver wlan_sdio = {
    .name = "wlan_sdio",
    .id_table = wlan_ids,
    .probe = woal_sdio_probe,
    .remove = woal_sdio_remove,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
    .drv = {
            .owner = THIS_MODULE,
#ifdef SDIO_SUSPEND_RESUME
#ifdef MMC_PM_KEEP_POWER
            .pm = &wlan_sdio_pm_ops,
#endif
#endif
            }
#else
#ifdef SDIO_SUSPEND_RESUME
#ifdef MMC_PM_KEEP_POWER
    .drv = {
            .pm = &wlan_sdio_pm_ops,
            }
#endif
#endif
#endif
};
#endif


u32 roundup(u32 x,u32 y) 
{							
	return (((x) + (y - 1)) / y) * y;
}
u32 rounddown(u32 x,u32 y) 
{							
	return x-(x%y);
}

static int sdio_io_rw_ext_helper(int write,
	unsigned addr, int incr_addr, u8 *buf, unsigned size)
{
	unsigned remainder = size;
	unsigned max_blocks;
	int ret;

	if (cccr.multi_block/* && (size > 256)*/){
		/* Blocks per command is limited by host count, host transfer
		 * size (we only use a single sg entry) and the maximum for
		 * IO_RW_EXTENDED of 511 blocks. */
		//p_dbg("sdio_io_rw_ext_helper,addr:%d,sz;%d,cur_blksize:%d \n",addr,size, func.cur_blksize);
		//¡Á¡é¨°a
//		if(func.cur_blksize == 256)
		max_blocks = 512;

		while (remainder > func.cur_blksize) {
			unsigned blocks;

			blocks = remainder / func.cur_blksize;
			if (blocks > max_blocks)
				blocks = max_blocks;
			size = blocks * func.cur_blksize;

			ret = mmc_io_rw_extended(write,
				func.num, addr, incr_addr, buf,
				blocks, func.cur_blksize);
			if (ret)
				return ret;

			remainder -= size;
			buf += size;
			if (incr_addr)
				addr += size;
		}
	}
	/* Write the remainder using byte mode. */
	while (remainder > 0) {
		size = min(remainder,256);

		ret = mmc_io_rw_extended(write, func.num, addr,
			 incr_addr, buf, 1, size);
		if (ret)
			return ret;

		remainder -= size;
		buf += size;
		if (incr_addr)
			addr += size;
	}
	return 0;
}


int sdio_reset(void)
{
	int ret;
	u8 abort;

	/* SDIO Simplified Specification V2.0, 4.4 Reset for SDIO */
	ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_ABORT, 0, &abort);

	if (ret)
		abort = 0x08;
	else
		abort |= 0x08;

	ret = mmc_io_rw_direct(1, 0, SDIO_CCCR_ABORT, abort, NULL);

	return ret;
}


u8 sdio_readb(int fn,unsigned int addr, int *err_ret)
{
	int ret;
	u8 val = 0;

	if (err_ret)
		*err_ret = 0;
	
	ret = mmc_io_rw_direct(0, fn, addr, 0, &val);
	if (ret) {
		if (err_ret)
			*err_ret = ret;
//		return 0xFF;
	}

	return val;
}

void sdio_writeb(int fn,u8 b, unsigned int addr, int *err_ret)
{
	int ret;
	ret = mmc_io_rw_direct(1, fn, addr, b, NULL);
	if (err_ret)
	{
		*err_ret = ret;
	}
	if(ret){
		p_err("faild:%d",err_ret);
	}
}


u8 sdio_writeb_readb(int fn, u8 write_byte,
	unsigned int addr, int *err_ret)
{
	int ret;
	u8 val;

	ret = mmc_io_rw_direct(1, fn, addr,
			write_byte, &val);
	if (err_ret)
		*err_ret = ret;
	if (ret)
		val = 0xff;

	return val;
}


int sdio_readsb(void *dst, unsigned int addr,
	int count)
{
	return sdio_io_rw_ext_helper(0, addr, 0, dst, count);
}


int sdio_writesb(unsigned int addr, void *src,
	int count)
{
	return sdio_io_rw_ext_helper(1, addr, 0, src, count);
}

unsigned char sdio_f0_readb(unsigned int addr, int *err_ret)
{
	int ret;
	unsigned char val;

	if (err_ret)
		*err_ret = 0;

	ret = mmc_io_rw_direct(0, 0, addr, 0, &val);
	if (ret) {
		if (err_ret)
			*err_ret = ret;
		return 0xFF;
	}

	return val;
}

void sdio_f0_writeb(unsigned char b, unsigned int addr,
	int *err_ret)
{
	int ret;

	if ((addr < 0xF0 || addr > 0xFF) /*&& (!mmc_card_lenient_fn0(func->card))*/) {
		if (err_ret)
			*err_ret = -EINVAL;
		return;
	}

	ret = mmc_io_rw_direct(1, 0, addr, b, NULL);
	if (err_ret)
		*err_ret = ret;
}

int sdio_set_block_size(int fn,unsigned blksz)
{
	int ret = 0;
	ret = mmc_io_rw_direct(1, 0,
		SDIO_FBR_BASE(fn) + SDIO_FBR_BLKSIZE,
		blksz & 0xff, NULL);
	if (ret)
		goto end;
	ret = mmc_io_rw_direct(1, 0,
		SDIO_FBR_BASE(fn) + SDIO_FBR_BLKSIZE + 1,
		(blksz >> 8) & 0xff, NULL);

	func.cur_blksize = blksz;
	func.max_blksize = 512;
	if (ret)
		goto end;
end:
	return ret;
}


int sdio_enable_func(struct sdio_func *func)
{
	int ret;
	unsigned char reg;
	
	ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_IOEx, 0, &reg);
	if (ret  != SD_OK)
		goto err;

	 func->num = 1;

	reg |= 1 << func->num;

	ret = mmc_io_rw_direct(1, 0, SDIO_CCCR_IOEx, reg, NULL);
	if (ret  != SD_OK)
		goto err;

	while (1) {
		ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_IORx, 0, &reg);
		if (ret  != SD_OK)
			goto err;
		if (reg & (1 << func->num))
			break;
	}

	p_dbg("SDIO: Enabled func %d\n", reg);

	return 0;

err:
	p_err("SDIO: Failed to enable device\n");
	return ret;
}


int sdio_disable_func(struct sdio_func *func)
{
	int ret;
	unsigned char reg;


	ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_IOEx, 0, &reg);
	if (ret)
		goto err;

	reg &= ~(1 << func->num);

	ret = mmc_io_rw_direct(1, 0, SDIO_CCCR_IOEx, reg, NULL);
	if (ret)
		goto err;

	return 0;

err:
	p_err("SDIO: Failed to disable device\n");
	return -EIO;
}


int sdio_claim_irq(struct sdio_func *func, void *handler)
{
	int ret;
	unsigned char reg;

	ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_IENx, 0, &reg);
	if (ret)
	{
		p_err("sdio_claim_irq err\n");
		return ret;
	}
	p_dbg("sdio_claim_irq1 :%x\n", reg);	
	
	reg |= 1 << func->num;
	reg |= 0x01; /* Master interrupt enable */

	ret = mmc_io_rw_direct(1, 0, SDIO_CCCR_IENx, reg, NULL);
	if (ret)
		p_err("sdio_claim_irq err1\n");

	ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_IENx, 0, &reg);
	p_dbg("sdio_claim_irq2 :%x\n", reg);	

	//func->sdio_int_wait = init_event();
	//thread_create((void (*)(void *))handler, func, TASK_SDIO_THREAD_PRIO, 0, TASK_SDIO_STACK_SIZE, "sdio_thread");
	
	enable_sdio_int();
	
	return ret;
}

int sdio_release_irq(struct sdio_func *func)
{
	int ret;
	unsigned char reg;


	ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_IENx, 0, &reg);
	if (ret)
		return ret;

	reg &= ~(1 << func->num);

	/* Disable master interrupt with the last function interrupt */
	if (!(reg & 0xFE))
		reg = 0;

	ret = mmc_io_rw_direct(1, 0, SDIO_CCCR_IENx, reg, NULL);
	if (ret)
		return ret;

	return 0;
}

/********************************************************
		Local Functions
********************************************************/
/**  @brief This function dump the sdio register
 *  
 *  @param handle  A Pointer to the moal_handle structure
 *  @return 	   N/A
 */
void
woal_dump_sdio_reg(moal_handle * handle)
{
    int ret = 0;
	t_u8 data = 0;
	struct sdio_mmc_card *card = (struct sdio_mmc_card *) handle->card;
	
	data = data;
    data =
        sdio_f0_readb(0x05, &ret);
    PRINTM(MMSG, "fun0: reg 0x05=0x%x ret=%d\n", data, ret);
    data =
        sdio_f0_readb(0x04, &ret);
    PRINTM(MMSG, "fun0: reg 0x04=0x%x ret=%d\n", data, ret);
    data =
        sdio_readb(card->func->num, 0x03, &ret);
    PRINTM(MMSG, "fun1: reg 0x03=0x%x ret=%d\n", data, ret);
    data =
        sdio_readb(card->func->num, 0x04, &ret);
    PRINTM(MMSG, "fun1: reg 0x04=0x%x ret=%d\n", data, ret);
    data =
        sdio_readb(card->func->num, 0x05, &ret);
    PRINTM(MMSG, "fun1: reg 0x05=0x%x ret=%d\n", data, ret);
    data =
        sdio_readb(card->func->num, 0x60, &ret);
    PRINTM(MMSG, "fun1: reg 0x60=0x%x ret=%d\n", data, ret);
    data =
        sdio_readb(card->func->num, 0x61, &ret);
    PRINTM(MMSG, "fun1: reg 0x61=0x%x ret=%d\n", data, ret);
    return;
}

/********************************************************
		Global Functions
********************************************************/
/**  @brief This function updates the SDIO card types
 *  
 *  @param handle  A Pointer to the moal_handle structure
 *  @param card    A Pointer to card
 *
 *  @return 	   N/A
 */
t_void
woal_sdio_update_card_type(moal_handle * handle, t_void * card)
{
    struct sdio_mmc_card *cardp = (struct sdio_mmc_card *) card;

    /* Update card type */
    if (cardp->func->device == SD_DEVICE_ID_8797)
        handle->card_type = CARD_TYPE_SD8797;
    else if (cardp->func->device == SD_DEVICE_ID_8782)
        handle->card_type = CARD_TYPE_SD8782;
}

/** 
 *  @brief This function handles the interrupt.
 *  
 *  @param func	   A pointer to the sdio_func structure
 *  @return 	   N/A
 */
void
woal_sdio_interrupt(struct sdio_func *func)
{
    moal_handle *handle;
    struct sdio_mmc_card *card;

    ENTER();

	card = sdio_get_drvdata(func);
  if(!card)
		return;
	
	handle = card->handle;

	queue_work(handle->workqueue, &handle->main_work);
	LEAVE();
}

/**  @brief This function handles client driver probe.
 *  
 *  @param func	   A pointer to sdio_func structure.
 *  @param id	   A pointer to sdio_device_id structure.
 *  @return 	   MLAN_STATUS_SUCCESS or MLAN_STATUS_FAILURE/error code
 */
int
woal_sdio_probe(struct sdio_func *func)
{
    int ret = MLAN_STATUS_SUCCESS;
    struct sdio_mmc_card *card = NULL;

    ENTER();

    PRINTM(MINFO, "vendor=0x%4.04X device=0x%4.04X class=%d function=%d\n",
           func->vendor, func->device, func->class, func->num);

    card = kzalloc(sizeof(struct sdio_mmc_card), GFP_KERNEL);
    if (!card) {
        PRINTM(MFATAL, "Failed to allocate memory in probe function!\n");
        LEAVE();
        return -ENOMEM;
    }

    card->func = func;

#ifdef MMC_QUIRK_BLKSZ_FOR_BYTE_MODE
    /* The byte mode patch is available in kernel MMC driver which fixes one
       issue in MP-A transfer. bit1: use func->cur_blksize for byte mode */
    func->card->quirks |= MMC_QUIRK_BLKSZ_FOR_BYTE_MODE;
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
    /* wait for chip fully wake up */
    if (!func->enable_timeout)
        func->enable_timeout = 200;
#endif
    sdio_claim_host(func);
    ret = sdio_enable_func(func);
    if (ret) {
        sdio_disable_func(func);
        sdio_release_host(func);
        kfree(card);
        PRINTM(MFATAL, "sdio_enable_func() failed: ret=%d\n", ret);
        LEAVE();
        return -EIO;
    }
    sdio_release_host(func);
    if (NULL == woal_add_card(card)) {
        PRINTM(MERROR, "woal_add_card failed\n");
        kfree(card);
        sdio_claim_host(func);
        sdio_disable_func(func);
        sdio_release_host(func);
        ret = MLAN_STATUS_FAILURE;
    }

    LEAVE();
    return ret;
}

/**  @brief This function handles client driver remove.
 *  
 *  @param func	   A pointer to sdio_func structure.
 *  @return 	   N/A
 */
void
woal_sdio_remove(struct sdio_func *func)
{
    //struct sdio_mmc_card *card;

    ENTER();
/*
    PRINTM(MINFO, "SDIO func=%d\n", func->num);

    if (func) {
        card = sdio_get_drvdata(func);
        if (card) {
            woal_remove_card(card);
            kfree(card);
        }
    }
*/
    LEAVE();
}

#ifdef SDIO_SUSPEND_RESUME
#ifdef MMC_PM_KEEP_POWER
#ifdef MMC_PM_FUNC_SUSPENDED
/**  @brief This function tells lower driver that WLAN is suspended
 *  
 *  @param handle  A Pointer to the moal_handle structure
 *  @return 	   N/A
 */
void
woal_wlan_is_suspended(moal_handle * handle)
{
    ENTER();
    if (handle->suspend_notify_req == MTRUE) {
        handle->is_suspended = MTRUE;
        sdio_func_suspended(((struct sdio_mmc_card *) handle->card)->func);
    }
    LEAVE();
}
#endif

/**  @brief This function handles client driver suspend
 *  
 *  @param dev	   A pointer to device structure
 *  @return 	   MLAN_STATUS_SUCCESS or error code
 */
int
woal_sdio_suspend(struct device *dev)
{
    struct sdio_func *func = dev_to_sdio_func(dev);
    mmc_pm_flag_t pm_flags = 0;
    moal_handle *handle = NULL;
    struct sdio_mmc_card *cardp;
    int i;
    int ret = MLAN_STATUS_SUCCESS;
    int hs_actived = 0;
    mlan_ds_ps_info pm_info;

    ENTER();
    PRINTM(MCMND, "<--- Enter woal_sdio_suspend --->\n");
    if (func) {
        pm_flags = sdio_get_host_pm_caps(func);
        PRINTM(MCMND, "%s: suspend: PM flags = 0x%x\n", sdio_func_id(func),
               pm_flags);
        if (!(pm_flags & MMC_PM_KEEP_POWER)) {
            PRINTM(MERROR, "%s: cannot remain alive while host is suspended\n",
                   sdio_func_id(func));
            LEAVE();
            return -ENOSYS;
        }
        cardp = sdio_get_drvdata(func);
        if (!cardp || !cardp->handle) {
            PRINTM(MERROR, "Card or moal_handle structure is not valid\n");
            LEAVE();
            return MLAN_STATUS_SUCCESS;
        }
    } else {
        PRINTM(MERROR, "sdio_func is not specified\n");
        LEAVE();
        return MLAN_STATUS_SUCCESS;
    }

    handle = cardp->handle;
    if (handle->is_suspended == MTRUE) {
        PRINTM(MWARN, "Device already suspended\n");
        LEAVE();
        return MLAN_STATUS_SUCCESS;
    }

    handle->suspend_fail = MFALSE;
    memset(&pm_info, 0, sizeof(pm_info));
    if (MLAN_STATUS_SUCCESS ==
        woal_get_pm_info(woal_get_priv(handle, MLAN_BSS_ROLE_ANY), &pm_info)) {
        if (pm_info.is_suspend_allowed == MFALSE) {
            PRINTM(MMSG, "suspend not allowed!");
            ret = -EBUSY;
            goto done;
        }
    }
    for (i = 0; i < handle->priv_num; i++)
        netif_device_detach(handle->priv[i]->netdev);

    if (pm_keep_power) {
        /* Enable the Host Sleep */
#ifdef MMC_PM_FUNC_SUSPENDED
        handle->suspend_notify_req = MTRUE;
#endif
        hs_actived = woal_enable_hs(woal_get_priv(handle, MLAN_BSS_ROLE_ANY));
#ifdef MMC_PM_FUNC_SUSPENDED
        handle->suspend_notify_req = MFALSE;
#endif
        if (hs_actived) {
#ifdef MMC_PM_SKIP_RESUME_PROBE
            PRINTM(MCMND, "suspend with MMC_PM_KEEP_POWER and "
                   "MMC_PM_SKIP_RESUME_PROBE\n");
            ret = sdio_set_host_pm_flags(func, MMC_PM_KEEP_POWER |
                                         MMC_PM_SKIP_RESUME_PROBE);
#else
            PRINTM(MCMND, "suspend with MMC_PM_KEEP_POWER\n");
            ret = sdio_set_host_pm_flags(func, MMC_PM_KEEP_POWER);
#endif
        } else {
            PRINTM(MMSG, "HS not actived, suspend fail!");
            ret = -EBUSY;
            goto done;
        }
    }

    /* Indicate device suspended */
    handle->is_suspended = MTRUE;
  done:
    PRINTM(MCMND, "<--- Leave woal_sdio_suspend --->\n");
    LEAVE();
    return ret;
}

/**  @brief This function handles client driver resume
 *  
 *  @param dev	   A pointer to device structure
 *  @return 	   MLAN_STATUS_SUCCESS
 */
int
woal_sdio_resume(struct device *dev)
{
    struct sdio_func *func = dev_to_sdio_func(dev);
    mmc_pm_flag_t pm_flags = 0;
    moal_handle *handle = NULL;
    struct sdio_mmc_card *cardp;
    int i;

    ENTER();
    PRINTM(MCMND, "<--- Enter woal_sdio_resume --->\n");
    if (func) {
        pm_flags = sdio_get_host_pm_caps(func);
        PRINTM(MCMND, "%s: resume: PM flags = 0x%x\n", sdio_func_id(func),
               pm_flags);
        cardp = sdio_get_drvdata(func);
        if (!cardp || !cardp->handle) {
            PRINTM(MERROR, "Card or moal_handle structure is not valid\n");
            LEAVE();
            return MLAN_STATUS_SUCCESS;
        }
    } else {
        PRINTM(MERROR, "sdio_func is not specified\n");
        LEAVE();
        return MLAN_STATUS_SUCCESS;
    }
    handle = cardp->handle;

    if (handle->is_suspended == MFALSE) {
        PRINTM(MWARN, "Device already resumed\n");
        LEAVE();
        return MLAN_STATUS_SUCCESS;
    }

    handle->is_suspended = MFALSE;
    for (i = 0; i < handle->priv_num; i++)
        netif_device_attach(handle->priv[i]->netdev);

    /* Disable Host Sleep */
    woal_cancel_hs(woal_get_priv(handle, MLAN_BSS_ROLE_ANY), MOAL_NO_WAIT);
    PRINTM(MCMND, "<--- Leave woal_sdio_resume --->\n");
    LEAVE();
    return MLAN_STATUS_SUCCESS;
}
#endif
#endif /* SDIO_SUSPEND_RESUME */

/** 
 *  @brief This function writes data into card register
 *
 *  @param handle   A Pointer to the moal_handle structure
 *  @param reg      Register offset
 *  @param data     Value
 *
 *  @return    		MLAN_STATUS_SUCCESS or MLAN_STATUS_FAILURE
 */
mlan_status
woal_write_reg(moal_handle * handle, t_u32 reg, t_u32 data)
{
    mlan_status ret = MLAN_STATUS_FAILURE;
    sdio_writeb(((struct sdio_mmc_card *) handle->card)->func->num, (t_u8) data, reg,
                (int *) &ret);
    return ret;
}

/** 
 *  @brief This function reads data from card register
 *
 *  @param handle   A Pointer to the moal_handle structure
 *  @param reg      Register offset
 *  @param data     Value
 *
 *  @return    		MLAN_STATUS_SUCCESS or MLAN_STATUS_FAILURE
 */
mlan_status
woal_read_reg(moal_handle * handle, t_u32 reg, t_u32 * data)
{
    mlan_status ret = MLAN_STATUS_FAILURE;
    t_u8 val;

    val =
        sdio_readb(((struct sdio_mmc_card *) handle->card)->func->num, reg,
                   (int *) &ret);
    *data = val;

    return ret;
}

/**
 *  @brief This function writes multiple bytes into card memory
 *
 *  @param handle   A Pointer to the moal_handle structure
 *  @param pmbuf	Pointer to mlan_buffer structure
 *  @param port		Port
 *  @param timeout 	Time out value
 *
 *  @return    		MLAN_STATUS_SUCCESS or MLAN_STATUS_FAILURE
 */
mlan_status
woal_write_data_sync(moal_handle * handle, mlan_buffer * pmbuf, t_u32 port,
                     t_u32 timeout)
{
    mlan_status ret = MLAN_STATUS_FAILURE;
    t_u8 *buffer = (t_u8 *) (pmbuf->pbuf + pmbuf->data_offset);
    t_u8 blkmode = (port & MLAN_SDIO_BYTE_MODE_MASK) ? BYTE_MODE : BLOCK_MODE;
    t_u32 blksz = (blkmode == BLOCK_MODE) ? MLAN_SDIO_BLOCK_SIZE : 1;
    t_u32 blkcnt =
        (blkmode ==
         BLOCK_MODE) ? (pmbuf->data_len /
                        MLAN_SDIO_BLOCK_SIZE) : pmbuf->data_len;
    t_u32 ioport = (port & MLAN_SDIO_IO_PORT_MASK);
#ifdef SDIO_MMC_DEBUG
    handle->cmd53w = 1;
#endif
    if (!sdio_writesb(ioport, buffer,
         blkcnt * blksz))
        ret = MLAN_STATUS_SUCCESS;
#ifdef SDIO_MMC_DEBUG
    handle->cmd53w = 2;
#endif
    return ret;
}

/**
 *  @brief This function reads multiple bytes from card memory
 *
 *  @param handle   A Pointer to the moal_handle structure
 *  @param pmbuf	Pointer to mlan_buffer structure
 *  @param port		Port
 *  @param timeout 	Time out value
 *
 *  @return    		MLAN_STATUS_SUCCESS or MLAN_STATUS_FAILURE
 */
mlan_status
woal_read_data_sync(moal_handle * handle, mlan_buffer * pmbuf, t_u32 port,
                    t_u32 timeout)
{
    mlan_status ret = MLAN_STATUS_FAILURE;
    t_u8 *buffer = (t_u8 *) (pmbuf->pbuf + pmbuf->data_offset);
    t_u8 blkmode = (port & MLAN_SDIO_BYTE_MODE_MASK) ? BYTE_MODE : BLOCK_MODE;
    t_u32 blksz = (blkmode == BLOCK_MODE) ? MLAN_SDIO_BLOCK_SIZE : 1;
    t_u32 blkcnt =
        (blkmode ==
         BLOCK_MODE) ? (pmbuf->data_len /
                        MLAN_SDIO_BLOCK_SIZE) : pmbuf->data_len;
    t_u32 ioport = (port & MLAN_SDIO_IO_PORT_MASK);

#ifdef SDIO_MMC_DEBUG
    handle->cmd53r = 1;
#endif
    if (!sdio_readsb(buffer, ioport,blkcnt * blksz))
        ret = MLAN_STATUS_SUCCESS;
    else
        woal_dump_sdio_reg(handle);
#ifdef SDIO_MMC_DEBUG
    handle->cmd53r = 2;
#endif
    return ret;
}

/** 
 *  @brief This function registers the IF module in bus driver
 *  
 *  @return	   MLAN_STATUS_SUCCESS or MLAN_STATUS_FAILURE
 */
mlan_status
woal_bus_register(void)
{
    mlan_status ret = MLAN_STATUS_SUCCESS;

    ENTER();

    /* SDIO Driver Registration */
//    if (sdio_register_driver(&wlan_sdio)) {
//        PRINTM(MFATAL, "SDIO Driver Registration Failed \n");
//        LEAVE();
//        return MLAN_STATUS_FAILURE;
//    }

    LEAVE();
    return ret;
}

/** 
 *  @brief This function de-registers the IF module in bus driver
 *  
 *  @return 	   N/A
 */
void
woal_bus_unregister(void)
{
    ENTER();

    /* SDIO Driver Unregistration */
//    sdio_unregister_driver(&wlan_sdio);

    LEAVE();
}

/** 
 *  @brief This function de-registers the device
 *  
 *  @param handle A pointer to moal_handle structure
 *  @return 	  N/A
 */
void
woal_unregister_dev(moal_handle * handle)
{
    ENTER();
    if (handle->card) {
        /* Release the SDIO IRQ */
        sdio_claim_host(((struct sdio_mmc_card *) handle->card)->func);
        sdio_release_irq(((struct sdio_mmc_card *) handle->card)->func);
        sdio_disable_func(((struct sdio_mmc_card *) handle->card)->func);
        sdio_release_host(((struct sdio_mmc_card *) handle->card)->func);

        sdio_set_drvdata(((struct sdio_mmc_card *) handle->card)->func, NULL);

        PRINTM(MWARN, "Making the sdio dev card as NULL\n");
    }

    LEAVE();
}

/** 
 *  @brief This function registers the device
 *  
 *  @param handle  A pointer to moal_handle structure
 *  @return 	   MLAN_STATUS_SUCCESS or MLAN_STATUS_FAILURE
 */
mlan_status
woal_register_dev(moal_handle * handle)
{
    int ret = MLAN_STATUS_SUCCESS;
    struct sdio_mmc_card *card = handle->card;
    struct sdio_func *func;

    ENTER();

    func = card->func;
	
    sdio_claim_host(func);
    /* Request the SDIO IRQ */
    ret = sdio_claim_irq(func, /*woal_sdio_interrupt*/NULL);
    if (ret) {
        PRINTM(MFATAL, "sdio_claim_irq failed: ret=%d\n", ret);
        goto release_host;
    }

    /* Set block size */
    ret = sdio_set_block_size(card->func->num, MLAN_SDIO_BLOCK_SIZE);
    if (ret) {
        PRINTM(MERROR, "sdio_set_block_seize(): cannot set SDIO block size\n");
        ret = MLAN_STATUS_FAILURE;
        goto release_irq;
    }
	//sleep(100);
    sdio_release_host(func);
    sdio_set_drvdata(func, card);

    //handle->hotplug_device = &func->dev;

    LEAVE();
    return MLAN_STATUS_SUCCESS;

  release_irq:
    sdio_release_irq(func);
  release_host:
    sdio_release_host(func);
    handle->card = NULL;

    LEAVE();
    return MLAN_STATUS_FAILURE;
}

/** 
 *  @brief This function set bus clock on/off
 *  
 *  @param handle    A pointer to moal_handle structure
 *  @param option    TRUE--on , FALSE--off
 *  @return 	   MLAN_STATUS_SUCCESS
 */

int
woal_sdio_set_bus_clock(moal_handle * handle, t_u8 option)
{
#if 0
    struct sdio_mmc_card *cardp = (struct sdio_mmc_card *) handle->card;
    struct mmc_host *host = cardp->func->card->host;

    ENTER();
    if (option == MTRUE) {
        /* restore value if non-zero */
        if (cardp->host_clock)
            host->ios.clock = cardp->host_clock;
    } else {
        /* backup value if non-zero, then clear */
        if (host->ios.clock)
            cardp->host_clock = host->ios.clock;
        host->ios.clock = 0;
    }

    host->ops->set_ios(host, &host->ios);
    LEAVE();
#endif
	p_err_miss;
    return MLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function updates card reg based on the Cmd52 value in dev structure
 *  
 *  @param handle  	A pointer to moal_handle structure
 *  @param func    	A pointer to store func variable
 *  @param reg    	A pointer to store reg variable
 *  @param val    	A pointer to store val variable
 *  @return 	   	MLAN_STATUS_SUCCESS or MLAN_STATUS_FAILURE
 */
int
woal_sdio_read_write_cmd52(moal_handle * handle, int func, int reg, int val)
{
    int ret = MLAN_STATUS_SUCCESS;
    struct sdio_mmc_card *card = (struct sdio_mmc_card *) handle->card;

    ENTER();
    /* Save current func and reg for read */
    handle->cmd52_func = func;
    handle->cmd52_reg = reg;
    sdio_claim_host(card->func);
    if (val >= 0) {
        /* Perform actual write only if val is provided */
        if (func)
            sdio_writeb(card->func->num, val, reg, &ret);
        else
            sdio_f0_writeb(/*card->func, */val, reg, &ret);
        if (ret) {
            PRINTM(MERROR, "Cannot write value (0x%x) to func %d reg 0x%x\n",
                   val, func, reg);
        } else {
            PRINTM(MMSG, "write value (0x%x) to func %d reg 0x%x\n", (u8) val,
                   func, reg);
            handle->cmd52_val = val;
        }
    } else {
        if (func)
            val = sdio_readb(card->func->num, reg, &ret);
        else
            val = sdio_f0_readb(/*card->func,*/reg, &ret);
        if (ret) {
            PRINTM(MERROR, "Cannot read value from func %d reg 0x%x\n", func,
                   reg);
        } else {
            PRINTM(MMSG, "read value (0x%x) from func %d reg 0x%x\n", (u8) val,
                   func, reg);
            handle->cmd52_val = val;
        }
    }
    sdio_release_host(card->func);
    LEAVE();
    return ret;
}

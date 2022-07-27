/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ffconf.h"
#include "diskio.h"    /* FatFs lower layer API */
#include <stdbool.h>
#include "diskio_snor.h"
#include "hal_flash_disk_internal.h"

#ifdef MTK_FATFS_ON_SERIAL_NOR_FLASH
#include "flash_manager.h"
DSTATUS serial_nor_disk_state = STA_NOINIT;	/* Physical drive status */

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS serial_nor_disk_status(void)
{
	return serial_nor_disk_state;
}

DSTATUS serial_nor_disk_initialize(
    BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
    uint32_t retry = 0;
    int32_t result;

#if 0    
    if (pdrv != DRV_CFC) {
        return RES_PARERR;    /* Supports only drive 0 */
    }
#endif

    while (retry < 3) {
		result = FDM_MountDevice(pdrv,2,0);
		
        if (512 == result) {
            serial_nor_disk_state &= ~STA_NOINIT;  /* Clear STA_NOINIT flag */
            return serial_nor_disk_state;
        } else {
            retry++;
        }
    }

    return serial_nor_disk_state;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT serial_nor_disk_read(
    BYTE pdrv,		/* Physical drive nmuber to identify the drive */
    BYTE *buff,		/* Data buffer to store read data */
    DWORD sector,	/* Sector address in LBA */
    UINT count		/* Number of sectors to read */
)
{
    int32_t status;
	
    status = FDM_ReadSectors(sector, count, buff);

    return ((DRESULT)status);
}


/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT serial_nor_disk_write(
    BYTE pdrv,			/* Physical drive nmuber to identify the drive */
    const BYTE *buff,	/* Data to be written */
    DWORD sector,		/* Sector address in LBA */
    UINT count			/* Number of sectors to write */
)
{
    int32_t status;
	
    status = FDM_WriteSectors(sector, count, (void*)buff);

    return ((DRESULT)status);
}

DRESULT serial_nor_disk_ioctl(
    BYTE pdrv,		/* Physical drive nmuber (0..) */
    BYTE cmd,		/* Control code */
    void *buff		/* Buffer to send/receive control data */
)
{
    DRESULT result;

    if (serial_nor_disk_state & STA_NOINIT) {
        return RES_NOTRDY;    /* Check if drive is ready */
    }

    result = RES_ERROR;


    switch (cmd) {
        case CTRL_SYNC :  /* write process has been completed */
            result = RES_OK;
            break;

        case GET_SECTOR_COUNT :
        	   //get sectors number
            *(uint32_t *)buff = FDM_GetSectors();   /* unit is sector */
            result = RES_OK;
            break;

        case GET_BLOCK_SIZE : /* Get erase block size in unit of sector (DWORD) */
         //get block size
            *(uint32_t *)buff = FDM_GetBlockSize();    ///8;
            result = RES_OK;
            break;

        case CTRL_TRIM : /* Erase a block of sectors (used when _USE_TRIM == 1) */
#if 0
            temp_erase_pointer = buff;
            erase_start = temp_erase_pointer[0];
            erase_end = temp_erase_pointer[1];
            ///TODO::: 
            //status = hal_sd_erase_sectors(fatfs_sd_port, erase_start, (erase_end - erase_start));
            if (status == 0) {
                result = RES_OK;
            }
#endif
            //has no this api, do nothing
            result = RES_OK;
            break;

        default:
            result = RES_PARERR;
    }
    return result;
}

#endif /*MTK_FATFS_ON_SERIAL_NOR_FLASH*/



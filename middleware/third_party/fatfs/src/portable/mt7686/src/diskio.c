/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ffconf.h"
#include "diskio.h"     /* FatFs lower layer API */
#include <stdbool.h>
#include <string.h>
#include "hal.h"

#ifdef MTK_FATFS_ON_SERIAL_NAND
#include "diskio_snand.h"
#endif

#ifdef MTK_FATFS_ON_SERIAL_NOR_FLASH
#include "diskio_snor.h"
#endif

#if defined(MTK_FATFS_ON_SPI_SD) || defined(HAL_SD_MODULE_ENABLED)
#define FATFS_SD_ENABLED
#include "diskio_sd.h"
#endif


typedef enum {
    FATFS_DISK_SD    = 0, /*SD card*/
    FATFS_DISK_NAND  = 1, /*nand flash*/
    FATFS_DISK_NOR   = 2, /*nor flash*/
    FATFS_DISK_ERR   = 3, /*invalid parameter*/
} disk_type_t;


/*-----------------------------------------------------------------------*/
/* Get the disk type, the disk type define in ffconf.h                   */
/*-----------------------------------------------------------------------*/

disk_type_t disk_find_disk_type(BYTE pdrv)
{
    const char* const disk_str[] = {_VOLUME_STRS};  
    
    if (strcmp(disk_str[pdrv],"SD")==0) {
        return  FATFS_DISK_SD;
    }   


    if (strcmp(disk_str[pdrv],"NAND")==0) {
        return  FATFS_DISK_NAND;
    }

    if (strcmp(disk_str[pdrv],"NOR")==0) {
        return  FATFS_DISK_NOR;
    }

    return FATFS_DISK_ERR;
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
    BYTE pdrv       /* Physical drive nmuber to identify the drive */
)
{

    disk_type_t disk_type;

    disk_type = disk_find_disk_type(pdrv);

    if (disk_type == FATFS_DISK_ERR) {
        return STA_NOINIT;
    }

    switch(disk_type) {
        #ifdef FATFS_SD_ENABLED
        case FATFS_DISK_SD: {
            return  sd_disk_status();
        }
        #endif

        #ifdef MTK_FATFS_ON_SERIAL_NOR_FLASH
        case FATFS_DISK_NOR: {
            return serial_nor_disk_status();
        }
        #endif
    }

    return STA_NOINIT;
}


/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
    BYTE pdrv               /* Physical drive nmuber to identify the drive */
)
{
    disk_type_t disk_type;

    disk_type = disk_find_disk_type(pdrv);

    if (disk_type == FATFS_DISK_ERR) {
        return RES_PARERR;
    }

    switch(disk_type) {
        #ifdef FATFS_SD_ENABLED
        case FATFS_DISK_SD: {
            return  sd_disk_initialize(pdrv);
        }
        #endif

        #ifdef MTK_FATFS_ON_SERIAL_NOR_FLASH
        case FATFS_DISK_NOR: {
            return serial_nor_disk_initialize(pdrv);
        }
        #endif
    }

    return RES_PARERR;


}


/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
    BYTE pdrv,      /* Physical drive nmuber to identify the drive */
    BYTE *buff,     /* Data buffer to store read data */
    DWORD sector,   /* Sector address in LBA */
    UINT count      /* Number of sectors to read */
)
{
    disk_type_t disk_type;

    disk_type = disk_find_disk_type(pdrv);

    if (disk_type == FATFS_DISK_ERR) {
        return RES_PARERR;
    }

    switch(disk_type) {
        #ifdef FATFS_SD_ENABLED
        case FATFS_DISK_SD: {
            return  sd_disk_read(pdrv, buff, sector, count);
        }
        #endif

        #ifdef MTK_FATFS_ON_SERIAL_NOR_FLASH
        case FATFS_DISK_NOR: {
            return serial_nor_disk_read(pdrv, buff, sector, count);
        }
        #endif
    }

    return RES_PARERR;
}


/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write(
    BYTE pdrv,          /* Physical drive nmuber to identify the drive */
    const BYTE *buff,   /* Data to be written */
    DWORD sector,       /* Sector address in LBA */
    UINT count          /* Number of sectors to write */
)
{
    disk_type_t disk_type;

    disk_type = disk_find_disk_type(pdrv);

    if (disk_type == FATFS_DISK_ERR) {
        return RES_PARERR;
    }

    switch(disk_type) {
        #ifdef FATFS_SD_ENABLED
        case FATFS_DISK_SD: {
            return  sd_disk_write(pdrv, buff, sector, count);
        }
        #endif

        #ifdef MTK_FATFS_ON_SERIAL_NOR_FLASH
        case FATFS_DISK_NOR: {
            return serial_nor_disk_write(pdrv, buff, sector, count);
        }
        #endif
    }

    return RES_PARERR;
}
#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl(
    BYTE pdrv,      /* Physical drive nmuber (0..) */
    BYTE cmd,       /* Control code */
    void *buff      /* Buffer to send/receive control data */
)
{
    disk_type_t disk_type;

    disk_type = disk_find_disk_type(pdrv);

    if (disk_type == FATFS_DISK_ERR) {
        return RES_PARERR;
    }

    switch(disk_type) {
        #ifdef FATFS_SD_ENABLED
        case FATFS_DISK_SD: {
            return  sd_disk_ioctl(pdrv, cmd, buff);
        }
        #endif

        #ifdef MTK_FATFS_ON_SERIAL_NOR_FLASH
        case FATFS_DISK_NOR: {
            return  serial_nor_disk_ioctl(pdrv, cmd, buff);
        }
        #endif
    }

    return RES_PARERR;
    
}
#endif


#if !_FS_READONLY && !_FS_NORTC
DWORD get_fattime(void)
{
    // get the current time

    return 0; //return the current time instead of 0 if current time is got
}
#endif



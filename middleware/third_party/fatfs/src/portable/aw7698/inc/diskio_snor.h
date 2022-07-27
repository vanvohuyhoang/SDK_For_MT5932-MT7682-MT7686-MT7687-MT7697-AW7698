/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/


#ifndef _DISKIO_SNOR_DEFINED
#define _DISKIO_SNOR_DEFINED

#ifdef MTK_FATFS_ON_SERIAL_NOR_FLASH
/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
DSTATUS serial_nor_disk_status(void);

DSTATUS serial_nor_disk_initialize(
    BYTE pdrv				/* Physical drive nmuber to identify the drive */
);

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT serial_nor_disk_read(
    BYTE pdrv,		/* Physical drive nmuber to identify the drive */
    BYTE *buff,		/* Data buffer to store read data */
    DWORD sector,	/* Sector address in LBA */
    UINT count		/* Number of sectors to read */
);


/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT serial_nor_disk_write(
    BYTE pdrv,			/* Physical drive nmuber to identify the drive */
    const BYTE *buff,	/* Data to be written */
    DWORD sector,		/* Sector address in LBA */
    UINT count			/* Number of sectors to write */
);



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT serial_nor_disk_ioctl(
    BYTE pdrv,		/* Physical drive nmuber (0..) */
    BYTE cmd,		/* Control code */
    void *buff		/* Buffer to send/receive control data */
);
#endif /*MTK_FATFS_ON_SPI_SERIAL_NOR_FLASH*/

#endif


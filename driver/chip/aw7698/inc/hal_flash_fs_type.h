/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
 * Without the prior written permission of Airoha and/or its licensors,
 * any reproduction, modification, use or disclosure of Airoha Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
 * if you have agreed to and been bound by the applicable license agreement with
 * Airoha ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of Airoha Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
 */
 
#ifndef _FS_TYPE_H
#define _FS_TYPE_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

//-- Type definitions
typedef unsigned short  WORD;
typedef unsigned char   BYTE;

typedef void           *HANDLE;
typedef int             FS_HANDLE;
/*---------------------------------------------------------------------------
 * Error Codes
 *---------------------------------------------------------------------------*/
typedef enum {
    FS_NO_ERROR                    =    0,    /* No error */
    FS_ERROR_RESERVED              =   -1,    /* Reserved error, not used currently */
    FS_PARAM_ERROR                 =   -2,    /* User */
    FS_INVALID_FILENAME            =   -3,    /* User */
    FS_DRIVE_NOT_FOUND             =   -4,    /* User or Fatal */
    FS_TOO_MANY_FILES              =   -5,    /* User or Normal: use over max file handle number or more than 256 files in sort */
    FS_NO_MORE_FILES               =   -6,    /* Normal */
    FS_WRONG_MEDIA                 =   -7,    /* Fatal */
    FS_INVALID_FILE_SYSTEM         =   -8,    /* Fatal */
    FS_FILE_NOT_FOUND              =   -9,    /* User or Normal */
    FS_INVALID_FILE_HANDLE         =  -10,    /* User or Normal */
    FS_UNSUPPORTED_DEVICE          =  -11,    /* User */
    FS_UNSUPPORTED_DRIVER_FUNCTION =  -12,    /* User or Fatal */
    FS_CORRUPTED_PARTITION_TABLE   =  -13,    /* fatal */
    FS_TOO_MANY_DRIVES             =  -14,    /* not use so far */
    FS_INVALID_FILE_POS            =  -15,    /* User */
    FS_ACCESS_DENIED               =  -16,    /* User or Normal */
    FS_STRING_BUFFER_TOO_SMALL     =  -17,    /* User */
    FS_GENERAL_FAILURE             =  -18,    /* Normal */
    FS_PATH_NOT_FOUND              =  -19,    /* User */
    FS_FAT_ALLOC_ERROR             =  -20,    /* Fatal: disk crash */
    FS_ROOT_DIR_FULL               =  -21,    /* Normal */
    FS_DISK_FULL                   =  -22,    /* Normal */
    FS_TIMEOUT                     =  -23,    /* Normal: FS_CloseMSDC with nonblock */
    FS_BAD_SECTOR                  =  -24,    /* Normal: NAND flash bad block */
    FS_DATA_ERROR                  =  -25,    /* Normal: NAND flash bad block */
    FS_MEDIA_CHANGED               =  -26,    /* Normal */
    FS_SECTOR_NOT_FOUND            =  -27,    /* Fatal */
    FS_ADDRESS_MARK_NOT_FOUND      =  -28,    /* not use so far */
    FS_DRIVE_NOT_READY             =  -29,    /* Normal */
    FS_WRITE_PROTECTION            =  -30,    /* Normal: only for MSDC */
    FS_DMA_OVERRUN                 =  -31,    /* not use so far */
    FS_CRC_ERROR                   =  -32,    /* not use so far */
    FS_DEVICE_RESOURCE_ERROR       =  -33,    /* Fatal: Device crash */
    FS_INVALID_SECTOR_SIZE         =  -34,    /* Fatal */
    FS_OUT_OF_BUFFERS              =  -35,    /* Fatal */
    FS_FILE_EXISTS                 =  -36,    /* User or Normal */
    FS_LONG_FILE_POS               =  -37,    /* User : FS_Seek new pos over sizeof int */
    FS_FILE_TOO_LARGE              =  -38,    /* User: filesize + pos over sizeof int */
    FS_BAD_DIR_ENTRY               =  -39,    /* Fatal */
    FS_ATTR_CONFLICT               =  -40,    /* User: Can't specify FS_PROTECTION_MODE and FS_NONBLOCK_MOD */
    FS_CHECKDISK_RETRY             =  -41,    /* System: don't care */
    FS_LACK_OF_PROTECTION_SPACE    =  -42,    /* Fatal: Device crash */
    FS_SYSTEM_CRASH                =  -43,    /* Normal */
    FS_FAIL_GET_MEM                =  -44,    /* Normal */
    FS_READ_ONLY_ERROR             =  -45,    /* User or Normal */
    FS_DEVICE_BUSY                 =  -46,    /* Normal */
    FS_ABORTED_ERROR               =  -47,    /* Normal */
    FS_QUOTA_OVER_DISK_SPACE       =  -48,    /* Normal: Configuration Mistake */
    FS_PATH_OVER_LEN_ERROR         =  -49,    /* Normal */
    FS_APP_QUOTA_FULL              =  -50,    /* Normal */
    FS_VF_MAP_ERROR                =  -51,    /* User or Normal */
    FS_DEVICE_EXPORTED_ERROR       =  -52,    /* User or Normal */
    FS_DISK_FRAGMENT               =  -53,    /* Normal */
    FS_DIRCACHE_EXPIRED            =  -54,    /* Normal */
    FS_QUOTA_USAGE_WARNING         =  -55,    /* Normal or Fatal: System Drive Free Space Not Enought */
    FS_ERR_DIRDATA_LOCKED          =  -56,    /* Normal */
    FS_INVALID_OPERATION           =  -57,    /* Normal */
    FS_ERR_VF_PARENT_CLOSED        =  -58,    /* Virtual file's parent is closed */
    FS_ERR_UNSUPPORTED_SERVICE     =  -59,    /* The specified FS service is closed in this project. */

    FS_ERR_INVALID_JOB_ID          =  -81,
    FS_ERR_ASYNC_JOB_NOT_FOUND     =  -82,

    FS_MSDC_MOUNT_ERROR            = -100,    /* Normal */
    FS_MSDC_READ_SECTOR_ERROR      = -101,    /* Normal */
    FS_MSDC_WRITE_SECTOR_ERROR     = -102,    /* Normal */
    FS_MSDC_DISCARD_SECTOR_ERROR   = -103,    /* Normal */
    FS_MSDC_PRESNET_NOT_READY      = -104,    /* System */
    FS_MSDC_NOT_PRESENT            = -105,    /* Normal */

    FS_EXTERNAL_DEVICE_NOT_PRESENT = -106,    /* Normal */
    FS_HIGH_LEVEL_FORMAT_ERROR     = -107,    /* Normal */

    FS_CARD_BATCHCOUNT_NOT_PRESENT = -110,    /* Normal */

    FS_FLASH_MOUNT_ERROR           = -120,    /* Normal */
    FS_FLASH_ERASE_BUSY            = -121,    /* Normal: only for nonblock mode */
    FS_NAND_DEVICE_NOT_SUPPORTED   = -122,    /* Normal: Configuration Mistake */
    FS_FLASH_OTP_UNKNOWERR         = -123,    /* User or Normal */
    FS_FLASH_OTP_OVERSCOPE         = -124,    /* User or Normal */
    FS_FLASH_OTP_WRITEFAIL         = -125,    /* User or Normal */
    FS_FDM_VERSION_MISMATCH        = -126,    /* System */
    FS_FLASH_OTP_LOCK_ALREADY      = -127,    /* User or Normal */
    FS_FDM_FORMAT_ERROR            = -128,    /* The format of the disk content is not correct */

    FS_FDM_USER_DRIVE_BROKEN       = -129,    /*User drive unrecoverable broken*/
    FS_FDM_SYS_DRIVE_BROKEN        = -130,    /*System drive unrecoverable broken*/
    FS_FDM_MULTIPLE_BROKEN         = -131,    /*multiple unrecoverable broken*/

    FS_LOCK_MUTEX_FAIL             = -141,    /* System: don't care */
    FS_NO_NONBLOCKMODE             = -142,    /* User: try to call nonblock mode other than NOR flash */
    FS_NO_PROTECTIONMODE           = -143,    /* User: try to call protection mode other than NOR flash */

    /*
     * If disk size exceeds FS_MAX_DISK_SIZE (unit is KB, defined in makefile),
     * FS_TestMSDC(), FS_GetDevStatus(FS_MOUNT_STATE_ENUM) and all access behaviors will
     * get this error code.
     */
    FS_DISK_SIZE_TOO_LARGE         = (FS_MSDC_MOUNT_ERROR),     /*Normal*/

    FS_MINIMUM_ERROR_CODE          = -65536 /* 0xFFFF0000 */
} fs_error_enum;

#ifdef MTK_FATFS_ON_SERIAL_NOR_FLASH

#define FS_FMT_SINGLE_FAT       0x00000100L
#define FS_FMT_FORCE_LOW_LEVEL  0x00000200L
#define FS_FMT_NO_LOW_LEVEL     0x00000400L
#define FS_FMT_GET_DATA_SECTOR  0x00000800L
#define FS_FMT_FAT_12           0x00001000L
#define FS_FMT_FAT_16           0x00002000L
#define FS_FMT_FAT_32           0x00004000L
#define FS_FMT_NO_FAT_32        0x00008000L

#define FS_FLASH_NO_HIGH_FMT    0x20000000L
#define FS_FLASH_NO_LOW_FMT     0x40000000L


#define FS_IOCTRL_QUERY_NAND_INFO (0xF)


typedef enum {
    FS_DEVICE_UNKNOWN,
    FS_DEVICE_FLOPPY,
    FS_DEVICE_FDISK
} FS_DRIVER_DEVICE_ENUM;


typedef enum {
	   FS_FMT_PGS_DONE = -1,
	   FS_FMT_PGS_FAIL,
    FS_FMT_PGS_LOW_FMT,
    FS_FMT_PGS_HIGH_FMT,
}fs_format_pgs_enum;


/* Structure of partition record */
typedef void (* FS_FormatCallback)(const char *DeviceName, int Action, uint32_t Total, uint32_t Completed);

typedef struct {
    uint8_t   BootIndicator; /* 0x80 for bootable, 0 otherwise */
    uint8_t   StartHead;     /* 0 based */
    uint8_t   StartSector;   /* 1 based, bits 0-5 */
    uint8_t   StartTrack;    /* 0 based, bits 0-7, take bits 8,9 from StartSector */
    uint8_t   OSType;        /* FAT-12: 1, FAT-16: 4, 6, 14, FAT-32: 11, 12 */
    uint8_t   EndHead;       /* see StartHead */
    uint8_t   EndSector;     /* see StartSector */
    uint8_t   EndTrack;      /* see StartTrack */

    /*
     * offset to first sector of partition data
     * for primary partitions, this is the absolute
     * LBA of the boot sector
     */
    uint32_t  RelativeSector;
    uint32_t  Sectors;       /* number of sectors in partition */
} FS_PartitionRecord;

typedef struct {
    uint8_t BootCode[512 - 4 * sizeof(FS_PartitionRecord) - sizeof(uint16_t)];
    FS_PartitionRecord PTable[4];
    uint16_t Signature;
} FS_MasterBootRecord;


typedef struct {
    uint8_t     PhysicalDiskNumber;
    uint8_t     CurrentHead;
    uint8_t     Signature;
    uint32_t    SerialNumber;
    uint8_t     Label[11];
    char       SystemID[8];
} FS_ExtendedBIOSParameter;


typedef struct {
    char         OEMName[8];
    uint16_t     BytesPerSector;
    uint8_t      SectorsPerCluster;
    uint16_t     ReservedSectors;
    uint8_t      NumberOfFATs;
    uint16_t     DirEntries;
    uint16_t     SectorsOnDisk;
    uint8_t      MediaDescriptor;
    uint16_t     SectorsPerFAT;
    uint16_t     SectorsPerTrack;
    uint16_t     NumberOfHeads;
    uint32_t     NumberOfHiddenSectors;
    uint32_t     TotalSectors;
    union {
        struct {
            FS_ExtendedBIOSParameter BPB;
        } _16;
        struct {
            uint32_t     SectorsPerFAT;
            uint16_t     Flags;
            uint16_t     Version;
            uint32_t     RootDirCluster;
            uint16_t     FSInfoSector;
            uint16_t     BackupBootSector;
            uint8_t     Reserved[12];
            FS_ExtendedBIOSParameter BPB;
        } _32;
    } E;
} FS_BIOSParameter;

typedef struct {
    uint8_t NearJump[3];
    FS_BIOSParameter BP;
    uint8_t BootCode[512 - 3 - sizeof(FS_BIOSParameter) - sizeof(uint16_t)];
    uint16_t Signature;
} FS_BootRecord;


#define KAL_AND               2
#define KAL_CONSUME           1
#define KAL_AND_CONSUME       3
#define KAL_NO_SUSPEND        0
#define KAL_OR                0
#define KAL_OR_CONSUME        1
#define KAL_SUSPEND           0xFFFFFFFF


typedef struct __fsdriver {
    int  (* MountDevice)    (void *DriveData, int DeviceNumber, int DeviceType, uint32_t Flags);
    int  (* ShutDown)       (void *DriveData);
    int  (* ReadSectors)    (void *DriveData, uint32_t Sector, uint32_t Sectors, void *Buffer);
    int  (* WriteSectors)   (void *DriveData, uint32_t Sector, uint32_t Sectors, void *Buffer);
    int  (* MediaChanged)   (void *DriveData);
    int  (* DiscardSectors) (void *DriveData, uint32_t Sector, uint32_t Sectors);
    int  (* GetDiskGeometry)(void *DriveData, FS_PartitionRecord *DiskGeometry, uint8_t *MediaDescriptor);
    int  (* LowLevelFormat) (void *DriveData, const char *DeviceName, FS_FormatCallback Progress, uint32_t Flags);
    int  (* NonBlockWriteSectors)   (void *DriveData, uint32_t Sector, uint32_t Sectors, void *Buffer);
    int  (* RecoverableWriteSectors)(void *DriveData, uint32_t Sector, uint32_t Sectors, void *Buffer);
    int  (* ResumeSectorStates)     (void   *DriveData);
    int  (* HighLevelFormat)(void *DriveData, uint32_t BaseSector);
    int  (* RecoverDisk)    (void *DriveData);
    int  (* MessageAck)     (void *DriveData, int AckType);
    int  (* CopySectors)    (void *DriveData, uint32_t SrcSector, uint32_t DstSector, uint32_t Sectors);
    int  (* OTPAccess)      (void *DriveData, int type, uint32_t Offset, void *BufferPtr, uint32_t Length);
    int  (* OTPQueryLength) (void *DriveData, uint32_t *Length);
    int  (* IOCtrl)         (void *DriveData, uint32_t CtrlAction, void *CtrlData);   // For device IO control
} FS_Driver;

#endif //MTK_FATFS_ON_SERIAL_NOR_FLASH

#ifdef __cplusplus
}
#endif


#endif //_FS_TYPE_H


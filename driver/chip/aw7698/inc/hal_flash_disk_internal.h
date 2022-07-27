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
 
#ifndef __FLASH_DISK_INTERNAL_H__
#define __FLASH_DISK_INTERNAL_H__

/*******************************************
*   Include File
********************************************/

#include "hal_flash_disk.h"
#include "hal_flash.h"
#ifdef HAL_FLASH_MODULE_ENABLED
#include "hal_flash_custom_memorydevice.h"
#include "hal_flash_opt.h"
#include "hal_flash_mtd.h"
#include "aw7698.h"
#include <stdio.h>
#include <stdlib.h>
#include "hal_flash_drvflash.h"

#if defined(__SINGLE_BANK_NOR_FLASH_SUPPORT__)
//#include "intrCtrl.h"
//#include "drv_comm.h"
#endif /*__SINGLE_BANK_NOR_FLASH_SUPPORT__*/

#ifdef __INTEL_SIBLEY__
#include "app_buff_alloc.h" //get control buffer
#endif //__INTEL_SIBLEY__

#ifndef __SECURITY_OTP__
#define  __SECURITY_OTP__
#endif

/*******************************************
*   Structure and Macro Definition
********************************************/
#define RAW_DISK0 0
#define RAW_DISK1 1


#ifdef __SYSDRV_BACKUP_DISK_RAW__

#define BACKUP_HEADER_LEN (FILE_PREFIX_LEN + sizeof(uint32_t))
#define INIT_MARK  0x1234
#define LOCK_MARK  0x4321

#ifdef __INTEL_SIBLEY__
#define SIBLEY_REGION_SIZE 1024
#endif //__INTEL_SIBLEY__

#endif //__SYSDRV_BACKUP_DISK_RAW__


// Flash raw disk driver's data
typedef struct {
    uint32_t   DiskSize;
    NOR_MTD_Driver *MTDDriver;       ///< MTD driver
    void    *MTDData;                ///< MTD data
    NOR_MTD_FlashInfo FlashInfo;     ///< Flash information
#if defined (__INTEL_SIBLEY__) && defined(__SYSDRV_BACKUP_DISK_RAW__)
    uint32_t    bufIdx;
    BYTE    *Buffer;                 ///< SIBLEY Backup Partition Buffer(1024 bytes): used as write buffer and write data in control mode
#endif
    bool is_mount;
} NOR_FLASH_DISK_Data;

/*******************************************
*   Function and Variable Definition
********************************************/
#ifdef __NOR_SUPPORT_RAW_DISK__
extern NOR_FLASH_DISK_Data FlashDiskDriveData[NOR_BOOTING_NOR_DISK_NUM];
#endif

extern bool         INT_QueryExceptionStatus(void);
extern uint32_t       BlockIndex(void *DriverData, uint32_t blk_addr);
//extern kal_mutexid      fdm_reclaim_mutex;

hal_flash_status_t get_NOR_init_status(void);
/* Raw disk API */
int32_t readRawDiskData(NOR_FLASH_DISK_Data *D, uint32_t addr, uint8_t *data_ptr, uint32_t len);
#if defined(__NOR_SUPPORT_RAW_DISK__) || defined(__NOR_FULL_DRIVER__)
int32_t writeRawDiskData(NOR_FLASH_DISK_Data *D, uint32_t addr, uint8_t *data_ptr, uint32_t len);
int32_t eraseRawDiskBlock(NOR_FLASH_DISK_Data *D, uint32_t block_baseaddr);
#endif //__NOR_SUPPORT_RAW_DISK__ || __NOR_FULL_DRIVER__


#ifdef __SECURITY_OTP__
/**
 * @brief     flash write OTP
 * @param[in]  address is starting address to write from. The address must less than 0x200.
 * @param[in]  length is data length, the length add length need less than 0x200.
 * @param[in]  data is source data to be written.
 * @return
 * #HAL_FLASH_STATUS_OK on success
 */
hal_flash_status_t hal_flash_otp_write(uint32_t address, const uint8_t *data, uint32_t length);

/**
 * @brief     flash read OTP
 * @param[in]  start_address is starting address to read the data from. The address must less than 0x200.
 * @param[out]  buffer is place to hold the incoming data.
 * @param[in]  length is the length of the data content. the length add length need less than 0x200.
 * @return
 * #HAL_FLASH_STATUS_OK on success
 */
hal_flash_status_t hal_flash_otp_read(uint32_t start_address, uint8_t *buffer, uint32_t length);

/**
 * @brief     flash lock OTP, after call this API to lock down OTP area, you can't write data into the OTP area any more.
 * @return
 * #HAL_FLASH_STATUS_OK on success
 */
hal_flash_status_t hal_flash_otp_lock(void);
#endif

#endif
#endif //__FLASH_DISK_H__

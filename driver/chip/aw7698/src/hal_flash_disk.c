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
 
#include "hal_flash.h" //MT2523 common flash IOT API
#ifdef HAL_FLASH_MODULE_ENABLED

/********************************************************/
/*                  Include Header Files                */
/********************************************************/

#include "hal_flash_custom_memorydevice.h"
#include "string.h"
#include "hal_flash_disk_internal.h"
#include "hal_flash_mtd_internal.h"
#include "hal_flash_disk.h"
#include "hal_flash_mtd_sf_dal.h"
#include "hal_flash.h"
#include "memory_attribute.h"
#include "hal_cache.h"
#include "hal_nvic.h"

//#include "fs_errcode.h"
//#include "nvram_defs.h"


#ifdef __UBL__
volatile uint32_t processing_irqCnt;
#endif //__UBL__

/*---------------------------------------------------------------
 * flag to indicate if flash is mounted
 * set to true in MountDevice(), set to false in ShutDown()
 *--------------------------------------------------------------*/

extern NOR_FLASH_DISK_Data EntireDiskDriveData;

#define ENTIRE_DISK_DRIVER_DATA &EntireDiskDriveData

#if defined(__SYSDRV_BACKUP_DISK_RAW__) && !defined(__UBL__)
static uint32_t BackupPartitionStatus = BACKUP_DISK_STATUS_EMPTY;
static bool BP_is_mount = false;	//only one Backup Partition in system, use a global var instead of tie with data structure
static uint32_t currW, currR;
#endif //__SYSDRV_BACKUP_DISK_RAW__ && !__UBL__

#define RAW_DISK0_DRIVER_DATA &FlashDiskDriveData[RAW_DISK0]
#define TC01_DISK_DRIVER_DATA &FlashDiskDriveData[TC01_DISK]

#if 0
void CreateFDMLock(void)
{
}

hal_flash_status_t GetFDMLock(void)
{
    return HAL_FLASH_STATUS_OK;
}
#endif

void create_FDM_lock()
{
}

/*****************************************************************
Description : acquire FDM synchronization lock.
Input       :
Output      : None
******************************************************************/
int32_t retrieve_FDM_lock(void)
{
    return 0;
}

/*****************************************************************
Description : relieve FDM synchronization lock.
Input       :
Output      : None
******************************************************************/
void release_FDM_lock(void)
{
}


/*******************************************************************//**
 * Invoke before flash read/write/erase operation.
 * used when __NOR_SUPPORT_RAW_DISK__ or __UBL_NOR_FULL_DRIVER__ is defined.
 * accessed by backup partition and LG partition. (!__UBL__)
 * also access LG partition in bootloader (__UBL__ || __UBL_NOR_FULL_DRIVER__)
 **********************************************************************/
static void MountDevice(NOR_FLASH_DISK_Data *D)
{
    int32_t Result;

#ifndef __UBL__
    //if(D->is_mount && INT_QueryExceptionStatus() == false)
    if (D->is_mount) {
        return;
    }
    create_FDM_lock();

    retrieve_FDM_lock();

#else //__UBL__

    if (D->is_mount) {
        return;
    }

#endif //__UBL__

    Result = D->MTDDriver->MountDevice(D->MTDData, (void *)&D->FlashInfo);

    release_FDM_lock();

    if (Result < FS_NO_ERROR) {
        ASSERT(0);
    }

    D->is_mount = true;
}


/*******************************************************************//**
 * Read data from flash raw disk
 *
 * @par Catagory:
 * NOR RAW DISK
 *
 * @param[in] disk The number of disk to be read
 * @param[in] addr Start address relative to specified disk
 * @param[in] data_ptr Buffer for storing read data
 * @param[in] len The size of data_ptr
 *
 * @remarks
 * 1. File system non-block operation will be finished and then do raw disk read opertaion
 *
 * @return
 * RAW_DISK_ERR_WRONG_ADDRESS: Illegal read address
 * RAW_DISK_ERR_NONE: Read successful
 **********************************************************************/
ATTR_TEXT_IN_RAM int32_t readRawDiskData(NOR_FLASH_DISK_Data *D, uint32_t addr, uint8_t *data_ptr, uint32_t len)
{
    uint32_t phyAddr;
    uint32_t diskSize = D->DiskSize;
    int32_t result;

    if (addr >= diskSize || addr + len > diskSize) {
        return RAW_DISK_ERR_WRONG_ADDRESS;
    }
    phyAddr = (uint32_t)((NOR_Flash_MTD_Data *)D->MTDData)->BaseAddr + addr;

    if (!D->is_mount) {
        MountDevice(D);
    }

    retrieve_FDM_lock();

    do {
        result = SF_DAL_CheckDeviceReady(D->MTDData, 0);
    } while (FS_FLASH_ERASE_BUSY == result);
    
    memcpy(data_ptr, (void *)phyAddr, len);

    release_FDM_lock();

    return RAW_DISK_ERR_NONE;
}


#if !defined(__MINI_BOOTLOADER__) && (defined(__NOR_SUPPORT_RAW_DISK__)||defined(__NOR_FULL_DRIVER__))

/*******************************************************************//**
 * Write data into flash raw disk
 *
 * @par Catagory:
 * NOR RAW DISK
 *
 * @param[in] disk The number of disk to be read
 * @param[in] addr Start address relative to specified disk
 * @param[in] data_ptr Buffer for storing read data
 * @param[in] len The size of data_ptr
 *
 * @remarks
 * 1. File system non-block operation will be finished and then do raw disk read opertaion
 *
 * @return
 * RAW_DISK_ERR_WRONG_ADDRESS: Illegal program start address
 * RAW_DISK_ERR_NO_SPACE: No enough space to write len bytes data
 * RAW_DISK_ERR_PROG_FAIL: Program fail
 * RAW_DISK_ERR_NONE: Program successful
 **********************************************************************/

#if defined(__INTEL_SIBLEY__)

#ifdef __UBL__
//FDM is not exist, declare a 1KB buffer for Sibley
uint8_t SIBLEY_BUFFER[1024];
#else //!__UBL__
extern NOR_FLASH_DRV_Data FlashDriveData;
#define SIBLEY_BUFFER FlashDriveData.Buffer
#endif //__UBL__

#endif //__INTEL_SIBLEY__


int32_t writeRawDiskData(NOR_FLASH_DISK_Data *D, uint32_t addr, uint8_t *data_ptr, uint32_t len)
{
    uint32_t dest, src;
    uint32_t diskSize = D->DiskSize;
    int32_t result = FLASH_DISK_DONE;
    uint32_t DoneLength = 0;
    uint32_t mask;

    if (addr >= diskSize) {
        return RAW_DISK_ERR_WRONG_ADDRESS;
    }

    if (addr + len > diskSize) {
        return RAW_DISK_ERR_NO_SPACE;
    }

    if (!D->is_mount) {
        MountDevice(D);
    }

    retrieve_FDM_lock();

    while (DoneLength < len) {
        dest = (uint32_t)((NOR_Flash_MTD_Data *)D->MTDData)->BaseAddr + addr + DoneLength;
        src = (uint32_t)data_ptr + DoneLength;
        hal_nvic_save_and_set_interrupt_mask(&mask);  
        D->MTDDriver->MapWindow(D->MTDData, BlockIndex((void *)D->MTDData, addr + DoneLength), 0);
        hal_nvic_restore_interrupt_mask(mask);

        // if dest address not word align or write length is one, write one byte at a time
        if (((uint32_t)dest & (sizeof(FLASH_CELL) - 1)) || ((len - DoneLength) == 1)) {
            result = D->MTDDriver->ProgramData(D->MTDData, (void *)dest, (void *)src, 1);
            if (result != FLASH_DISK_DONE) {
                break;
            }
            DoneLength++;
        } // dest address is word align
        else {
            uint32_t blockSize = BlockSize(D->MTDData, BlockIndex(D->MTDData, addr + DoneLength));
            uint32_t programBytes = len - DoneLength;
            //calculate the block address boundary
            uint32_t nextBlkAddr = (dest + blockSize) & (~(blockSize - 1));

            //uint32_t next128ByteAddr = (dest+ 128) & (~(128 -1));

#ifdef __INTEL_SIBLEY__
            uint32_t nextRegionAddr = (dest + 1024) & (~(1024 - 1));

#endif // __INTEL_SIBLEY__  

            //if the data write across block boundary, shrink into a block
            if (dest + programBytes > (nextBlkAddr)) {
                programBytes = nextBlkAddr - dest;
            }

#ifndef __INTEL_SIBLEY__
            //round down to multiple of word
            programBytes = (programBytes) & (~(sizeof(FLASH_CELL) - 1));
#else // __INTEL_SIBLEY__
            if (dest + programBytes > nextRegionAddr) {
                programBytes = nextRegionAddr - dest;
            }
            if (programBytes & 0x1) { //the length is odd, use FDM buffer as temp buffer
                memcpy(SIBLEY_BUFFER, (void *)src, programBytes);
                SIBLEY_BUFFER[programBytes] = 0xFF;
            }
#endif // __INTEL_SIBLEY__  


            // program a word should be word align (MTD limitation)
            if (programBytes == sizeof(FLASH_CELL) && (src % sizeof(FLASH_CELL))) {
                FLASH_CELL Cell;
                uint8_t *b = (uint8_t *) &Cell;
                b[0] = *((uint8_t *)src);
                b[1] = *((uint8_t *)src + 1);
                result = D->MTDDriver->ProgramData(D->MTDData, (void *)dest, (void *)&Cell, programBytes);
                if (result != FLASH_DISK_DONE) {
                    break;
                }
            } else {
                if (programBytes & 0x1) { //must be Sibley flash
#ifdef __INTEL_SIBLEY__
                    result = D->MTDDriver->ProgramData(D->MTDData, (void *)dest, (void *)SIBLEY_BUFFER, programBytes + 1);
#else
                    ASSERT_RET(0, 0);
#endif
                } else {
                    result = D->MTDDriver->ProgramData(D->MTDData, (void *)dest, (void *)src, programBytes);
                }
                if (result != FLASH_DISK_DONE) {
                    break;
                }
            }
            DoneLength += programBytes;
        }
    }

    release_FDM_lock();

    if (result != FLASH_DISK_DONE) {
        return RAW_DISK_ERR_PROG_FAIL;
    }
    return RAW_DISK_ERR_NONE;

}

/*******************************************************************//**
 * Erase a block of flash raw disk
 *
 * @par Catagory:
 * NOR RAW DISK
 *
 * @param[in] disk The number of disk to be read
 * @param[in] blkIdx The block index to be erased
 *
 * @remarks
 * 1. File system non-block operation will be finished and then do raw disk read opertaion
 *
 * @return
 * RAW_DISK_ERR_WRONG_ADDRESS: Illegal block index to be erased
 * RAW_DISK_ERR_ERASE_FAIL: Erase fail
 * RAW_DISK_ERR_NONE: Program successful
 **********************************************************************/
int32_t eraseRawDiskBlock(NOR_FLASH_DISK_Data *D, uint32_t blkIdx)
{

    int32_t result;
    uint32_t block_index = (blkIdx & 0xFFFFFFF);
    if (!D->is_mount) {
        MountDevice(D);
    }

    if (block_index >= D->FlashInfo.TotalBlocks) { //TotalBlocks are initialized in MountDevice()
        return RAW_DISK_ERR_WRONG_ADDRESS;
        //ASSERT_RET(0, 0); //how to output msg?
    }

    retrieve_FDM_lock();

    //D->MTDDriver->MapWindow(D->MTDData, block_index, 0);

    result = D->MTDDriver->EraseBlock(D->MTDData, blkIdx);

    release_FDM_lock();

    if (result == FLASH_DISK_DONE) {
        return RAW_DISK_ERR_NONE;
    }
    return RAW_DISK_ERR_ERASE_FAIL;
}

#endif //!__MINI_BOOTLOADER__ && (__NOR_SUPPORT_RAW_DISK__ || __NOR_FULL_DRIVER__)



extern void Custom_NOR_Init(void);

ATTR_RWDATA_IN_TCM static hal_flash_status_t flash_init_status = HAL_FLASH_STATUS_ERROR_NO_INIT;
int32_t NOR_init(void)
{
    //only init flash one time
    if (flash_init_status == HAL_FLASH_STATUS_ERROR_NO_INIT) {
        Custom_NOR_Init();
        MountDevice(ENTIRE_DISK_DRIVER_DATA);
        flash_init_status = HAL_FLASH_STATUS_OK;
    }
    return ERROR_NOR_SUCCESS;
}

ATTR_TEXT_IN_RAM hal_flash_status_t get_NOR_init_status(void)
{
    return flash_init_status;
}

int32_t NOR_ReadPhysicalPage(uint32_t block_idx, uint32_t page_idx, uint8_t *data_ptr)
{
    NOR_FLASH_DISK_Data *D = ENTIRE_DISK_DRIVER_DATA;
    NOR_Flash_MTD_Data *mtdData = (NOR_Flash_MTD_Data *)D->MTDData;
    int32_t result;

    uint32_t addr = ((uint32_t)BlockAddress((void *)mtdData , block_idx) - (uint32_t)mtdData->BaseAddr)  + (page_idx << NOR_FLASH_PAGE_SHIFT);

    result = readRawDiskData(D, addr, data_ptr, NOR_FLASH_PAGE_SIZE);

    if (result != RAW_DISK_ERR_NONE) {
        return ERROR_NOR_READ;
    }
    return ERROR_NOR_SUCCESS;
}

#if defined(__NOR_FULL_DRIVER__) && !defined(__MINI_BOOTLOADER__)
int32_t NOR_ProgramPhysicalPage(uint32_t block_idx, uint32_t page_idx, uint8_t *data_ptr)
{
    NOR_FLASH_DISK_Data *D = ENTIRE_DISK_DRIVER_DATA;
    NOR_Flash_MTD_Data *mtdData = (NOR_Flash_MTD_Data *)D->MTDData;
    int32_t result;

    uint32_t addr = ((uint32_t)BlockAddress((void *)mtdData , block_idx) - (uint32_t)mtdData->BaseAddr)  + (page_idx << NOR_FLASH_PAGE_SHIFT);

    result = writeRawDiskData(D, addr, data_ptr, NOR_FLASH_PAGE_SIZE);
    if (result != RAW_DISK_ERR_NONE) {
        return ERROR_NOR_PROGRAM;
    }
    return ERROR_NOR_SUCCESS;
}

int32_t NOR_ErasePhysicalBlock(uint32_t block_idx)
{
    int32_t result;
    result = eraseRawDiskBlock(ENTIRE_DISK_DRIVER_DATA, block_idx);
    if (result != RAW_DISK_ERR_NONE) {
        return ERROR_NOR_ERASE;
    }
    return ERROR_NOR_SUCCESS;
}
#endif //__UBL_NOR_FULL_DRIVER__

uint32_t NOR_BlockSize(uint32_t block_idx)
{
    NOR_FLASH_DISK_Data *D = ENTIRE_DISK_DRIVER_DATA;
    NOR_Flash_MTD_Data *mtdData = (NOR_Flash_MTD_Data *)D->MTDData;
    return BlockSize(mtdData, block_idx);
}

uint32_t NOR_BlockIndex(uint32_t block_addr)
{
    NOR_FLASH_DISK_Data *D = ENTIRE_DISK_DRIVER_DATA;
    NOR_Flash_MTD_Data *mtdData = (NOR_Flash_MTD_Data *)D->MTDData;

    return BlockIndex(mtdData, block_addr);
}

// Return value:
//    ERROR_NOR_OVERRANGE: address out of NOR flash size
//    ERROR_NOR_SUCCESS: block_addr to *block_index, *offset translation successful
int32_t NOR_Addr2BlockIndexOffset(uint32_t block_addr, uint32_t *block_index, uint32_t *offset)
{
    NOR_FLASH_DISK_Data *D = ENTIRE_DISK_DRIVER_DATA;
    NOR_Flash_MTD_Data *mtdData = (NOR_Flash_MTD_Data *)D->MTDData;

#ifdef __UBL__
    block_addr = block_addr & (~(SFC_GENERIC_FLASH_BANK_MASK));
#endif

    *block_index = BlockIndex(mtdData, block_addr);
    if (*block_index == INVALID_BLOCK_INDEX) {
        return ERROR_NOR_OVERRANGE;
    }

    *offset = block_addr % BlockSize(mtdData, *block_index);

    return ERROR_NOR_SUCCESS;
}

// Translate block_index and offset to block address (*addr)
// Return value:
//    ERROR_NOR_SUCCESS
int32_t NOR_BlockIndexOffset2Addr(uint32_t block_index, uint32_t offset, uint32_t *addr)
{
    NOR_FLASH_DISK_Data *D = ENTIRE_DISK_DRIVER_DATA;
    NOR_Flash_MTD_Data *mtdData = (NOR_Flash_MTD_Data *)D->MTDData;

    *addr = (uint32_t)BlockAddress(mtdData, block_index) - (uint32_t)mtdData->BaseAddr + offset;

    return ERROR_NOR_SUCCESS;
}

//-----------------------------------------------------------------------------------------------
// MT2523 Raw Disk API for IOT
//-----------------------------------------------------------------------------------------------
hal_flash_status_t get_rawdisk_error_code(int32_t ori_err_code)
{
    if (ori_err_code == RAW_DISK_ERR_WRONG_ADDRESS) {
        return HAL_FLASH_STATUS_ERROR_WRONG_ADDRESS;
    } else if (ori_err_code == RAW_DISK_ERR_NO_SPACE) {
        return HAL_FLASH_STATUS_ERROR_NO_SPACE;
    } else if (ori_err_code == RAW_DISK_ERR_PROG_FAIL) {
        return HAL_FLASH_STATUS_ERROR_PROG_FAIL;
    } else if (ori_err_code == RAW_DISK_ERR_ERASE_FAIL) {
        return HAL_FLASH_STATUS_ERROR_ERASE_FAIL;
    } else if (ori_err_code == HAL_FLASH_STATUS_ERROR_LOCKED) { //mutex lock
        return HAL_FLASH_STATUS_ERROR_ERASE_FAIL;
    } else {
        ASSERT_RET(0, HAL_FLASH_STATUS_ERROR);
    }
}
hal_flash_block_t get_block_size_from_address(uint32_t address)
{
    uint32_t block_size;
    uint32_t block_index;

    block_index = NOR_BlockIndex(address);
    if (block_index == INVALID_BLOCK_INDEX) {
        ASSERT_RET(0, (hal_flash_block_t)(-1));
    }
    block_size = NOR_BlockSize(block_index);
    if (block_size == 0x1000) {
        return HAL_FLASH_BLOCK_4K;
    } else if (block_size == 0x8000) {
        return HAL_FLASH_BLOCK_32K;
    } else if (block_size == 0x10000) {
        return HAL_FLASH_BLOCK_64K;
    } else {
        ASSERT_RET(0, (hal_flash_block_t)(-1));
    }
}

//BL and FreeRTOS always do flash init
hal_flash_status_t hal_flash_init(void)
{
    NOR_init();
    return HAL_FLASH_STATUS_OK;
}
//Do nothing for flash deinit
hal_flash_status_t hal_flash_deinit(void)
{
    return HAL_FLASH_STATUS_OK;
}

//Notice: please call get_block_size_from_address() first to get block size
hal_flash_status_t hal_flash_erase(uint32_t start_address, hal_flash_block_t block)
{
    int32_t result;
    uint32_t block_index;
    uint32_t erase_type = 0;
    int32_t address;
    int32_t length;

    if (HAL_FLASH_BLOCK_4K == block) {
        length = 0x1000;
        if ((start_address & 0xFFF) != 0) {
            return HAL_FLASH_STATUS_ERROR_WRONG_ADDRESS;
        }
        erase_type = 0x1;
    } else if (HAL_FLASH_BLOCK_32K == block) {
        length = 0x8000;
        if ((start_address & 0x7FFF) != 0) {
            return HAL_FLASH_STATUS_ERROR_WRONG_ADDRESS;
        }
        erase_type = 0x2;
    } else if (HAL_FLASH_BLOCK_64K == block) {
        length = 0x10000;
        if ((start_address & 0xFFFF) != 0) {
            return HAL_FLASH_STATUS_ERROR_WRONG_ADDRESS;
        }
        erase_type = 0x4;
    } else {
        return HAL_FLASH_STATUS_ERROR_WRONG_ADDRESS;
    }

    block_index = NOR_BlockIndex(start_address);
    if (block_index == INVALID_BLOCK_INDEX) {
        return HAL_FLASH_STATUS_ERROR_WRONG_ADDRESS;
    }

    block_index |= (erase_type << 28);
    result = eraseRawDiskBlock(ENTIRE_DISK_DRIVER_DATA, block_index);
    if (result != RAW_DISK_ERR_NONE) {
        return get_rawdisk_error_code(result);
    }

#ifdef HAL_CACHE_MODULE_ENABLED
    address = start_address + HAL_FLASH_BASE_ADDRESS;

    /* Address should be alignment with cashe line size*/
    if (hal_cache_is_cacheable(address)) {
        uint32_t addr;
        for (addr = (address & CACHE_LINE_ALIGNMENT_MASK); addr <= ((address + length + CACHE_LINE_SIZE) & CACHE_LINE_ALIGNMENT_MASK); addr += CACHE_LINE_SIZE) {
            hal_cache_invalidate_one_cache_line(addr);
        }
    }
#endif
    return HAL_FLASH_STATUS_OK;
}

hal_flash_status_t hal_flash_read(uint32_t start_address, uint8_t *buffer, uint32_t length)
{
    NOR_FLASH_DISK_Data *D = ENTIRE_DISK_DRIVER_DATA;
    int32_t result;

    result = readRawDiskData(D, start_address, buffer, length);

    if (result != RAW_DISK_ERR_NONE) {
        return get_rawdisk_error_code(result);
    }
    return HAL_FLASH_STATUS_OK;
}

hal_flash_status_t hal_flash_write(uint32_t address, const uint8_t *data, uint32_t length)
{
    NOR_FLASH_DISK_Data *D = ENTIRE_DISK_DRIVER_DATA;
    int32_t result;

    result = writeRawDiskData(D, address, (uint8_t *)data, length);
    if (result != RAW_DISK_ERR_NONE) {
        return get_rawdisk_error_code(result);
    }

    #ifdef HAL_CACHE_MODULE_ENABLED
    address += HAL_FLASH_BASE_ADDRESS;
    /* Address should be alignment with cashe line size*/
    if(hal_cache_is_cacheable(address)){
       uint32_t addr;
       for(addr=(address&CACHE_LINE_ALIGNMENT_MASK); addr<=((address+length+CACHE_LINE_SIZE) & CACHE_LINE_ALIGNMENT_MASK); addr+=CACHE_LINE_SIZE) {
            hal_cache_invalidate_one_cache_line(addr);
       }
    }
    #endif

    return HAL_FLASH_STATUS_OK;
}


#ifdef __SECURITY_OTP__
int32_t OTP_Write_Data(NOR_FLASH_DISK_Data *D, uint32_t otp_addr, uint8_t *data_ptr, uint32_t len)
{
    int32_t result = FLASH_DISK_DONE;

    if (otp_addr >= ((SF_MTD_Data *)D->MTDData)->OTPLength) {
        return RAW_DISK_ERR_WRONG_ADDRESS;
    }

    if ((otp_addr + len) > (((SF_MTD_Data *)D->MTDData)->OTPLength + ((SF_MTD_Data *)D->MTDData)->OTPBase)) {
        return RAW_DISK_ERR_NO_SPACE;
    }

    if (!D->is_mount) {
        MountDevice(D);
    }

    if( retrieve_FDM_lock() != FS_NO_ERROR) {
        return RAW_DISK_ERR_FDM_LOCKED;
    }

    if (D->MTDDriver->OTPAccess) {
	    result = D->MTDDriver->OTPAccess(D->MTDData, FS_OTP_WRITE, (uint32_t)otp_addr, (void *)data_ptr, len);
    }

    release_FDM_lock();

    if (result != FLASH_DISK_DONE) {
        return RAW_DISK_ERR_PROG_FAIL;
    }
    return RAW_DISK_ERR_NONE;

}

int32_t OTP_Read_Data(NOR_FLASH_DISK_Data *D, uint32_t otp_addr, uint8_t *data_ptr, uint32_t len)
{
    int32_t result = FLASH_DISK_DONE;

    if (otp_addr >= ((SF_MTD_Data *)D->MTDData)->OTPLength) {
        return RAW_DISK_ERR_WRONG_ADDRESS;
    }

    if ((otp_addr + len) > (((SF_MTD_Data *)D->MTDData)->OTPLength + ((SF_MTD_Data *)D->MTDData)->OTPBase)) {
        return RAW_DISK_ERR_NO_SPACE;
    }

    if (!D->is_mount) {
        MountDevice(D);
    }

    if( retrieve_FDM_lock() != FS_NO_ERROR) {
        return RAW_DISK_ERR_FDM_LOCKED;
    }

    if (D->MTDDriver->OTPAccess) {
	    result = D->MTDDriver->OTPAccess(D->MTDData, FS_OTP_READ, (uint32_t)otp_addr, (void *)data_ptr, len);
    }

    release_FDM_lock();

    if (result != FLASH_DISK_DONE) {
        return RAW_DISK_ERR_PROG_FAIL;
    }
    return RAW_DISK_ERR_NONE;

}


int32_t OTP_Lock(NOR_FLASH_DISK_Data *D)
{
    int32_t result = FLASH_DISK_DONE;

    if (!D->is_mount) {
        MountDevice(D);
    }

    if( retrieve_FDM_lock() != FS_NO_ERROR) {
        return RAW_DISK_ERR_FDM_LOCKED;
    }

    if (D->MTDDriver->OTPAccess) {
	    result = D->MTDDriver->OTPAccess(D->MTDData, FS_OTP_LOCK, 0, NULL, 0);
    }

    release_FDM_lock();

    if (result != FLASH_DISK_DONE) {
        return RAW_DISK_ERR_PROG_FAIL;
    }
    return RAW_DISK_ERR_NONE;

}


hal_flash_status_t hal_flash_otp_write(uint32_t address, const uint8_t *data, uint32_t length)
{
    NOR_FLASH_DISK_Data *D = ENTIRE_DISK_DRIVER_DATA;
    int32_t result;

    result = OTP_Write_Data(D, address, (uint8_t *)data, length);
    if (result != RAW_DISK_ERR_NONE) {
        return get_rawdisk_error_code(result);
    }
    return HAL_FLASH_STATUS_OK;
}

hal_flash_status_t hal_flash_otp_read(uint32_t start_address, uint8_t *buffer, uint32_t length)
{
    NOR_FLASH_DISK_Data *D = ENTIRE_DISK_DRIVER_DATA;
    int32_t result;

	result = OTP_Read_Data(D, start_address, buffer, length);
    if (result != RAW_DISK_ERR_NONE) {
        return get_rawdisk_error_code(result);
    }
    return HAL_FLASH_STATUS_OK;
}

hal_flash_status_t hal_flash_otp_lock(void)
{
    NOR_FLASH_DISK_Data *D = ENTIRE_DISK_DRIVER_DATA;
    int32_t result;

	result = OTP_Lock(D);
    if (result != RAW_DISK_ERR_NONE) {
        return get_rawdisk_error_code(result);	
    }
    return HAL_FLASH_STATUS_OK;
}
#endif


//-----------------------------------------------------------------------------------------------
// Raw Disk API/ TC01 Disk/ Backup Partition Unit Test
//-----------------------------------------------------------------------------------------------
//#define NOR_RAW_DISK_UNIT_TEST
#define NOR_RAW_DISK_UNIT_TEST_REPEAT true
//#define NOR_RAW_DISK_UNIT_TEST_REPEAT false


#ifdef NOR_RAW_DISK_UNIT_TEST

#include "drvflash_ut.h"
#include "kal_public_api.h"

#include "hal_flash_custom_memorydevice.h"

#define BUFF_SIZE 2048
uint8_t buff[BUFF_SIZE];
uint8_t buff2[BUFF_SIZE];

#define dbg_text text

extern char text[200];

#define TC01_DISK_PAGE_PER_BLOCK (TC01_DISK_BLOCK_SIZE/TC01_DISK_PAGE_SIZE)
//return true if data is not consistant

extern NOR_FLASH_DISK_Data EntireDiskDriveData;

extern bool CompareData(void *Address, void *Data, uint32_t Length);

void NOR_CUSTOMER_TC01_DISK_Unit_Test(void);

void  NOR_Full_Driver_Unit_Test(void);


void NOR_Raw_Disk_Unit_Test(void)
{
    while (NOR_RAW_DISK_UNIT_TEST_REPEAT) {

        basic_log("----- Read Raw Disk API UT Start -----\n\r");


        NOR_Full_Driver_Unit_Test();


        //basic_log("----- Raw Disk API UT Start -----\n\r");
        //TBD

        NOR_CUSTOMER_TC01_DISK_Unit_Test();

        //basic_log("----- Backup Partition UT Start -----\n\r");
        //TBD

        basic_log("----- Read Raw Disk API UT End -----\n\r");


//   basic_log("NOR_Raw_Disk_Unit_Test Finished.\n\r");



        kal_sleep_task(10);
    }
}


#if defined(__NOR_FULL_DRIVER__) && !defined(__MINI_BOOTLOADER__)

void  NOR_Full_Driver_Unit_Test(void)
{

    uint32_t i, j;
    uint32_t addr = 0;
    uint32_t pageIdx, blkIdx, blkSize;
    uint32_t totalBlk = 0;



    NOR_init();


    for (blkIdx = 0; blkIdx < EntireDiskDriveData.FlashInfo.TotalBlocks ; blkIdx++) {

        for (pageIdx = 0; pageIdx < NOR_BlockSize(blkIdx) / NOR_FLASH_PAGE_SIZE; pageIdx++) {
            NOR_ReadPhysicalPage(blkIdx, pageIdx, buff);

            if (CompareData(buff, (uint8_t *)addr, NOR_FLASH_PAGE_SIZE)) {
                ASSERT(0);
            }

            addr += NOR_FLASH_PAGE_SIZE;
        }
    }

}
#else

void  NOR_Full_Driver_Unit_Test(void)
{
    ;
}


#endif


#ifdef __UBL__


bool CompareData(void *Address, void *Data, uint32_t Length)
{
    uint32_t i;

    if ((uint32_t)Address & 0x3 == 0 && (uint32_t)Data & 0x3 == 0) {
        uint32_t *Source = (uint32_t *)Address;
        uint32_t *Dest = (uint32_t *)Data;
        for (i = 0; i < (Length >> SFC_GENERIC_2_BIT_OFFSET); i++) {
            if (Source[i] != Dest[i]) {
                return true;
            }
        }
    } else if ((uint32_t)Address & 0x1 == 0 && (uint32_t)Data & 0x1 == 0) {
        uint16_t *Source = (uint16_t *)Address;
        uint16_t *Dest = (uint16_t *)Data;
        for (i = 0; i < (Length >> SFC_GENERIC_1_BIT_OFFSET); i++) {
            if (Source[i] != Dest[i]) {
                return true;
            }
        }
    } else {
        uint8_t *Source = (uint8_t *)Address;
        uint8_t *Dest = (uint8_t *)Data;
        for (i = 0; i < Length; i++) {
            if (Source[i] != Dest[i]) {
                return true;
            }
        }
    }
    return false;
}


#endif //__UBL__


#ifdef __CUSTOMER_TC01_DISK__
void NOR_CUSTOMER_TC01_DISK_Unit_Test(void)
{

    uint32_t i, j;


    basic_log("----- TC01 Disk UT Start -----\n\r");

    basic_log("[1] Erase all blocks.\n\r");

    for (i = 0 ; i < NOR_BOOTING_NOR_DISK1_SIZE / TC01_DISK_BLOCK_SIZE; i++) {
        EraseRawDataItem(i);
        kal_sleep_task(10);
    }

    basic_log("[2] Write page data sequencial.\n\r");

    for (i = 0 ; i < NOR_BOOTING_NOR_DISK1_SIZE / TC01_DISK_BLOCK_SIZE; i++) {
        for (j = 0; j < TC01_DISK_PAGE_PER_BLOCK; j++) {
            memset(buff, i * TC01_DISK_PAGE_PER_BLOCK + j, BUFF_SIZE);
            WriteRawDataItem(i, j, BUFF_SIZE / (j + 1), buff);
            ReadRawDataItem(i, j, BUFF_SIZE, buff2);
            if (true == CompareData(buff, buff2, BUFF_SIZE / (j + 1))) {
                ASSERT(0);
            }
            kal_sleep_task(10);
        }
    }

    basic_log("[3] Check page data .\n\r");

    for (i = 0 ; i < NOR_BOOTING_NOR_DISK1_SIZE / TC01_DISK_BLOCK_SIZE; i++) {
        for (j = 0; j < TC01_DISK_PAGE_PER_BLOCK; j++) {
            memset(buff, i * TC01_DISK_PAGE_PER_BLOCK + j, BUFF_SIZE);
            ReadRawDataItem(i, j, BUFF_SIZE, buff2);
            if (true == CompareData(buff, buff2, BUFF_SIZE / (j + 1))) {
                ASSERT(0);
            }
            kal_sleep_task(10);
        }
    }

    basic_log("[4] Erease block one by one.\n\r");

    for (i = 0 ; i < NOR_BOOTING_NOR_DISK1_SIZE / TC01_DISK_BLOCK_SIZE; i++) {
        for (j = 0; j < TC01_DISK_PAGE_PER_BLOCK; j++) {
            memset(buff, i * TC01_DISK_PAGE_PER_BLOCK + j, BUFF_SIZE);
            ReadRawDataItem(i, j, BUFF_SIZE, buff2);
            if (true == CompareData(buff, buff2, BUFF_SIZE / (j + 1))) {
                ASSERT(0);
            }
        }

        EraseRawDataItem(i);

        memset(buff, 0xff, BUFF_SIZE);

        for (j = 0; j < TC01_DISK_PAGE_PER_BLOCK; j++) {
            ReadRawDataItem(i, j, BUFF_SIZE, buff2);
            if (true == CompareData(buff, buff2, BUFF_SIZE)) {
                ASSERT(0);
            }
        }
        kal_sleep_task(10);
    }



}
#else // !__CUSTOMER_TC01_DISK__
void NOR_CUSTOMER_TC01_DISK_Unit_Test(void)
{
    ;
}
#endif //__CUSTOMER_TC01_DISK__

#endif //NOR_RAW_DISK_UNIT_TEST

#else //! HAL_FLASH_MODULE_ENABLED
signed int NOR_init(void)
{
    return 0;
}
#endif //#ifdef HAL_FLASH_MODULE_ENABLED


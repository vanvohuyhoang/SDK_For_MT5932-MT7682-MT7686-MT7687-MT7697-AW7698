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
 
/*
 *******************************************************************************
 PART 1:
   FLASH CONFIG Options Definition here
 *******************************************************************************
*/

#ifndef __FLASH_OPT_GEN__
#define __FLASH_OPT_GEN__
#include "memory_map.h"

#define __PAGE_BUFFER_PROGRAM__
#define __SERIAL_FLASH__
#define SF_DAL_MXIC
#define SF_DAL_WINBOND
#define __NON_INTEL_SIBLEY__

#define __SINGLE_BANK_NOR_DEVICE__

/*
 *******************************************************************************
 PART 2:
   FLASH FDM FEATURE CONFIG PARAMETERS translated from Manual custom_Memorydevice.h
 *******************************************************************************
*/

#define BUFFER_PROGRAM_ITERATION_LENGTH  (64)

/*
 *******************************************************************************
 PART 3:
   FLASH GEOMETRY translated from MEMORY DEVICE DATABASE
 *******************************************************************************
*/

/* NOR flash maximum block size (Byte) in file system region */
#define NOR_BLOCK_SIZE       0x8000
#define NOR_DISK0_BLOCK_SIZE 0x0

#if 0
/* NAND flash total size (MB). PLEASE configure it as 0 if it is unknown. */
#define NAND_TOTAL_SIZE 0

/* NAND flash block size (KB). PLEASE configure it as 0 if it is unknown. */
#define NAND_BLOCK_SIZE 0
#endif


/*
 *******************************************************************************
 PART 4:
   FLASH FAT CONFIG translated from Manual custom_Memorydevice.h
 *******************************************************************************
*/

/* File system block size:
     if FS size  is less than 127 * 4 KB, please set block size 4KB;
*/
#define FS_NOR_BLOCK_SIZE                  (0x8000)        
/* File system base address which is a offset from flash start. */
#define ROM_FS_BASE                        (0x00100000)
/*File system size */
#define ROM_FS_LENGHT                      (0x002E0000)
#define NOR_FLASH_BASE_ADDRESS_DEFAULT     (ROM_FS_BASE)
#define NOR_ALLOCATED_FAT_SPACE_DEFAULT    (ROM_FS_LENGHT)
#define FOTA_DM_FS_OFFSET                  (0x0)
#define FOTA_DM_FS_SECTOR_OFFSET           (0)

/* NOR flash total block count =  127 (FDM4.0 max) - bin_size - NVDM - wifi */
#define CMEM_MAX_BLOCKS     (ROM_FS_LENGHT/FS_NOR_BLOCK_SIZE)    //127 - bin size - Fota - (nvdm + wifi) /FS_NOR_BLOCK_SIZE
/* NOR flash Max sectors */
#define CMEM_MAX_SECTORS   (CMEM_MAX_BLOCKS * (FS_NOR_BLOCK_SIZE/512)) 


#endif  //__FLASH_OPT_GEN__

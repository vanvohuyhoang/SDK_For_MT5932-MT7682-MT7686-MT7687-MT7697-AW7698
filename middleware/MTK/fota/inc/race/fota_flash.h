/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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
 
#ifndef __FOTA_FLASH_H__
#define __FOTA_FLASH_H__


#include "fota_platform.h"
#include "fota_util.h"


typedef enum
{
    FLASH_PARTITION_TYPE_PARTITION_TABLE,
    FLASH_PARTITION_TYPE_SECURITY_HEADER,
    FLASH_PARTITION_TYPE_BOOTLOADER,
    FLASH_PARTITION_TYPE_N9,
    FLASH_PARTITION_TYPE_CM4_FW,
    FLASH_PARTITION_TYPE_DSP0,
    FLASH_PARTITION_TYPE_DSP1,
    FLASH_PARTITION_TYPE_RESVD,
    FLASH_PARTITION_TYPE_NVDM,
    FLASH_PARTITION_TYPE_FOTA,
    FLASH_PARTITION_TYPE_FOTA_EXT,
    FLASH_PARTITION_TYPE_ROFS,

    FLASH_PARTITION_TYPE_MAX = 0xFF       
} flash_partition_type_enum;


typedef struct
{
    flash_partition_type_enum partition_type;
    uint32_t BinaryId;  /*bit 0 ~ bit 2: 1 sec header; 1 bl; 2 n9; 3 cm4; 4 dsp0; 5 dsp1;*/
    uint32_t LoadAddressHigh;  /* Reserved for msdc the address space of which may be larger than 4GB. */
    uint32_t LoadAddressLow;   /* Relative address. 0x0 for the start address of the flash. */
    uint32_t BinaryLengthHigh; /* Reserved for msdc the address space of which may be larger than 4GB. */
    uint32_t BinaryLengthLow;
    bool is_external;
}PACKED fota_flash_partition_info;


FotaStorageType fota_flash_get_storage_type(void);

void fota_flash_bootup(void);

#ifdef BL_FOTA_ENABLE
bool fota_flash_is_addr_range_valid(uint32_t addr, uint32_t length);
#endif

fota_flash_partition_info *fota_flash_get_partition_info(flash_partition_type_enum partition_type);

FOTA_ERRCODE fota_flash_get_fota_partition_info(FotaStorageType * storage_type,uint32_t * fota_address,uint32_t * length);

FOTA_ERRCODE fota_flash_init(bool is_int);

FOTA_ERRCODE fota_flash_erase(uint32_t start_address, hal_flash_block_t block_type, bool is_int);

FOTA_ERRCODE fota_flash_write(uint32_t start_address, const uint8_t *data, uint32_t length, bool is_int);

FOTA_ERRCODE fota_flash_read(uint32_t start_address, uint8_t *buffer, uint32_t length, bool is_int);

bool fota_flash_config_init(fota_flash_partition_info **partition_info);

/* This API is used to config the start address and the length of the FOTA parition in the external flash if the
 * package is intended to be stored in the external flash. If it's not called, FOTA_EXT_RESERVED_BASE_DEFAULT
 * and FOTA_EXT_RESERVED_LENGTH_DEFAULT are used as the start address and the length by default.
 * start_address can be with or without the offset of SPI_SERIAL_FLASH_ADDRESS. 
 * It must be called after fota_flash_config_init() and before FOTA is started, if it need be called.
 */
FOTA_ERRCODE fota_flash_config_fota_partition_in_external_flash(uint32_t start_address, uint32_t length);

FOTA_ERRCODE fota_flash_get_partition_erase_status(uint8_t *erase_status,
                                                           uint16_t *erase_status_size,
                                                           uint32_t start_address,
                                                           uint32_t length,
                                                           bool is_int);
#endif /* __FOTA_FLASH_H__ */


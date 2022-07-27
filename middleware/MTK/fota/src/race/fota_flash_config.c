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
 
#include "fota_flash.h"
#include "memory_map.h"
#if FOTA_STORE_IN_EXTERNAL_FLASH
#include "bsp_flash.h"
#endif
#include "fota_platform.h"


#ifdef ROM_BASE
#define FLASH_CONFIG_BASE    (ROM_BASE)
#else
#define FLASH_CONFIG_BASE    (BL_BASE)
#endif


#if FOTA_STORE_IN_EXTERNAL_FLASH
/* Start address of FOTA partition in the external flash. 
  * The address can be with or without the offset of SPI_SERIAL_FLASH_ADDRESS. 
  */
#define FOTA_EXT_RESERVED_BASE_DEFAULT    (FOTA_RESERVED_BASE - FLASH_CONFIG_BASE)
/* Length of FOTA partition in the external flash */
#define FOTA_EXT_RESERVED_LENGTH_DEFAULT  (FOTA_RESERVED_LENGTH)
#else
#define FOTA_EXT_RESERVED_BASE_DEFAULT    (0x0)
#define FOTA_EXT_RESERVED_LENGTH_DEFAULT  (0x0)
#endif


#if (PRODUCT_VERSION == 2523)
/* Only include FOTA partition and partitons which can be updated by FOTA */
static fota_flash_partition_info s_flash_table[] =
{
#ifdef BL_FOTA_ENABLE
    {
        .partition_type = FLASH_PARTITION_TYPE_CM4_FW,
        .BinaryId = 0x03,
        .LoadAddressHigh = 0,
        .LoadAddressLow = CM4_BASE - FLASH_CONFIG_BASE,
        .BinaryLengthHigh = 0,
        .BinaryLengthLow = CM4_LENGTH,
        .is_external = FALSE
    },
#endif
    {
        .partition_type = FLASH_PARTITION_TYPE_FOTA,
        .BinaryId = 0x06,
        .LoadAddressHigh = 0,
        .LoadAddressLow = FOTA_RESERVED_BASE - FLASH_CONFIG_BASE,
        .BinaryLengthHigh = 0,
        .BinaryLengthLow = FOTA_RESERVED_LENGTH,  /* The last block of fota temp partition is reserved for triggered flag & update info */
        .is_external = FALSE
    },
    {
        // TODO: update external FOTA info
        .partition_type = FLASH_PARTITION_TYPE_FOTA_EXT,
        .BinaryId = 0x06,
        .LoadAddressHigh = 0,
        .LoadAddressLow = FOTA_RESERVED_BASE - FLASH_CONFIG_BASE,
        .BinaryLengthHigh = 0,
        .BinaryLengthLow = FOTA_RESERVED_LENGTH,  /* The last block of fota temp partition is reserved for triggered flag & update info */
        .is_external = TRUE
    },
    {
        .partition_type = FLASH_PARTITION_TYPE_MAX,
        .BinaryId = 0,
        .LoadAddressHigh = 0,
        .LoadAddressLow = 0,
        .BinaryLengthHigh = 0,
        .BinaryLengthLow = 0,
        .is_external = FALSE
    }
};

#else

/* Only include FOTA partition as well as partitons which can be updated by FOTA */
static fota_flash_partition_info s_flash_table[] =
{
#ifdef BL_FOTA_ENABLE
    {
        .partition_type = FLASH_PARTITION_TYPE_N9,
        .BinaryId = 0x02,
        .LoadAddressHigh = 0,
        .LoadAddressLow = 0,
        .BinaryLengthHigh = 0,
        .BinaryLengthLow = 0,
        .is_external = FALSE
    },
    {
        .partition_type = FLASH_PARTITION_TYPE_CM4_FW,
        .BinaryId = 0x03,
        .LoadAddressHigh = 0,
        .LoadAddressLow = 0,
        .BinaryLengthHigh = 0,
        .BinaryLengthLow = 0,
        .is_external = FALSE
    },
    {
        .partition_type = FLASH_PARTITION_TYPE_DSP0,
        .BinaryId = 0x04,
        .LoadAddressHigh = 0,
        .LoadAddressLow = 0,
        .BinaryLengthHigh = 0,
        .BinaryLengthLow = 0,
        .is_external = FALSE
    },
    {
        .partition_type = FLASH_PARTITION_TYPE_DSP1,
        .BinaryId = 0x05,
        .LoadAddressHigh = 0,
        .LoadAddressLow = 0,
        .BinaryLengthHigh = 0,
        .BinaryLengthLow = 0,
        .is_external = FALSE
    },
    {
        .partition_type = FLASH_PARTITION_TYPE_NVDM,
        .BinaryId = 0x07,
        .LoadAddressHigh = 0,
        .LoadAddressLow = 0,
        .BinaryLengthHigh = 0,
        .BinaryLengthLow = 0,
        .is_external = FALSE
    },            
#if (PRODUCT_VERSION == 1552) && defined(ROFS_BASE)
    {
        .partition_type = FLASH_PARTITION_TYPE_ROFS,
        .BinaryId = 0x08,
        .LoadAddressHigh = 0,
        .LoadAddressLow = 0,
        .BinaryLengthHigh = 0,
        .BinaryLengthLow = 0,
        .is_external = FALSE
    },
#endif
#endif /* BL_FOTA_ENABLE */
    {
        .partition_type = FLASH_PARTITION_TYPE_FOTA,
        .BinaryId = 0x06,
        .LoadAddressHigh = 0,
        .LoadAddressLow = 0,
        .BinaryLengthHigh = 0,
        .BinaryLengthLow = 0,  /* The last block of fota temp partition is reserved for triggered flag & update info */
        .is_external = FALSE
    },
    {
        /* FOTA_EXT is not included in the partition table. So just set the BinaryId to be 0xFF. */
        .partition_type = FLASH_PARTITION_TYPE_FOTA_EXT,
        .BinaryId = 0xFF,
        .LoadAddressHigh = 0,
        .LoadAddressLow = 0,
        .BinaryLengthHigh = 0,
        .BinaryLengthLow = 0,  /* The last block of fota temp partition is reserved for triggered flag & update info */
        .is_external = TRUE
    },
    {
        .partition_type = FLASH_PARTITION_TYPE_MAX,
        .BinaryId = 0,
        .LoadAddressHigh = 0,
        .LoadAddressLow = 0,
        .BinaryLengthHigh = 0,
        .BinaryLengthLow = 0,
        .is_external = FALSE
    }
};
#endif

bool fota_flash_config_init(fota_flash_partition_info **partition_info)
{
#if (PRODUCT_VERSION != 2523)
    int i = 0;
#endif

    if (!partition_info || *partition_info)
    {
        return false;
    }

#if (PRODUCT_VERSION != 2523)

    while (FLASH_PARTITION_TYPE_MAX != s_flash_table[i].partition_type)
    {
        switch (s_flash_table[i].partition_type)
        {
#ifdef BL_FOTA_ENABLE
            case FLASH_PARTITION_TYPE_N9:
            {
                s_flash_table[i].LoadAddressLow = N9_BASE - FLASH_CONFIG_BASE;
                s_flash_table[i].BinaryLengthLow = N9_LENGTH;
                break;
            }

            case FLASH_PARTITION_TYPE_CM4_FW:
            {
                s_flash_table[i].LoadAddressLow = CM4_BASE - FLASH_CONFIG_BASE;
                s_flash_table[i].BinaryLengthLow = CM4_LENGTH;
                break;
            }

            case FLASH_PARTITION_TYPE_DSP0:
            {
                s_flash_table[i].LoadAddressLow = DSP0_BASE - FLASH_CONFIG_BASE;
                s_flash_table[i].BinaryLengthLow = DSP0_LENGTH;
                break;
            }

            case FLASH_PARTITION_TYPE_DSP1:
            {
                s_flash_table[i].LoadAddressLow = DSP1_BASE - FLASH_CONFIG_BASE;
                s_flash_table[i].BinaryLengthLow = DSP1_LENGTH;
                break;
            }
            
            case FLASH_PARTITION_TYPE_NVDM:
            {
                s_flash_table[i].LoadAddressLow = ROM_NVDM_BASE - FLASH_CONFIG_BASE;
                s_flash_table[i].BinaryLengthLow = ROM_NVDM_LENGTH;
                break;
            }

#if (PRODUCT_VERSION == 1552) && defined(ROFS_BASE)
            case FLASH_PARTITION_TYPE_ROFS:
            {
                s_flash_table[i].LoadAddressLow = ROFS_BASE - FLASH_CONFIG_BASE;
                s_flash_table[i].BinaryLengthLow = ROFS_LENGTH;
                break;
            }
#endif
#endif /* BL_FOTA_ENABLE */
            case FLASH_PARTITION_TYPE_FOTA:
            {
                s_flash_table[i].LoadAddressLow = FOTA_RESERVED_BASE - FLASH_CONFIG_BASE;
                s_flash_table[i].BinaryLengthLow = FOTA_RESERVED_LENGTH;
                break;
            }

            case FLASH_PARTITION_TYPE_FOTA_EXT:
            {
#if FOTA_STORE_IN_EXTERNAL_FLASH
                s_flash_table[i].LoadAddressLow = FOTA_EXT_RESERVED_BASE_DEFAULT & (~SPI_SERIAL_FLASH_ADDRESS);
#else
                s_flash_table[i].LoadAddressLow = FOTA_EXT_RESERVED_BASE_DEFAULT;
#endif
                s_flash_table[i].BinaryLengthLow = FOTA_EXT_RESERVED_LENGTH_DEFAULT;
                break;
            }
            
            default:
            {
                FOTA_LOG_MSGID_W("Unknown partition type:%d",1, s_flash_table[i].partition_type);
                return false;
            }
        }
        i++;
    }
#endif

    *partition_info = s_flash_table;
    return true;
}


FOTA_ERRCODE fota_flash_config_fota_partition_in_external_flash(uint32_t start_address,
                                                                          uint32_t length)
{
#if FOTA_STORE_IN_EXTERNAL_FLASH
    uint32_t i = 0;

    if (!length)
    {
        return FOTA_ERRCODE_INVALID_PARAMETER;
    }

    while (FLASH_PARTITION_TYPE_MAX != s_flash_table[i].partition_type)
    {
        if (FLASH_PARTITION_TYPE_FOTA_EXT == s_flash_table[i].partition_type)
        {
            s_flash_table[i].LoadAddressLow = start_address & (~SPI_SERIAL_FLASH_ADDRESS);
            s_flash_table[i].BinaryLengthLow = length;
            return FOTA_ERRCODE_SUCCESS;
        }
        i++;
    }

    return FOTA_ERRCODE_FAIL;
#else
    return FOTA_ERRCODE_UNSUPPORTED;
#endif
}


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
 
#include "hal_flash.h"
#ifdef HAL_FLASH_MODULE_ENABLED
#include "memory_attribute.h"
#include "hal_flash_opt.h"
#include "hal_flash_cfi_internal.h"
#include "hal_flash_cfi.h"
#include "hal_flash_sf.h"
#include "hal_flash_custom_memorydevice.h"


extern uint32_t SaveAndSetIRQMask(void);
extern void RestoreIRQMask(uint32_t irq);
extern bool CMEM_CheckValidDeviceID(uint8_t *id);

//-----------------------------------------------------------------------------
/*!
  @brief
    Read NOR Flash ID

  @param[in] BaseAddr Pointer to the NOR Flash memory base address
  @param[out] flashid Array of FlashID, the maximum length is 4
  @retval
    Query result, byte aligned.

  @remarks
    Limited to the length return value (32 bits), the given length shall not exceed 4 (bytes)
    This function must be put in RAM
*/

ATTR_TEXT_IN_RAM void NOR_ReadID(const uint16_t CS, volatile FLASH_CELL *BaseAddr, uint16_t *flashid)
{
    uint32_t savedMask;

    // Read Serial Flash ID
    {
        uint8_t cmd, id[SF_FLASH_ID_LENGTH], i;

        cmd = SF_CMD_READ_ID_QPI;
        savedMask = SaveAndSetIRQMask();
        SFI_Dev_Command_Ext(CS, &cmd, id, 1, SF_FLASH_ID_LENGTH);
        RestoreIRQMask(savedMask);

        if (id[0] == 0x00 || id[0] == 0xFF || (CMEM_CheckValidDeviceID(id) == false)) {
            cmd = SF_CMD_READ_ID;
            savedMask = SaveAndSetIRQMask();
            SFI_Dev_Command_Ext(CS, &cmd, id, 1, SF_FLASH_ID_LENGTH);
            RestoreIRQMask(savedMask);
        }

        for (i = 0; i < SF_FLASH_ID_LENGTH; i++) {
            flashid[i] = id[i];
        }
    }
    return;
}

#endif //HAL_FLASH_MODULE_ENABLED


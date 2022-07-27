/* Copyright Statement:
 *
 * (C) 2018  Airoha Technology Corp. All rights reserved.
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


#ifdef MTK_FATFS_ON_SERIAL_NOR_FLASH
#include "snor_port.h"
#include "flash_manager.h"

int32_t FDM_MountDevice(int32_t DeviceNumber, int32_t DeviceType, uint32_t Flags)
{
    int32_t status;
#ifdef MTK_FATFS_ON_SERIAL_NOR_FLASH
    status = SNOR_MountDevice(DeviceNumber, DeviceType, Flags);
#endif
    return status;
}

int32_t FDM_ReadSectors(uint32_t Sector, uint32_t Sectors, void * Buffer)
{
    int32_t status = -1;
#ifdef MTK_FATFS_ON_SERIAL_NOR_FLASH
    status = SNOR_ReadSectors(Sector, Sectors, Buffer);
#endif
    return status;
}

int32_t FDM_WriteSectors(uint32_t Sector, uint32_t Sectors, void * Buffer)
{
    int32_t status = -1;
#ifdef MTK_FATFS_ON_SERIAL_NOR_FLASH
    status = SNOR_WriteSectors(Sector, Sectors, Buffer);
#endif
    return status;
}	

uint32_t FDM_GetSectors()
{
   uint32_t sectors = 0;
#ifdef MTK_FATFS_ON_SERIAL_NOR_FLASH
    //get sectors number
    sectors = SNOR_GetSectors();
#endif
    return sectors;
}

uint32_t FDM_GetBlockSize()
{
    uint32_t blocksize = 0;
#ifdef MTK_FATFS_ON_SERIAL_NOR_FLASH
    //erase block size in unit of sector
    blocksize = SNOR_GetBlockSize();
#endif
    return blocksize;
}

#endif //MTK_FATFS_ON_SERIAL_NOR_FLASH


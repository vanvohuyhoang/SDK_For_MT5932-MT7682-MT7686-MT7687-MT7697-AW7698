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

#ifndef __SNOR_PORT_H__
#define __SNOR_PORT_H__
#ifdef MTK_FATFS_ON_SERIAL_NOR_FLASH
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//#include "syslog.h"
//#define NOR_FDM_LOGW(fmt, arg...)  LOG_W(common, "NOR FDM]: ",fmt,##arg)
//#define NOR_FDM_LOGE(fmt, arg...)  LOG_E(common, "NOR FDM]: ",fmt,##arg)
//#define NOR_FDM_LOGI(fmt, arg...)  LOG_I(common, "NOR FDM]: ",fmt,##arg)


int32_t SNOR_MountDevice(int32_t DeviceNumber, int32_t DeviceType, uint32_t Flags);
int32_t SNOR_ReadSectors(uint32_t Sector, uint32_t Sectors, void * Buffer);
int32_t SNOR_WriteSectors(uint32_t Sector, uint32_t Sectors, void * Buffer);
uint32_t SNOR_GetSectors(void);
uint32_t SNOR_GetBlockSize(void);

bool INT_QueryExceptionStatus(void);
void FDM_LOCK_Init(void);
void FDM_LOCK(void);
void FDM_UNLOCK(void);
void FDM_Set_Reclaim_Event(uint32_t event);
void CreateFDMLock(void);
void GetFDMLock(void);
void FreeFDMLock(void);
uint32_t FDM_query_reclaim_event(void);
void *get_ctrl_buffer(uint32_t size);
void free_ctrl_buffer(void *pdata);

#ifdef FREERTOS_ENABLE
#define CHECK_WAIT_STATE(n, s)\
   if( 0 == FDM_query_reclaim_event())\
   {\
      status = s;\
      status |= (n << 24);\
      break;\
   }
#else
#define CHECK_WAIT_STATE(n, s)
#endif

#define FLASH_ROM_BASE (0x08000000)

#ifdef __cplusplus
}
#endif

#endif
#endif


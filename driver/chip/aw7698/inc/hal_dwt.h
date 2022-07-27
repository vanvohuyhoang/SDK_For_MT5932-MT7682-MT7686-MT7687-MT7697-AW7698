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
 
#ifndef __HAL_DWT_H__
#define __HAL_DWT_H__

#include "hal_platform.h"

#ifdef HAL_DWT_MODULE_ENABLED

typedef enum { // now only suppuort for EMITRANGE=0, CYCMATCH = 0
    COMP_DISABLE = 0,
    WDE_INST = 4, // 0x0100: Generate PC watchpoint debug event whem instruction fetches
    WDE_DATA_RO,  //    0x0101: Generate watchpoint debug event when read data accesses
    WDE_DATA_WO,  //    0x0110: Generate watchpoint debug event when write data accesses
    WDE_DATA_RW   //    0x0111: Generate watchpoint debug event when read/write data accesses
} DWT_FUNC_TYPE;

typedef enum {
    HAL_DWT_0 = 0,
    HAL_DWT_1 = 1,
    HAL_DWT_2 = 2,
    HAL_DWT_3 = 3,  /* used for pxCurrentTCB stack overflow check */
    HAL_DWT_MAX
} DWT_COMPARATOR_PORT;

/* DWT APIs **********************************************************/
void hal_dwt_reset(void);
void hal_dwt_init(void);
void hal_dwt_dump_status(void);

int32_t hal_dwt_request_watchpoint(uint32_t index, uint32_t addr_base, uint32_t addr_mask, DWT_FUNC_TYPE func);

#endif /* HAL_DWT_MODULE_ENABLED */
#endif /* __DWT_H__ */

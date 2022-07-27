/* Copyright Statement:
 *
 * (C) 2017-2017  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */


#ifndef __HAL_ARB_REGS_H__
#define __HAL_ARB_REGS_H__


//#include <stdint.h>


#define HAL_ARB_REGS_REMAP(_x)      ((uint32_t)_x + (0xC0900000 - 0x60100000))


#ifndef HAL_ARB_REGS_REMAP
/**
 * Define this macro before including this header file to remapping addresses
 * to needed bus addresses.
 */
#define HAL_ARB_REGS_REMAP(_x)      (_x)
#endif


/**
 * This is a duplicated definition of CR base address from *internal* source
 * tree. The addresss are for MCU. Need to be mapped to Cortex-M4 CPU
 * bus addresses using HAL_ARB_REGS_REMAP.
 */
#define ARB_base                    (HAL_ARB_REGS_REMAP(0x60100000) + 0x20000)

#define ARB_PTR0                    (ARB_base + 0x0094)
#define ARB_PTR1                    (ARB_base + 0x0098)
#define ARB_PTR2                    (ARB_base + 0x009C)
#define ARB_RWPCFR0                 (ARB_base + 0x00C0)
#define ARB_PB1WERWR                (ARB_base + 0x00CC)
#define ARB_RPTDD0                  (ARB_base + 0x0140)

//#define  ARB_PB1WERWR           (ARB_base + 0x00CC)
    #define     FIELD_BT1WE_EARUP_PERIOD                BITS(16,23)
    #define     FIELD_BT1WE_EARUP_PERIOD_OFFSET         16


#endif /* __HAL_ARB_REGS_H__ */

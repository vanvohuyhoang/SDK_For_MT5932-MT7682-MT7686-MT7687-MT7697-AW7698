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
 
/*****************************************************************************
 *
 * Filename:
 * ---------
 *    gpt_sw.h
 *
 * Project:
 * --------
 *   Maui_Software
 *
 * Description:
 * ------------
 *   This file is intends for GPT driver.
 *
 * Author:
 * -------
 *  James Liu
 *
 ****************************************************************************/
#ifndef GPT_SW_H
#define GPT_SW_H

#include "hal_gpt.h"
#include "top.h"

#ifdef HAL_GPT_MODULE_ENABLED

#include <stdbool.h>

//GPT address
#define GPT_ISR            (CM4_GPT_BASE+0x00)
#define GPT_IER            (CM4_GPT_BASE+0x04)
#define GPT0_CTRL          (CM4_GPT_BASE+0x10)
#define GPT0_ICNT          (CM4_GPT_BASE+0x14)
#define GPT1_CTRL          (CM4_GPT_BASE+0x20)
#define GPT1_ICNT          (CM4_GPT_BASE+0x24)
#define GPT2_CTRL          (CM4_GPT_BASE+0x30)
#define GPT2_CNT           (CM4_GPT_BASE+0x34)
#define GPT0_CNT           (CM4_GPT_BASE+0x40)
#define GPT1_CNT           (CM4_GPT_BASE+0x44)

#define GPT3_CTRL          (CM4_GPT_BASE+0x50)
#define GPT3_INIT          (CM4_GPT_BASE+0x54)
#define GPT3_CNT           (CM4_GPT_BASE+0x58)
#define GPT3_EXPIRE        (CM4_GPT_BASE+0x5C)

#define GPT4_CTRL          (CM4_GPT_BASE+0x60)
#define GPT4_INIT          (CM4_GPT_BASE+0x64)
#define GPT4_CNT           (CM4_GPT_BASE+0x68)

//GPT_ISR
#define GPT0_INT                (0x01)
#define GPT1_INT                (0x02)

//GPT_IER
#define GPT0_INT_EN             (0x01)
#define GPT1_INT_EN             (0x02)

//GPT_CTRL
#define GPT_CTRL_EN             (0x01)
#define GPT_CTRL_AUTOMODE       (0x02)

//GPT3
#define GPT3_OSC_CNT_1US_SHIFT	(16)
#define GPT3_OSC_CNT_1US_MASK	(0x3F)

typedef struct {
    void (*gpt0_func)(void);
    void (*gpt1_func)(void);
    void (*gpt3_func)(void);
} gpt_func;


typedef struct {
    gpt_func       GPT_FUNC;
} GPTStruct;


extern void GPT_ResetTimer(kal_uint32 timerNum, kal_uint32 countValue, bool autoRepeat);
extern void GPT_Start(kal_uint32 timerNum);
extern void GPT_Stop(kal_uint32 timerNum);
extern void GPT_init(kal_uint32 timerNum, kal_uint32 speed_32us, void (*GPT_Callback)(void));
extern void GPT_LISR(void);
extern kal_uint32 GPT_return_current_count(kal_uint32 timerNum);

extern void CM4_GPT2Init(void);
extern void CM4_GPT4Init(void);

#endif

#endif  /* GPT_SW_H */

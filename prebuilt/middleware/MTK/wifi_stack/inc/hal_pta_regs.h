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


#ifndef __HAL_PTA_REGS_H__
#define __HAL_PTA_REGS_H__


//#include <stdint.h>
#include "rt_config.h"

#define HAL_PTA_REGS_REMAP(_x)      ((uint32_t)_x + (0xC0060000 - 0x81060000))


#ifndef HAL_PTA_REGS_REMAP
/**
 * Define this macro before including this header file to remapping addresses
 * to needed bus addresses.
 */
#define HAL_PTA_REGS_REMAP(_x)      (_x)
#endif


/**
 * This is a duplicated definition of CR base address from *internal* source
 * tree. The addresss are for MCU. Need to be mapped to Cortex-M4 CPU
 * bus addresses using HAL_PTA_REGS_REMAP.
 *
 * @note Original source in n9rom/system/init/include/reg_base_7686.h.
 */
#define PTA_base                    HAL_PTA_REGS_REMAP(0x81060000)


#define PTA_CON                     (PTA_base + 0x000)
#define PTA_ARB_MODE_ADDR           (PTA_base + 0x00C)
#define PTA_CLK_CFG                 (PTA_base + 0x064)
#define BT_ABT_CNT                  (PTA_base + 0x070)
#define WF_ABT_CNT                  (PTA_base + 0x074)
#define BT_TX_CNT                   (PTA_base + 0x078)
#define BT_RX_CNT                   (PTA_base + 0x07c)
#define WF_TX_CNT                   (PTA_base + 0x080)
#define WF_RX_CNT                   (PTA_base + 0x084)
#define WF_BT_CNT_CON               (PTA_base + 0x088)
#define REQ_GEN_CON                 (PTA_base + 0x090)
#define OBS_REQ_GRANT_CON           (PTA_base + 0x0a0)
#define ANT_SWITCH_CON1             (PTA_base + 0x0e0)

#define BCCR0                       (PTA_base + 0x380)
#define BCCR1                       (PTA_base + 0x384)
#define BCCR2                       (PTA_base + 0x310)
#define B1WCR0                      (PTA_base + 0x398)
#define BT_EXT_CLK_CFG              (PTA_base + 0x3d0)
#define B1WECR0                     (PTA_base + 0x3a0)
#define B1WECR1                     (PTA_base + 0x3a4)
#define B1WECR2                     (PTA_base + 0x3a8)
#define B1WECR3                     (PTA_base + 0x3ac)
#define B1WECR4                     (PTA_base + 0x3b0)
#define B1WECR5                     (PTA_base + 0x3b4)
#define B1WECR6                     (PTA_base + 0x3b8)
#define B1WECR7                     (PTA_base + 0x3bc)
#define BMWCR                       (PTA_base + 0x3c0)
#define B1CTMR0                     (PTA_base + 0x3c8)

//#define PTA_CON                     (PTA_base + 0x000)
    #define     WIFI_PTA_EN                     BIT(15)
    #define     WIFI_PTA_EN_OFFSET              15
    #define     BT_PTA_EN                       BIT(14)
    #define     BT_PTA_EN_OFFSET                14    
    #define     BT_RX_DIS_WIFI_RX_EN            BIT(5)
    #define     BT_RX_DIS_WIFI_RX_EN_OFFSET     5
    #define     BT_TX_DIS_WIFI_RX_EN            BIT(4)
    #define     BT_TX_DIS_WIFI_RX_EN_OFFSET     4
    #define     EN_PTA_ARB                      BIT(0)
    #define     EN_PTA_ARB_OFFSET               0
    
//#define PTA_ARB_MODE_ADDR             (PTA_base + 0x00C)
    #define     BT_WF_ARB_OUTBAND                                   BITS(28, 31)
    #define     BT_WF_ARB_OUTBAND_OFFSET                            28
    #define     BT_WF_ARB_INBAND                                    BITS(12, 15)
    #define     BT_WF_ARB_INBAND_OFFSET                             12

//#define BCCR0                       (PTA_base + 0x380)
    #define     BT_RW_TIMEOUT                   BITS(16,23)
    #define     BT_RW_TIMEOUT_OFFSET            16
    #define     BT_PTA_DELAY                    BITS(8,15)
    #define     BT_PTA_DELAY_OFFSET             8
    #define     BT_EXT_MODE                     BITS(1,3)
    #define     BT_EXT_MODE_OFFSET              1
    #define     BT_EXT_EN                       BIT(0)
    #define     BT_EXT_EN_OFFSET                0    

//#define B1WCR0                      (PTA_base + 0x398)
    #define     BT_1W_RSVD_EN                   BIT(31)
    #define     BT_1W_RSVD_EN_OFFSET            31
    #define     BT_1W_RSVD_PAT1                 BITS(24,28)
    #define     BT_1W_RSVD_PAT1_OFFSET          24
    #define     BT_1W_RSVD_PAT0                 BITS(16,20)
    #define     BT_1W_RSVD_PAT0_OFFSET          16
    #define     BT_1W_T8_PERIOD                 BITS(8,15)
    #define     BT_1W_T8_PERIOD_OFFSET          8
    #define     BT_1W_T6_PERIOD                 BITS(0,4)
    #define     BT_1W_T6_PERIOD_OFFSET          0
    
#define OUTBAND_ARB_MODE					1
#define INBAND_ARB_MODE						0

#define PTA_ONE_WIRE_TICK_0         (0xc00c0000)
#define PTA_ONE_WIRE_TICK_1         (0xc0070100)
#define PTA_ONE_WIRE_TICK_2         (0xc0c00004)
#define PTA_ONE_WIRE_TICK_3         (0xc09200cc)


#define MCU_CR_0400                 (0xC0070400)  // write 0x2B000000
#define PTA_CR_0200                 (0xC0060200)  //Read for debug


#endif /* __HAL_PTA_REGS_H__ */

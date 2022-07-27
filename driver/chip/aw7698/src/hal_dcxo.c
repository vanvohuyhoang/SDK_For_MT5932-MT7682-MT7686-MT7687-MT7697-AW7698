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
 
#include "hal_dcxo.h"
#ifdef HAL_DCXO_MODULE_ENABLED
#include "hal_clock_internal.h"
#include "hal_gpt.h"
#include "hal_log.h"
#include "hal_gpt.h"
#include "stdio.h"
#include "stdlib.h"
#include "hal_clock.h"
#include "aw7698.h"

//#define HAL_DCXO_DEBUG_ENABLE
#ifdef  HAL_DCXO_DEBUG_ENABLE
#define debug_log(_message,...) printf(_message, ##__VA_ARGS__)
#else
#define debug_log(_message,...)
#endif

#define SPM_SEQUENCER_32K_REG_1              ((volatile uint32_t*)(SPM_BASE + 0x0104))
#define SPM_SEQUENCER_32K_REG_2              ((volatile uint32_t*)(SPM_BASE + 0x0108))

uint8_t extern_xosc32;
void hal_dcxo_init()
{
    debug_log("DCXO init\r\n");
    uint32_t xtal_type,xosc32_enable;
    xtal_type = hal_clock_fxo_is_26m(); //26M or 40M
#if defined MTK_HAL_SW_EXT_32K_ENABLE
    xosc32_enable = 0;
#elif defined MTK_HAL_SW_INT_32K_ENABLE
    xosc32_enable = 1;
#else
    xosc32_enable = dcxo_get_register_value((uint32_t)SYSTEM_INFOD, 1, 5);
#endif
    hal_clock_enable(HAL_CLOCK_CG_SW_SPM);
    if(xosc32_enable == 0){
    /* use external 32K */
    dcxo_set_xtal_lpm_status(xosc32_enable);
    *SPM_SEQUENCER_32K_REG_1 &= 0xFF00;     //XO_OFF_RISE_SETTLE_TIME = 0
    *SPM_SEQUENCER_32K_REG_2 &= 0xFF00;     //XO_OFF_FALL_SETTLE_TIME = 0
    }else{
    dcxo_set_xtal_lpm_status(xosc32_enable);
    /* SYSTEM_INFOD */
        if (((*(volatile uint32_t *)(0xA2010040)) & 0x20) == 0) {
        /* use external 32K */
        *SPM_SEQUENCER_32K_REG_1 &= 0xFF00;    //XO_OFF_RISE_SETTLE_TIME = 0
        *SPM_SEQUENCER_32K_REG_2 &= 0xFF00;    //XO_OFF_FALL_SETTLE_TIME = 0
        } else {
        /* use internal 32K */
        *SPM_SEQUENCER_32K_REG_1 |= 0x000C;    //XO_OFF_RISE_SETTLE_TIME = 0x0C
        *SPM_SEQUENCER_32K_REG_2 |= 0x001D;    //XO_OFF_FALL_SETTLE_TIME = 0x1D
        }
    }
    if(xtal_type==freq_26M){
        debug_log("Xtal is 26M");
        *XTAL_CTL1 = 0xbb25f;
        *XTAL_CTL2 = 0xbb25f;
        *XTAL_CTL3 = 0xbb25f;
        *XTAL_CTL4 = 0xbb25f;
        *XTAL_CTL5 = 0x69f5f;
        *XTAL_CTL6 = 0x58f3f;
        *XTAL_CTL7 = 0x48a1f;
        *XTAL_CTL8 = 0x40a0f;
        *XTAL_CTL9 = 0x88a0f;
        *XTAL_CTL10 = 0x98f1f;
        *XTAL_CTL11 = 0xa9f3f;
        dcxo_set_register_value((uint32_t) XTAL_CTL16, XTAL_CTL16_XO_DIGPREBUF_STABLE_TIME_MASK, XTAL_CTL16_XO_DIGPREBUF_STABLE_TIME_SHIFT, 0X3b);
        dcxo_set_register_value((uint32_t) XTAL_CTL16, XTAL_CTL16_XO_PREBUF_STABLE_TIME_MASK, XTAL_CTL16_XO_PREBUF_STABLE_TIME_SHIFT, 0x42);
        dcxo_set_register_value((uint32_t)XTAL_CTL16, XTAL_CTL16_XO_RDY_STABLE_TIME_MASK, XTAL_CTL16_XO_RDY_STABLE_TIME_SHIFT, 0x63);
#ifdef XO_3225
            dcxo_set_register_value((uint32_t)XTAL_CTL16, XTAL_CTL16_XO_DIGPREBUF_STABLE_TIME_MASK, XTAL_CTL16_XO_DIGPREBUF_STABLE_TIME_SHIFT, 0X35);
            dcxo_set_register_value((uint32_t)XTAL_CTL16, XTAL_CTL16_XO_PREBUF_STABLE_TIME_MASK, XTAL_CTL16_XO_PREBUF_STABLE_TIME_SHIFT, 0x3b);
            dcxo_set_register_value((uint32_t)XTAL_CTL16, XTAL_CTL16_XO_RDY_STABLE_TIME_MASK, XTAL_CTL16_XO_RDY_STABLE_TIME_SHIFT, 0x3F);
#endif
#ifdef XO_2016
            dcxo_set_register_value((uint32_t)XTAL_CTL16, XTAL_CTL16_XO_DIGPREBUF_STABLE_TIME_MASK, XTAL_CTL16_XO_DIGPREBUF_STABLE_TIME_SHIFT, 0x3b);
            dcxo_set_register_value((uint32_t)XTAL_CTL16, XTAL_CTL16_XO_PREBUF_STABLE_TIME_MASK, XTAL_CTL16_XO_PREBUF_STABLE_TIME_SHIFT, 0x42);
            dcxo_set_register_value((uint32_t)XTAL_CTL16, XTAL_CTL16_XO_RDY_STABLE_TIME_MASK, XTAL_CTL16_XO_RDY_STABLE_TIME_SHIFT, 0x63);
#endif
#ifdef XO_2520
            dcxo_set_register_value((uint32_t)XTAL_CTL16, XTAL_CTL16_XO_DIGPREBUF_STABLE_TIME_MASK, XTAL_CTL16_XO_DIGPREBUF_STABLE_TIME_SHIFT, 0X35);
            dcxo_set_register_value((uint32_t)XTAL_CTL16, XTAL_CTL16_XO_PREBUF_STABLE_TIME_MASK, XTAL_CTL16_XO_PREBUF_STABLE_TIME_SHIFT, 0x3b);
            dcxo_set_register_value((uint32_t)XTAL_CTL16, XTAL_CTL16_XO_RDY_STABLE_TIME_MASK, XTAL_CTL16_XO_RDY_STABLE_TIME_SHIFT, 0x3F);
#endif
        dcxo_set_register_value((uint32_t)XTAL_CTL16, XTAL_CTL16_XO_POR_STEP_TIME_MASK, XTAL_CTL16_XO_POR_STEP_TIME_SHIFT, 0x2);
        dcxo_set_register_value((uint32_t)XTAL_CTL17, XTAL_CTL17_XO_RESET_LOW_STABLE_TIME_MASK, XTAL_CTL17_XO_RESET_LOW_STABLE_TIME_SHIFT, 0x4);
        dcxo_set_register_value((uint32_t)XTAL_CTL17, XTAL_CTL17_XO_RESET_HIGH_STABLE_TIME_MASK, XTAL_CTL17_XO_RESET_HIGH_STABLE_TIME_SHIFT, 0x3);
        dcxo_set_register_value((uint32_t)XTAL_CTL17, XTAL_CTL17_XO_BIAS_START_STABLE_TIME_MASK, XTAL_CTL17_XO_BIAS_START_STABLE_TIME_SHIFT, 0x1);
    }else{
        debug_log("Xtal is 40M");
        *XTAL_CTL1 = 0xbb27f;
        *XTAL_CTL2 = 0xbb27f;
        *XTAL_CTL3 = 0xbb27f;
        *XTAL_CTL4 = 0xbb27f;
        *XTAL_CTL5 = 0x69f7f;
        *XTAL_CTL6 = 0x58f7f;
        *XTAL_CTL7 = 0x48a3f;
        *XTAL_CTL8 = 0x40a0f;
        *XTAL_CTL9 = 0x88a1f;
        *XTAL_CTL10 = 0x98f3f;
        *XTAL_CTL11 = 0xa9f7f;
        dcxo_set_register_value((uint32_t) XTAL_CTL16, XTAL_CTL16_XO_DIGPREBUF_STABLE_TIME_MASK, XTAL_CTL16_XO_DIGPREBUF_STABLE_TIME_SHIFT, 0X3b);
        dcxo_set_register_value((uint32_t) XTAL_CTL16, XTAL_CTL16_XO_PREBUF_STABLE_TIME_MASK, XTAL_CTL16_XO_PREBUF_STABLE_TIME_SHIFT, 0x42);
        dcxo_set_register_value((uint32_t)XTAL_CTL16, XTAL_CTL16_XO_RDY_STABLE_TIME_MASK, XTAL_CTL16_XO_RDY_STABLE_TIME_SHIFT, 0x52);
#ifdef XO_3225
            dcxo_set_register_value((uint32_t)XTAL_CTL16, XTAL_CTL16_XO_DIGPREBUF_STABLE_TIME_MASK, XTAL_CTL16_XO_DIGPREBUF_STABLE_TIME_SHIFT, 0X35);
            dcxo_set_register_value((uint32_t)XTAL_CTL16, XTAL_CTL16_XO_PREBUF_STABLE_TIME_MASK, XTAL_CTL16_XO_PREBUF_STABLE_TIME_SHIFT, 0x3b);
            dcxo_set_register_value((uint32_t)XTAL_CTL16, XTAL_CTL16_XO_RDY_STABLE_TIME_MASK, XTAL_CTL16_XO_RDY_STABLE_TIME_SHIFT, 0x3F);
#endif
#ifdef XO_2016
            dcxo_set_register_value((uint32_t)XTAL_CTL16, XTAL_CTL16_XO_DIGPREBUF_STABLE_TIME_MASK, XTAL_CTL16_XO_DIGPREBUF_STABLE_TIME_SHIFT, 0x3b);
            dcxo_set_register_value((uint32_t)XTAL_CTL16, XTAL_CTL16_XO_PREBUF_STABLE_TIME_MASK, XTAL_CTL16_XO_PREBUF_STABLE_TIME_SHIFT, 0x42);
            dcxo_set_register_value((uint32_t)XTAL_CTL16, XTAL_CTL16_XO_RDY_STABLE_TIME_MASK, XTAL_CTL16_XO_RDY_STABLE_TIME_SHIFT, 0x52);
#endif
#ifdef XO_2520
            dcxo_set_register_value((uint32_t)XTAL_CTL16, XTAL_CTL16_XO_DIGPREBUF_STABLE_TIME_MASK, XTAL_CTL16_XO_DIGPREBUF_STABLE_TIME_SHIFT, 0X35);
            dcxo_set_register_value((uint32_t)XTAL_CTL16, XTAL_CTL16_XO_PREBUF_STABLE_TIME_MASK, XTAL_CTL16_XO_PREBUF_STABLE_TIME_SHIFT, 0x3b);
            dcxo_set_register_value((uint32_t)XTAL_CTL16, XTAL_CTL16_XO_RDY_STABLE_TIME_MASK, XTAL_CTL16_XO_RDY_STABLE_TIME_SHIFT, 0x3F);
#endif
        dcxo_set_register_value((uint32_t)XTAL_CTL16, XTAL_CTL16_XO_POR_STEP_TIME_MASK, XTAL_CTL16_XO_POR_STEP_TIME_SHIFT, 0x2);
        dcxo_set_register_value((uint32_t)XTAL_CTL17, XTAL_CTL17_XO_RESET_LOW_STABLE_TIME_MASK, XTAL_CTL17_XO_RESET_LOW_STABLE_TIME_SHIFT, 0x4);
        dcxo_set_register_value((uint32_t)XTAL_CTL17, XTAL_CTL17_XO_RESET_HIGH_STABLE_TIME_MASK, XTAL_CTL17_XO_RESET_HIGH_STABLE_TIME_SHIFT, 0x3);
        dcxo_set_register_value((uint32_t)XTAL_CTL17, XTAL_CTL17_XO_BIAS_START_STABLE_TIME_MASK, XTAL_CTL17_XO_BIAS_START_STABLE_TIME_SHIFT, 0x1);
    }
    dcxo_calbration_efuse();
    *XTAL_CTL0 |= 0x1;
}

void dcxo_set_xtal_lpm_status(uint8_t xosc32_enable){
    dcxo_set_register_value((uint32_t)XTAL_CTL0, 0x1, 8, xosc32_enable);
    extern_xosc32=xosc32_enable;
}

/*xosc32_enable:0 exteranl 32k; xosc32_enable=1 : 32kless*/
bool hal_dcxo_is_32kless(void)
{
    return extern_xosc32;
}

bool dcxo_get_xtal_lpm_status(){
    return dcxo_get_register_value((uint32_t)XTAL_CTL0, 0x1, 8);
}

void dcxo_set_register_value(uint32_t address, short int mask, short int shift, short int value) {
    uint32_t mask_buffer;
    mask_buffer = (~(mask << shift));
    *((volatile uint32_t *)(address)) &=mask_buffer;
    *((volatile uint32_t *)(address)) |=(value << shift);
}

uint32_t dcxo_get_register_value(uint32_t address, short int mask, short int shift) {
    uint32_t get_value, mask_buffer;
    mask_buffer = (mask << shift);
    get_value = *((volatile uint32_t *)(address));
    get_value &=mask_buffer;
    get_value = (get_value>> shift);
    return get_value;
}

void dcxo_calbration_efuse(){
    uint32_t set_efuse_value;
    uint32_t read_efuse_value;
#ifdef SW_EFSUE_SIMULATION
    dcxo_set_register_value((uint32_t)XTAL_CTL4, DCXO_EFUSE_BIT_MASK, 8, SW_EFSUE_SIMULATION_VALUE);
#else
    read_efuse_value = *((volatile uint32_t *) 0xA20A0230);
    if ( read_efuse_value & CAPID2) {
        debug_log("[CAP2]\r\n");
        set_efuse_value = dcxo_get_register_value(0xA20A0230, DCXO_EFUSE_BIT_MASK, 24);
        dcxo_set_register_value((uint32_t)XTAL_CTL4, DCXO_EFUSE_BIT_MASK, 8, set_efuse_value);
    } else if ( read_efuse_value & CAPID1) {
        debug_log("[CAP1]\r\n");
        set_efuse_value = dcxo_get_register_value(0xA20A0230, DCXO_EFUSE_BIT_MASK, 16);
        dcxo_set_register_value((uint32_t)XTAL_CTL4, DCXO_EFUSE_BIT_MASK, 8, set_efuse_value);
    } else if ( read_efuse_value & CAPID0 ) {
        debug_log("[CAP0]\r\n");
        set_efuse_value = dcxo_get_register_value(0xA20A0230, DCXO_EFUSE_BIT_MASK, 8);
        dcxo_set_register_value((uint32_t)XTAL_CTL4, DCXO_EFUSE_BIT_MASK, 8, set_efuse_value);
    }else{
        debug_log("DCXO efuse default value\r\n");
    }
#endif

}
#endif /* HAL_DCXO_MODULE_ENABLED */

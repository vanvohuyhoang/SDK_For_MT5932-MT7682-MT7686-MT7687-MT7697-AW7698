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
 
#include "hal_wdt.h"

#if defined(HAL_WDT_MODULE_ENABLED)
#include "hal_wdt_internal.h"
#include "hal_gpt.h"

void wdt_set_length(uint32_t seconds)
{
    uint32_t length_register_value = 0;
    /* trasfer seconds to register value */
    length_register_value = (seconds * 10000) / WDT_1_TICK_LENGTH;
    length_register_value <<= WDT_STANDARD_16_OFFSET;
    length_register_value |= WDT_LENGTH_KEY;

    /* write the length register */
    WDT_REGISTER->WDT_LENGTH = length_register_value;

    /*restart WDT to let the new value take effect */
    WDT_REGISTER->WDT_SW_RESTART = WDT_SW_RESTART_KEY;
    /* Wait for 6T 32k cycle */
    hal_gpt_delay_us(185);
}

void wdt_set_mode(uint32_t value)
{
    if (HAL_WDT_MODE_INTERRUPT == value) {
        WDT_REGISTER->WDT_IE = (WDT_STANDARD_1_MASK | WDT_IE_KEY);
    } else {
        WDT_REGISTER->WDT_IE = WDT_IE_KEY;
    }
}

void wdt_set_restart(void)
{
    WDT_REGISTER->WDT_SW_RESTART = WDT_SW_RESTART_KEY;
    /* Wait for 6T 32k cycle */
    hal_gpt_delay_us(185);
}

void wdt_set_sw_rst(void)
{
    WDT_REGISTER->WDT_SW_RST = WDT_SW_RST_KEY;
}

void wdt_set_enable(uint32_t enable)
{
    WDT_REGISTER->WDT_STA = 0;

    if (0 != enable) {
        WDT_REGISTER->WDT_EN = (WDT_STANDARD_1_MASK | WDT_EN_KEY);
        WDT_REGISTER->WDT_AUTO_RESTART_EN = (WDT_STANDARD_1_MASK | WDT_AUTO_RESTART_EN_KEY);
    } else {
        WDT_REGISTER->WDT_EN = WDT_EN_KEY;
        WDT_REGISTER->WDT_AUTO_RESTART_EN = WDT_AUTO_RESTART_EN_KEY;
    }
}

uint32_t wdt_get_reset_status(void)
{
    uint32_t status_register_value = 0;

    status_register_value = WDT_REGISTER->WDT_STA;

    if (0 != (status_register_value & WDT_STA_HW_WDT_MASK)) {
        return HAL_WDT_TIMEOUT_RESET;
    } else if (0 != (status_register_value & WDT_STA_SW_WDT_MASK)) {
        return HAL_WDT_SOFTWARE_RESET;
    }

    return HAL_WDT_NONE_RESET;
}

uint32_t wdt_get_enable_status(void)
{
    return (WDT_REGISTER->WDT_EN >> WDT_STANDARD_1_OFFSET);
}

uint32_t wdt_get_mode_status(void)
{
    return (WDT_REGISTER->WDT_IE >> WDT_STANDARD_1_OFFSET);
}

void wdt_clear_irq(void)
{
    uint32_t status = 0;
    status = WDT_REGISTER->WDT_INT;
    status = status;
}

void wdt_set_pmu_mask(uint32_t enable)
{
    if (0 == enable) {
        WDT_REGISTER->WDT_MASK1 = (WDT_PMU_RST_KEY << WDT_STANDARD_16_OFFSET);
    } else {
        WDT_REGISTER->WDT_MASK1 = ((WDT_STANDARD_1_MASK | WDT_PMU_RST_KEY) << WDT_STANDARD_16_OFFSET);
    }
}

#endif


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
 
#include "hal_trng_internal.h"

#ifdef HAL_TRNG_MODULE_ENABLED
#include "hal_clock.h"
#include "hal_gpt.h"

/*trng  base address register global variable*/
TRNG_REGISTER_T  *trng = TRNG;


void  trng_init(void)
{
    /* enable clock pdn*/
    hal_clock_enable(HAL_CLOCK_CG_SW_TRNG);
}

Trng_Result trng_config_timeout_limit(uint32_t timeout_value)
{
    /*time out shuld be less than TIMEOUT_VALUE*/
    uint32_t  timeout_limit = timeout_value;

    if (timeout_limit > TIMEOUT_VALUE) {
        return TRNG_GEN_FAIL;
    }

    trng->TRNG_CONF &= ~(0xFFF << (16));
    trng->TRNG_CONF |= (timeout_value << (16));

    return  TRNG_GEN_SUCCESS;
}

Trng_Result trng_enable_mode(bool H_FIRO, bool H_RO, bool H_GARO)
{

    if (true == H_FIRO) {
        trng->TRNG_CONF |= TRNG_H_FIRO_EN_MASK;
    } else {
        trng->TRNG_CONF &= ~TRNG_H_FIRO_EN_MASK;
    }
    if (true == H_RO) {
        trng->TRNG_CONF |= TRNG_H_RO_EN_MASK;
    } else {
        trng->TRNG_CONF &= ~TRNG_H_RO_EN_MASK;
    }
    if (true == H_GARO) {
        trng->TRNG_CONF |= TRNG_H_GARO_EN_MASK;
    } else {
        trng->TRNG_CONF &= ~TRNG_H_GARO_EN_MASK;
    }

    return  TRNG_GEN_SUCCESS;
}

void  trng_start(void)
{
    volatile uint32_t temp_random = 0;
    temp_random = trng->TRNG_DATA;
    temp_random += 1;
    trng->TRNG_CTRL |= TRNG_START_MASK;
}

void  trng_stop(void)
{
    volatile uint32_t temp_random = 0;
    temp_random = trng->TRNG_DATA;
    temp_random += 1;

    trng->TRNG_CTRL &= ~TRNG_START_MASK;
}

void  trng_deinit(void)
{
    /*disable clock pdn*/
    hal_clock_disable(HAL_CLOCK_CG_SW_TRNG);
}

uint32_t  trng_get_random_data(void)
{
    volatile uint32_t random_data = 0;
    volatile uint32_t ready_data = 0;
    uint32_t gpt_start_count, gpt_current_count, gpt_duration_count;

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_start_count);
    ready_data = trng->TRNG_INT_SET;
    while ((ready_data & TRNG_RDY_MASK) == 0)  {
        if ((ready_data & TRNG_TIMEOUT_MASK) != 0) {
           trng->TRNG_INT_CLR = TRNG_INT_CLR_MASK;
           return 0;
        }
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_current_count);
        hal_gpt_get_duration_count(gpt_start_count, gpt_current_count, &gpt_duration_count);
        if (TRNG_POLLING_TIMEOUT_VALUE < gpt_duration_count) {
           trng->TRNG_INT_CLR = TRNG_INT_CLR_MASK;
           return 0;
        }
        ready_data = trng->TRNG_INT_SET;
    }
    trng->TRNG_INT_CLR = TRNG_INT_CLR_MASK;
    random_data = trng->TRNG_DATA;

    return random_data;
}

#endif /*HAL_TRNG_MODULE_ENABLED*/








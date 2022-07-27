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
 
/* Includes ------------------------------------------------------------------*/
#include "hal_wdt.h"

#if defined(HAL_WDT_MODULE_ENABLED)
#include "hal_wdt_internal.h"
#include "hal_nvic.h"

/* Global variable */
static hal_wdt_callback_t s_hal_wdt_callback = NULL;

static void hal_wdt_isr(void);

/*****************************************************************************
* Function
*****************************************************************************/

hal_wdt_status_t hal_wdt_init(hal_wdt_config_t *wdt_config)
{
    /* parameter check */
    if (wdt_config == NULL || (HAL_WDT_MODE_RESET != wdt_config->mode && HAL_WDT_MODE_INTERRUPT != wdt_config->mode)) {
        return HAL_WDT_STATUS_INVALID_PARAMETER;
    }

    if (wdt_config->seconds > WDT_MAX_TIMEOUT_VALUE) {
        return HAL_WDT_STATUS_INVALID_PARAMETER;
    }

    s_hal_wdt_callback = NULL;

    /*set WDT length*/
    wdt_set_length(wdt_config->seconds);

    /* set WDT mode */
    wdt_set_mode(wdt_config->mode);

    /* User can not call hal_wdt_register_callback */
    if (HAL_WDT_MODE_INTERRUPT == wdt_config->mode) {

        /* register NVIC */
        hal_nvic_register_isr_handler(RGU_IRQn, (hal_nvic_isr_t)hal_wdt_isr);
        hal_nvic_enable_irq(RGU_IRQn);
    }

    return HAL_WDT_STATUS_OK;
}

hal_wdt_status_t hal_wdt_deinit(void)
{
    /* disable wdt */
    wdt_set_enable(false);

    s_hal_wdt_callback = NULL;

    return HAL_WDT_STATUS_OK;
}

hal_wdt_callback_t hal_wdt_register_callback(hal_wdt_callback_t wdt_callback)
{
    hal_wdt_callback_t current_wdt_callback;
    /* no need error check. allow null register */
    current_wdt_callback = s_hal_wdt_callback;

    s_hal_wdt_callback = wdt_callback;

    return current_wdt_callback;
}

hal_wdt_status_t hal_wdt_feed(uint32_t magic)
{
    if (HAL_WDT_FEED_MAGIC != magic) {
        return HAL_WDT_STATUS_INVALID_MAGIC;
    }

    /* write wdt restart register */
    wdt_set_restart();

    return HAL_WDT_STATUS_OK;
}

hal_wdt_status_t hal_wdt_software_reset(void)
{
    wdt_set_sw_rst();
    return HAL_WDT_STATUS_OK;
}

hal_wdt_status_t hal_wdt_enable(uint32_t magic)
{
    if (HAL_WDT_ENABLE_MAGIC != magic) {
        return HAL_WDT_STATUS_INVALID_MAGIC;
    }

    wdt_set_enable(true);
    /* restart wdt */
    wdt_set_restart();

    return HAL_WDT_STATUS_OK;
}

hal_wdt_status_t hal_wdt_disable(uint32_t magic)
{
    if (HAL_WDT_DISABLE_MAGIC != magic) {
        return HAL_WDT_STATUS_INVALID_MAGIC;
    }

    wdt_set_enable(false);

    return HAL_WDT_STATUS_OK;
}

hal_wdt_reset_status_t hal_wdt_get_reset_status(void)
{
    return ((hal_wdt_reset_status_t)wdt_get_reset_status());
}

bool hal_wdt_get_enable_status(void)
{
    return (wdt_get_enable_status());
}

hal_wdt_mode_t hal_wdt_get_mode(void)
{
    return ((hal_wdt_mode_t)wdt_get_mode_status());
}

static void hal_wdt_isr(void)
{
    hal_wdt_reset_status_t wdt_reset_status;

    wdt_reset_status = (hal_wdt_reset_status_t)wdt_get_reset_status();
    if (NULL != s_hal_wdt_callback) {
        s_hal_wdt_callback(wdt_reset_status);
    }
    /* clear WDT IRQ */
    wdt_clear_irq();
}

#endif


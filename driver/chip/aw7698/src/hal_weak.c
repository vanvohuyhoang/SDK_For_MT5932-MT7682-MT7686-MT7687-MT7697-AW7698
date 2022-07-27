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
 
#include "hal.h"
#include "hal_log.h"

/* This file for HAL core lib release, void build error when disable some module*/
#if defined(__GNUC__) || defined(__ICCARM__) || defined(__CC_ARM)

hal_pinmux_status_t default_bt_driver_pinmux_set_function(hal_gpio_pin_t gpio_pin, uint8_t function_index)
{
    log_hal_error("This pin is not supported!!!");
    return HAL_GPIO_STATUS_OK;
}
#pragma weak bt_driver_pinmux_set_function = default_bt_driver_pinmux_set_function


hal_gpio_status_t default_bt_driver_hal_gpio_set_output(hal_gpio_pin_t gpio_pin, hal_gpio_data_t gpio_data)
{
    log_hal_error("This pin is not supported!!!");
    return HAL_GPIO_STATUS_OK;
}
#pragma weak bt_driver_hal_gpio_set_output = default_bt_driver_hal_gpio_set_output


hal_gpio_status_t default_bt_driver_hal_gpio_set_direction(hal_gpio_pin_t gpio_pin, hal_gpio_direction_t gpio_direction)
{
    log_hal_error("This pin is not supported!!!");
    return HAL_GPIO_STATUS_OK;
}
#pragma weak bt_driver_hal_gpio_set_direction = default_bt_driver_hal_gpio_set_direction


hal_gpio_status_t default_bt_driver_hal_gpio_pull_up(hal_gpio_pin_t gpio_pin)
{
    log_hal_error("This pin is not supported!!!");
    return HAL_GPIO_STATUS_OK;
}
#pragma weak bt_driver_hal_gpio_pull_up = default_bt_driver_hal_gpio_pull_up


hal_gpio_status_t default_bt_driver_hal_gpio_pull_down(hal_gpio_pin_t gpio_pin)
{
    log_hal_error("This pin is not supported!!!");
    return HAL_GPIO_STATUS_OK;
}
#pragma weak bt_driver_hal_gpio_pull_down = default_bt_driver_hal_gpio_pull_down


hal_gpio_status_t default_bt_driver_hal_gpio_disable_pull(hal_gpio_pin_t gpio_pin)
{
    log_hal_error("This pin is not supported!!!");
    return HAL_GPIO_STATUS_OK;
}
#pragma weak bt_driver_hal_gpio_disable_pull = default_bt_driver_hal_gpio_disable_pull



//pwm_init
hal_pwm_status_t default_bt_driver_hal_pwm_init(hal_pwm_channel_t pwm_channel, hal_pwm_source_clock_t source_clock)
{
    log_hal_error("This pwm channel is not supported!!!");
    return HAL_PWM_STATUS_OK;
}
#pragma weak bt_driver_hal_pwm_init = default_bt_driver_hal_pwm_init
//pwm_deinit
hal_pwm_status_t default_bt_driver_hal_pwm_deinit(hal_pwm_channel_t pwm_channel)
{
    log_hal_error("This pwm channel is not supported!!!");
    return HAL_PWM_STATUS_OK;
}
#pragma weak bt_driver_hal_pwm_deinit = default_bt_driver_hal_pwm_deinit

//pwm_set_frequency
hal_pwm_status_t default_bt_driver_hal_pwm_set_frequency(hal_pwm_channel_t pwm_channel, uint32_t frequency, uint32_t *total_count)
{
    log_hal_error("This pwm channel is not supported!!!");
    return HAL_PWM_STATUS_OK;
}
#pragma weak bt_driver_hal_pwm_set_frequency = default_bt_driver_hal_pwm_set_frequency

//pwm_set_duty_cycle
hal_pwm_status_t default_bt_driver_hal_pwm_set_duty_cycle(hal_pwm_channel_t pwm_channel, uint32_t duty_cycle)
{
    log_hal_error("This pwm channel is not supported!!!");
    return HAL_PWM_STATUS_OK;
}
#pragma weak bt_driver_hal_pwm_set_duty_cycle = default_bt_driver_hal_pwm_set_duty_cycle

//pwm_start
hal_pwm_status_t default_bt_driver_hal_pwm_start(hal_pwm_channel_t pwm_channel)
{
    log_hal_error("This pwm channel is not supported!!!");
    return HAL_PWM_STATUS_OK;
}
#pragma weak bt_driver_hal_pwm_start = default_bt_driver_hal_pwm_start

//pwm_stop
hal_pwm_status_t default_bt_driver_hal_pwm_stop(hal_pwm_channel_t pwm_channel)
{
    log_hal_error("This pwm channel is not supported!!!");
    return HAL_PWM_STATUS_OK;
}
#pragma weak bt_driver_hal_pwm_stop = default_bt_driver_hal_pwm_stop


#else
#error "Unsupported Platform"
#endif


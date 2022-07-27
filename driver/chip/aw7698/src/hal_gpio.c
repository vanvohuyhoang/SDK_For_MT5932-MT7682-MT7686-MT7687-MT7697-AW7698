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
 
#include "hal_gpio.h"


#ifdef HAL_GPIO_MODULE_ENABLED
#include "hal_gpio_internal.h"
#include "hal_log.h"

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t save_and_set_interrupt_mask(void);
extern void restore_interrupt_mask(uint32_t mask);

GPIO_BASE_REGISTER_T *gpio_base = (GPIO_BASE_REGISTER_T *)(GPIO_BASE);
GPIO_CFG0_REGISTER_T *gpio_cfg0 = (GPIO_CFG0_REGISTER_T *)(IO_CFG_0_BASE);
GPIO_CFG1_REGISTER_T *gpio_cfg1 = (GPIO_CFG1_REGISTER_T *)(IO_CFG_1_BASE);
TOP_MISC_CFG_T *clk_out_contorl = TOP_MISC_CFG;



/* check if the pin has only one pull-up resister and one pull-down resister*/
/* pin with pullsel means the pin has only one pull-up resister and one pull-down resister*/
bool is_pin_with_pullsel(hal_gpio_pin_t gpio_pin)
{
    /* pin number which has only one pull-up resister and one pull-down resister, it is different on different chips */
    if (gpio_pin <= (hal_gpio_pin_t)10) {
        return true;
    }

    if ((gpio_pin >= HAL_GPIO_17) && (gpio_pin <= (hal_gpio_pin_t)20)) {
        return true;
    }

    if ((gpio_pin == (hal_gpio_pin_t)21) || (gpio_pin == (hal_gpio_pin_t)22)) {
        return true;
    }

    return false;
}

bool is_pin_in_cfg0(hal_gpio_pin_t gpio_pin)
{
    if (gpio_pin <= (hal_gpio_pin_t)10) {
        return true;
    }

    if ((gpio_pin == (hal_gpio_pin_t)21) || (gpio_pin == (hal_gpio_pin_t)22)) {
        return true;
    }

    return false;
}

bool is_pin_in_cfg1(hal_gpio_pin_t gpio_pin)
{
    if ((gpio_pin >= HAL_GPIO_17) && (HAL_GPIO_0 <= 20)) {
        return true;
    }

    return false;
}




hal_gpio_status_t hal_gpio_init(hal_gpio_pin_t gpio_pin)
{
    return HAL_GPIO_STATUS_OK;
}


hal_gpio_status_t hal_gpio_deinit(hal_gpio_pin_t gpio_pin)
{
    return HAL_GPIO_STATUS_OK;
}



hal_gpio_status_t hal_gpio_set_direction(hal_gpio_pin_t gpio_pin, hal_gpio_direction_t gpio_direction)
{
    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    if (gpio_pin >= EX_GPIO_0) {
        return bt_driver_hal_gpio_set_direction((gpio_pin - EX_GPIO_0), gpio_direction);
    }

    if (gpio_direction == HAL_GPIO_DIRECTION_INPUT) {
        gpio_base->GPIO_DIR.CLR = (GPIO_REG_ONE_BIT_SET_CLR << gpio_pin);
    } else {
        gpio_base->GPIO_DIR.SET = (GPIO_REG_ONE_BIT_SET_CLR << gpio_pin);
    }
    return HAL_GPIO_STATUS_OK;

}

hal_gpio_status_t hal_gpio_get_direction(hal_gpio_pin_t gpio_pin, hal_gpio_direction_t *gpio_direction)
{
    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    if (NULL == gpio_direction) {
        return HAL_GPIO_STATUS_INVALID_PARAMETER;
    }

    if (gpio_base->GPIO_DIR.RW & (GPIO_REG_ONE_BIT_SET_CLR << gpio_pin)) {
        *gpio_direction = HAL_GPIO_DIRECTION_OUTPUT;
    } else {
        *gpio_direction = HAL_GPIO_DIRECTION_INPUT;
    }

    return HAL_GPIO_STATUS_OK;

}



hal_pinmux_status_t hal_pinmux_set_function(hal_gpio_pin_t gpio_pin, uint8_t function_index)
{
    uint32_t no;
    uint32_t remainder;
    uint32_t irq_status;
    uint32_t temp;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_PINMUX_STATUS_ERROR_PORT;
    }

    /* check whether the function index is right as one function is corresponding to 4 bits of oen pin */
    if (function_index >= GPIO_MODE_MAX_NUMBER) {
        return HAL_PINMUX_STATUS_INVALID_FUNCTION;
    }

    if (gpio_pin >= EX_GPIO_0) {
        return bt_driver_pinmux_set_function((gpio_pin - EX_GPIO_0), function_index);
    }

    /* get the register number corresponding to the pin as one register can control 8 pins*/
    no = gpio_pin / GPIO_MODE_REG_CTRL_PIN_NUM;

    /* get the bit offset within the register as one register can control 8 pins*/
    remainder = gpio_pin % GPIO_MODE_REG_CTRL_PIN_NUM;

    /* protect the configuration to prevent possible interrupt */
    irq_status = save_and_set_interrupt_mask();
    temp = gpio_base->GPIO_MODE.RW[no];
    temp &= ~(GPIO_REG_FOUR_BIT_SET_CLR << (remainder * GPIO_MODE_FUNCTION_CTRL_BITS));
    temp |= (function_index << (remainder * GPIO_MODE_FUNCTION_CTRL_BITS));
    gpio_base->GPIO_MODE.RW[no] = temp;
    restore_interrupt_mask(irq_status);

    return HAL_PINMUX_STATUS_OK;

}



hal_gpio_status_t hal_gpio_get_input(hal_gpio_pin_t gpio_pin, hal_gpio_data_t *gpio_data)
{
    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    if (NULL == gpio_data) {
        return HAL_GPIO_STATUS_INVALID_PARAMETER;
    }

    if (gpio_base->GPIO_DIN.R & (GPIO_REG_ONE_BIT_SET_CLR << gpio_pin)) {
        *gpio_data = HAL_GPIO_DATA_HIGH;
    } else {
        *gpio_data = HAL_GPIO_DATA_LOW;
    }

    return HAL_GPIO_STATUS_OK;

}



hal_gpio_status_t hal_gpio_set_output(hal_gpio_pin_t gpio_pin, hal_gpio_data_t gpio_data)
{

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    if (gpio_pin >= EX_GPIO_0) {
        return bt_driver_hal_gpio_set_output((gpio_pin - EX_GPIO_0), gpio_data);
    }

    if (gpio_data) {
        gpio_base->GPIO_DOUT.SET = (GPIO_REG_ONE_BIT_SET_CLR << gpio_pin);
    } else {
        gpio_base->GPIO_DOUT.CLR = (GPIO_REG_ONE_BIT_SET_CLR << gpio_pin);
    }

    return HAL_GPIO_STATUS_OK;
}



hal_gpio_status_t hal_gpio_get_output(hal_gpio_pin_t gpio_pin, hal_gpio_data_t *gpio_data)
{
    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    if (NULL == gpio_data) {
        return HAL_GPIO_STATUS_INVALID_PARAMETER;
    }

    if (gpio_base->GPIO_DOUT.RW & (GPIO_REG_ONE_BIT_SET_CLR << gpio_pin)) {
        *gpio_data = HAL_GPIO_DATA_HIGH;
    } else {
        *gpio_data = HAL_GPIO_DATA_LOW;
    }

    return HAL_GPIO_STATUS_OK;

}



hal_gpio_status_t hal_gpio_toggle_pin(hal_gpio_pin_t gpio_pin)
{
    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    if (gpio_base->GPIO_DOUT.RW & (GPIO_REG_ONE_BIT_SET_CLR << gpio_pin)) {
        gpio_base->GPIO_DOUT.CLR = (GPIO_REG_ONE_BIT_SET_CLR << gpio_pin);
    } else {
        gpio_base->GPIO_DOUT.SET = (GPIO_REG_ONE_BIT_SET_CLR << gpio_pin);
    }

    return HAL_GPIO_STATUS_OK;
}

hal_gpio_status_t hal_gpio_pull_up(hal_gpio_pin_t gpio_pin)
{
    uint32_t irq_status;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    if (gpio_pin >= EX_GPIO_0) {
        return bt_driver_hal_gpio_pull_up((gpio_pin - EX_GPIO_0));
    }

    irq_status = save_and_set_interrupt_mask();

    if (is_pin_in_cfg0(gpio_pin) == true) {
        if (gpio_pin <= (hal_gpio_pin_t)10) {
            gpio_cfg0->GPIO_PD.CLR = (GPIO_REG_ONE_BIT_SET_CLR << gpio_pin);
            gpio_cfg0->GPIO_PU.SET = (GPIO_REG_ONE_BIT_SET_CLR << gpio_pin);
        }

        else if (gpio_pin == (hal_gpio_pin_t)21) {
            gpio_cfg0->GPIO_PD.CLR = (GPIO_REG_ONE_BIT_SET_CLR << 11);
            gpio_cfg0->GPIO_PU.SET = (GPIO_REG_ONE_BIT_SET_CLR << 11);

        } else if (gpio_pin == (hal_gpio_pin_t)22) {
            gpio_cfg0->GPIO_PD.CLR = (GPIO_REG_ONE_BIT_SET_CLR << 12);
            gpio_cfg0->GPIO_PU.SET = (GPIO_REG_ONE_BIT_SET_CLR << 12);
        }
    } else if (is_pin_in_cfg1(gpio_pin)  == true) {
        if ((gpio_pin <= (hal_gpio_pin_t)20) && (gpio_pin >= HAL_GPIO_17)) {
            gpio_cfg1->GPIO_PD.CLR = (GPIO_REG_ONE_BIT_SET_CLR << (gpio_pin - HAL_GPIO_17));
            gpio_cfg1->GPIO_PU.SET = (GPIO_REG_ONE_BIT_SET_CLR << (gpio_pin - HAL_GPIO_17));
        }
    } else if (!is_pin_with_pullsel(gpio_pin)) {
#ifdef HAL_GPIO_FEATURE_PUPD
        hal_gpio_set_pupd_register(gpio_pin, 0, 1, 0); //pull up 47k
#endif
    } else {
        return HAL_GPIO_STATUS_INVALID_PARAMETER;
    }

    restore_interrupt_mask(irq_status);

    return HAL_GPIO_STATUS_OK;

}



hal_gpio_status_t hal_gpio_pull_down(hal_gpio_pin_t gpio_pin)
{
    uint32_t irq_status;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    if (gpio_pin >= EX_GPIO_0) {
        return bt_driver_hal_gpio_pull_down((gpio_pin - EX_GPIO_0));
    }

    irq_status = save_and_set_interrupt_mask();

    if (is_pin_in_cfg0(gpio_pin) == true) {
        if (gpio_pin <= (hal_gpio_pin_t)10) {
            gpio_cfg0->GPIO_PU.CLR = (GPIO_REG_ONE_BIT_SET_CLR << gpio_pin);
            gpio_cfg0->GPIO_PD.SET = (GPIO_REG_ONE_BIT_SET_CLR << gpio_pin);
        } else if (gpio_pin == (hal_gpio_pin_t)21) {
            gpio_cfg0->GPIO_PU.CLR = (GPIO_REG_ONE_BIT_SET_CLR << 11);
            gpio_cfg0->GPIO_PD.SET = (GPIO_REG_ONE_BIT_SET_CLR << 11);

        } else if (gpio_pin == (hal_gpio_pin_t)22) {
            gpio_cfg0->GPIO_PU.CLR = (GPIO_REG_ONE_BIT_SET_CLR << 12);
            gpio_cfg0->GPIO_PD.SET = (GPIO_REG_ONE_BIT_SET_CLR << 12);
        }
    } else if (is_pin_in_cfg1(gpio_pin)  == true) {
        if ((gpio_pin <= (hal_gpio_pin_t)20) && (gpio_pin >= HAL_GPIO_17)) {
            gpio_cfg1->GPIO_PU.CLR = (GPIO_REG_ONE_BIT_SET_CLR << (gpio_pin - HAL_GPIO_17));
            gpio_cfg1->GPIO_PD.SET = (GPIO_REG_ONE_BIT_SET_CLR << (gpio_pin - HAL_GPIO_17));
        }
    } else if (!is_pin_with_pullsel(gpio_pin)) {
#ifdef HAL_GPIO_FEATURE_PUPD
        hal_gpio_set_pupd_register(gpio_pin, 1, 1, 0); //pull down 47k
#endif
    } else {
        return HAL_GPIO_STATUS_INVALID_PARAMETER;
    }

    restore_interrupt_mask(irq_status);

    return HAL_GPIO_STATUS_OK;

}



hal_gpio_status_t hal_gpio_disable_pull(hal_gpio_pin_t gpio_pin)
{
    uint32_t irq_status;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    if (gpio_pin >= EX_GPIO_0) {
        return bt_driver_hal_gpio_disable_pull((gpio_pin - EX_GPIO_0));
    }

    irq_status = save_and_set_interrupt_mask();

    if (is_pin_in_cfg0(gpio_pin) == true) {
        if (gpio_pin <= (hal_gpio_pin_t)10) {
            gpio_cfg0->GPIO_PU.CLR = (GPIO_REG_ONE_BIT_SET_CLR << gpio_pin);
            gpio_cfg0->GPIO_PD.CLR = (GPIO_REG_ONE_BIT_SET_CLR << gpio_pin);
        } else if (gpio_pin == (hal_gpio_pin_t)21) {
            gpio_cfg0->GPIO_PU.CLR = (GPIO_REG_ONE_BIT_SET_CLR << 11);
            gpio_cfg0->GPIO_PD.CLR = (GPIO_REG_ONE_BIT_SET_CLR << 11);

        } else if (gpio_pin == (hal_gpio_pin_t)22) {
            gpio_cfg0->GPIO_PU.CLR = (GPIO_REG_ONE_BIT_SET_CLR << 12);
            gpio_cfg0->GPIO_PD.CLR = (GPIO_REG_ONE_BIT_SET_CLR << 12);
        }
    } else if (is_pin_in_cfg1(gpio_pin)  == true) {
        if ((gpio_pin <= (hal_gpio_pin_t)20) && (gpio_pin >= HAL_GPIO_17)) {
            gpio_cfg1->GPIO_PU.CLR = (GPIO_REG_ONE_BIT_SET_CLR << (gpio_pin - HAL_GPIO_17));
            gpio_cfg1->GPIO_PD.CLR = (GPIO_REG_ONE_BIT_SET_CLR << (gpio_pin - HAL_GPIO_17));
        }
    } else if (!is_pin_with_pullsel(gpio_pin)) {
#ifdef HAL_GPIO_FEATURE_PUPD
        hal_gpio_set_pupd_register(gpio_pin, 0, 0, 0);
#endif
    } else {
        return HAL_GPIO_STATUS_INVALID_PARAMETER;
    }

    restore_interrupt_mask(irq_status);

    return HAL_GPIO_STATUS_OK;
}



#ifdef HAL_GPIO_FEATURE_CLOCKOUT
hal_gpio_status_t hal_gpio_set_clockout(hal_gpio_clock_t gpio_clock_num, hal_gpio_clock_mode_t clock_mode)
{

    if (gpio_clock_num >= HAL_GPIO_CLOCK_MAX) {
        return HAL_GPIO_STATUS_INVALID_PARAMETER;
    }

    if (gpio_clock_num < HAL_GPIO_CLOCK_4) {
        clk_out_contorl->CLKA_CTRL_UNION.CLKA_CELLS.CLK_MODE[gpio_clock_num] = clock_mode;
    } else {
        clk_out_contorl->CLKB_CTRL_UNION.CLKB_CELLS.CLK_MODE[gpio_clock_num - 4] = clock_mode;
    }

    return HAL_GPIO_STATUS_OK;
}

#endif

#ifdef HAL_GPIO_FEATURE_SET_SCHMITT
hal_gpio_status_t hal_gpio_set_schmitt(hal_gpio_pin_t gpio_pin)
{
    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    if (gpio_pin <= (hal_gpio_pin_t)10) {
        gpio_cfg0->GPIO_SMT.SET = (GPIO_REG_ONE_BIT_SET_CLR << gpio_pin);
    } else if (gpio_pin == (hal_gpio_pin_t)21) {
        gpio_cfg0->GPIO_SMT.SET = (GPIO_REG_ONE_BIT_SET_CLR << 11);

    } else if (gpio_pin == (hal_gpio_pin_t)22) {
        gpio_cfg0->GPIO_SMT.SET = (GPIO_REG_ONE_BIT_SET_CLR << 12);
    }


    if ((gpio_pin <= (hal_gpio_pin_t)20) && (gpio_pin >= HAL_GPIO_11)) {
        gpio_cfg1->GPIO_SMT.SET = (GPIO_REG_ONE_BIT_SET_CLR << (gpio_pin - HAL_GPIO_11));

    }

    return HAL_GPIO_STATUS_OK;
}


hal_gpio_status_t hal_gpio_clear_schmitt(hal_gpio_pin_t gpio_pin)
{
    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    if (gpio_pin <= (hal_gpio_pin_t)10) {
        gpio_cfg0->GPIO_SMT.CLR = (GPIO_REG_ONE_BIT_SET_CLR << gpio_pin);
    } else if (gpio_pin == (hal_gpio_pin_t)21) {
        gpio_cfg0->GPIO_SMT.CLR = (GPIO_REG_ONE_BIT_SET_CLR << 11);

    } else if (gpio_pin == (hal_gpio_pin_t)22) {
        gpio_cfg0->GPIO_SMT.CLR = (GPIO_REG_ONE_BIT_SET_CLR << 12);
    }

    if ((gpio_pin <= (hal_gpio_pin_t)20) && (gpio_pin >= HAL_GPIO_11)) {
        gpio_cfg1->GPIO_SMT.CLR = (GPIO_REG_ONE_BIT_SET_CLR << (gpio_pin - HAL_GPIO_11));
    }

    return HAL_GPIO_STATUS_OK;
}
#endif


#ifdef HAL_GPIO_FEATURE_PUPD
hal_gpio_status_t hal_gpio_set_pupd_register(hal_gpio_pin_t gpio_pin, uint8_t gpio_pupd, uint8_t gpio_r0, uint8_t gpio_r1)
{
    uint32_t shift;
    uint32_t irq_status;

    if ((gpio_pin >= HAL_GPIO_MAX) || (is_pin_with_pullsel(gpio_pin))) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    irq_status = save_and_set_interrupt_mask();

    shift = (uint32_t)(gpio_pin - HAL_GPIO_11);
    if (gpio_pupd) {
        gpio_cfg1->GPIO_PUPD.SET = (GPIO_REG_ONE_BIT_SET_CLR << shift);
    } else {
        gpio_cfg1->GPIO_PUPD.CLR = (GPIO_REG_ONE_BIT_SET_CLR << shift);
    }

    if (gpio_r0) {
        gpio_cfg1->GPIO_R0.SET = (GPIO_REG_ONE_BIT_SET_CLR << shift);
    } else {
        gpio_cfg1->GPIO_R0.CLR = (GPIO_REG_ONE_BIT_SET_CLR << shift);
    }

    if (gpio_r1) {
        gpio_cfg1->GPIO_R1.SET = (GPIO_REG_ONE_BIT_SET_CLR << shift);
    } else {
        gpio_cfg1->GPIO_R1.CLR = (GPIO_REG_ONE_BIT_SET_CLR << shift);
    }
    restore_interrupt_mask(irq_status);

    return HAL_GPIO_STATUS_OK;
}
#endif


#ifdef HAL_GPIO_FEATURE_HIGH_Z
hal_gpio_status_t hal_gpio_set_high_impedance(hal_gpio_pin_t gpio_pin)
{

    hal_pinmux_status_t ret1;
    hal_gpio_status_t   ret2;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    /* set GPIO mode of pin */
    ret1 = hal_pinmux_set_function(gpio_pin, 0);
    if (ret1 != HAL_PINMUX_STATUS_OK) {
        return HAL_GPIO_STATUS_ERROR;
    }

    /* set input direction of pin */
    ret2 = hal_gpio_set_direction(gpio_pin, HAL_GPIO_DIRECTION_INPUT);
    if (ret2 != HAL_GPIO_STATUS_OK) {
        return ret2;
    }

    /* disable input buffer enable function of pin */
    if (gpio_pin <= (hal_gpio_pin_t)10) {
        gpio_cfg0->GPIO_IES.CLR = (GPIO_REG_ONE_BIT_SET_CLR << gpio_pin);
    } else if (gpio_pin == (hal_gpio_pin_t)21) {
        gpio_cfg0->GPIO_IES.CLR = (GPIO_REG_ONE_BIT_SET_CLR << 11);

    } else if (gpio_pin == (hal_gpio_pin_t)22) {
        gpio_cfg0->GPIO_IES.CLR = (GPIO_REG_ONE_BIT_SET_CLR << 12);
    }

    if ((gpio_pin <= (hal_gpio_pin_t)20) && (gpio_pin >= HAL_GPIO_11)) {
        gpio_cfg1->GPIO_IES.CLR = (GPIO_REG_ONE_BIT_SET_CLR << (gpio_pin - HAL_GPIO_11));
    }

    /* disable pull function of pin */
    ret2 = hal_gpio_disable_pull(gpio_pin);
    if (ret2 != HAL_GPIO_STATUS_OK) {
        return ret2;
    }


    return HAL_GPIO_STATUS_OK;
}


hal_gpio_status_t hal_gpio_clear_high_impedance(hal_gpio_pin_t gpio_pin)
{
    hal_pinmux_status_t ret1;
    hal_gpio_status_t   ret2;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    /* set GPIO mode of pin. */
    ret1 = hal_pinmux_set_function(gpio_pin, 0);
    if (ret1 != HAL_PINMUX_STATUS_OK) {
        return HAL_GPIO_STATUS_ERROR;
    }

    /* set input direction of pin. */
    ret2 = hal_gpio_set_direction(gpio_pin, HAL_GPIO_DIRECTION_INPUT);
    if (ret2 != HAL_GPIO_STATUS_OK) {
        return ret2;
    }

    if (gpio_pin <= (hal_gpio_pin_t)10) {
        gpio_cfg0->GPIO_IES.SET = (GPIO_REG_ONE_BIT_SET_CLR << gpio_pin);
    } else if (gpio_pin == (hal_gpio_pin_t)21) {
        gpio_cfg0->GPIO_IES.SET = (GPIO_REG_ONE_BIT_SET_CLR << 11);

    } else if (gpio_pin == (hal_gpio_pin_t)22) {
        gpio_cfg0->GPIO_IES.SET = (GPIO_REG_ONE_BIT_SET_CLR << 12);
    }

    if ((gpio_pin <= (hal_gpio_pin_t)20) && (gpio_pin >= HAL_GPIO_11)) {
        gpio_cfg1->GPIO_IES.SET = (GPIO_REG_ONE_BIT_SET_CLR << (gpio_pin - HAL_GPIO_11));
    }

    /* enable pull down of pin. */
    if (is_pin_with_pullsel(gpio_pin)) {
        ret2 = hal_gpio_pull_down(gpio_pin);
        if (ret2 != HAL_GPIO_STATUS_OK) {
            return ret2;
        }
    }
#ifdef HAL_GPIO_FEATURE_PUPD
    else {
        ret2 = hal_gpio_set_pupd_register(gpio_pin, 1, 1, 1);
        if (ret2 != HAL_GPIO_STATUS_OK) {
            return ret2;
        }

    }
#endif

    return HAL_GPIO_STATUS_OK;
}
#endif

#ifdef HAL_GPIO_FEATURE_SET_DRIVING
hal_gpio_status_t hal_gpio_set_driving_current(hal_gpio_pin_t gpio_pin, hal_gpio_driving_current_t driving)
{
    uint32_t irq_status;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    irq_status = save_and_set_interrupt_mask();

    if (gpio_pin <= (hal_gpio_pin_t)10) {
        gpio_cfg0->GPIO_DRV.CLR = (GPIO_REG_TWO_BIT_SET_CLR << (gpio_pin * 2));
        gpio_cfg0->GPIO_DRV.SET = (driving << (gpio_pin * 2));
    } else if (gpio_pin == (hal_gpio_pin_t)21) {
        gpio_cfg0->GPIO_DRV.CLR = (GPIO_REG_TWO_BIT_SET_CLR << (11 * 2));
        gpio_cfg0->GPIO_DRV.SET = (driving << (11 * 2));

    } else if (gpio_pin == (hal_gpio_pin_t)22) {
        gpio_cfg0->GPIO_DRV.CLR = (GPIO_REG_TWO_BIT_SET_CLR << (12 * 2));
        gpio_cfg0->GPIO_DRV.SET = (driving << (12 * 2));
    }


    if ((gpio_pin <= (hal_gpio_pin_t)20) && (gpio_pin >= HAL_GPIO_11)) {
        gpio_cfg1->GPIO_DRV.CLR = (GPIO_REG_TWO_BIT_SET_CLR << ((gpio_pin - HAL_GPIO_11) * 2));
        gpio_cfg1->GPIO_DRV.SET = (driving << ((gpio_pin - HAL_GPIO_11) * 2));
    }

    restore_interrupt_mask(irq_status);

    return HAL_GPIO_STATUS_OK;
}


hal_gpio_status_t hal_gpio_get_driving_current(hal_gpio_pin_t gpio_pin, hal_gpio_driving_current_t *driving)
{
    uint32_t irq_status;
    uint32_t temp;
    uint32_t shift;

    temp = 0;

    if (gpio_pin >= HAL_GPIO_MAX) {
        return HAL_GPIO_STATUS_ERROR_PIN;
    }

    irq_status = save_and_set_interrupt_mask();

    if (gpio_pin <= (hal_gpio_pin_t)10) {
        shift = (gpio_pin * 2);
        temp = (gpio_cfg0->GPIO_DRV.RW >> shift) & GPIO_REG_TWO_BIT_SET_CLR;
    } else if (gpio_pin == (hal_gpio_pin_t)21) {
        shift = (11 * 2);
        temp = (gpio_cfg0->GPIO_DRV.RW >> shift) & GPIO_REG_TWO_BIT_SET_CLR;
    } else if (gpio_pin == (hal_gpio_pin_t)22) {
        shift = (12 * 2);
        temp = (gpio_cfg0->GPIO_DRV.RW >> shift) & GPIO_REG_TWO_BIT_SET_CLR;
    } else if ((gpio_pin <= (hal_gpio_pin_t)20) && (gpio_pin >= HAL_GPIO_11)) {
        shift = ((gpio_pin - HAL_GPIO_11) * 2);
        temp = (gpio_cfg1->GPIO_DRV.RW >> shift) & GPIO_REG_TWO_BIT_SET_CLR;
    }

    *driving = (hal_gpio_driving_current_t)(temp);

    restore_interrupt_mask(irq_status);

    return HAL_GPIO_STATUS_OK;

}
#endif

void gpio_get_state(hal_gpio_pin_t gpio_pin, gpio_state_t *gpio_state)
{

    uint32_t mode;
    uint32_t dir;
    uint32_t din;
    uint32_t dout;
    uint32_t pu;
    uint32_t pd;
    uint32_t pupd;
    uint32_t r0;
    uint32_t r1;

    gpio_pull_type_t pull_type;
    uint32_t temp;
    uint32_t reg_index;
    uint32_t bit_index;
    hal_gpio_driving_current_t driving_value;

    //const char *direct[2] = {"input", "output"};
    //const char *pull_state[8] = {"disable_pull", "PU_75k", "PD_75k", "PU_47K","PD_47K", "PU_23.5K", "PD_23.5K", "PUPD_Error"};

    reg_index = gpio_pin / 8;
    bit_index = (gpio_pin % 8) * 4;
    mode = (gpio_base->GPIO_MODE.RW[reg_index] >> (bit_index) & 0xf);

    reg_index = gpio_pin / 32;
    bit_index = gpio_pin % 32;
    dir  = (gpio_base->GPIO_DIR.RW >> (bit_index) & 0x1);
    din  = (gpio_base->GPIO_DIN.R >> (bit_index) & 0x1);
    dout = (gpio_base->GPIO_DOUT.RW >> (bit_index) & 0x1);

    pu = 0xf;
    pd = 0xf;
    pupd = 0xf;
    r0   = 0xf;
    r1   = 0xf;

    if (gpio_pin <= (hal_gpio_pin_t)10) {
        pu = (gpio_cfg0->GPIO_PU.RW >> gpio_pin) & 0x01;
        pd = (gpio_cfg0->GPIO_PD.RW >> gpio_pin) & 0x01;
    } else if ((gpio_pin == (hal_gpio_pin_t)21) || (gpio_pin == (hal_gpio_pin_t)22)) {
        pu = (gpio_cfg0->GPIO_PU.RW >> (gpio_pin - 10)) & 0x01;
        pd = (gpio_cfg0->GPIO_PD.RW >> (gpio_pin - 10)) & 0x01;
    } else if ((gpio_pin <= (hal_gpio_pin_t)20) && (gpio_pin >= HAL_GPIO_17)) {
        pu = (gpio_cfg1->GPIO_PU.RW >> (gpio_pin - 17)) & 0x01;
        pd = (gpio_cfg1->GPIO_PD.RW >> (gpio_pin - 17)) & 0x01;
    } else if ((gpio_pin <= (hal_gpio_pin_t)16) && (gpio_pin >= HAL_GPIO_11)) {
        pupd = (gpio_cfg1->GPIO_PUPD.RW >> (gpio_pin - 11)) & 0x1;
        r0   = (gpio_cfg1->GPIO_R0.RW >> (gpio_pin - 11)) & 0x1;
        r1   = (gpio_cfg1->GPIO_R1.RW >> (gpio_pin - 11)) & 0x1;
    } else {
        log_hal_info("get_gpio_state: input error pin number\r\n");
    }


    pull_type = (gpio_pull_type_t)0;

    temp = (pu << 4) + pd;

    //log_hal_info("pu=%d pd=%d, temp=%.3x\r\n",pu,pd,temp);

    if (temp == 0x00) {
        pull_type = GPIO_NO_PULL;
    } else if (temp == 0x10) {
        pull_type = GPIO_PU_75K;
    } else if (temp == 0x01) {
        pull_type = GPIO_PD_75K;
    } else if (temp != 0xff) {
        pull_type = GPIO_PUPD_ERR;
        log_hal_info("GPIO%d,error pu = %x, pd= %x\r\n", gpio_pin, pu, pd);
    }

    temp = (pupd << 8) + (r0 << 4) + r1;
    //printf("pupd=%d r0=%d, r1=%d, temp=%.3x\r\n",pupd,r0,r1,temp);

    if ((temp == 0x001) || (temp == 0x010)) {
        pull_type = GPIO_PU_47K;
    } else if ((temp == 0x101) || (temp == 0x110)) {
        pull_type = GPIO_PD_47K;
    } else if (temp == 0x011) {
        pull_type = GPIO_PU_23_5K;
    } else if (temp == 0x111) {
        pull_type = GPIO_PD_23_5K;
    } else if ((temp == 0x100) || (temp == 0x000)) {
        pull_type = GPIO_NO_PULL;
    } else if (temp != 0xfff) {
        pull_type = GPIO_PUPD_ERR;
        log_hal_info("error pupd-r0-r1 = %x\r\n", temp);
    }

    hal_gpio_get_driving_current((hal_gpio_pin_t)gpio_pin, &driving_value);

    gpio_state->mode = mode;
    gpio_state->dir  = dir;
    gpio_state->din  = (hal_gpio_data_t)din;
    gpio_state->dout = (hal_gpio_data_t)dout;
    gpio_state->pull_type = pull_type;
    gpio_state->current_type = (hal_gpio_driving_current_t)((uint8_t)driving_value);

    //printf("LOG%d: GPIO%d, mode=%d, %s, din=%d, dout=%d, %s\r\n",index, gpio_pin, mode, direct[dir], din,dout,pull_state[pull_type]);

}

#ifdef __cplusplus
}
#endif

#endif  /* HAL_GPIO_MODULE_ENABLED */


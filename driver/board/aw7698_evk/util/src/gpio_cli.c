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
 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "toi.h"
#include "hal_gpio.h"
#include "gpio_cli.h"

#define GPIO_IOT_MODE0_ADD 0x81023020
#define GPIO_IOT_PULL_UP0_ADD 0x8300B000
#define GPIO_IOT_PULL_DOWN0_ADD 0x8300B030

hal_gpio_status_t hal_iot_gpio_get_pinmux(hal_gpio_pin_t pin_number, uint8_t *function_index)
{
    uint32_t no;
    uint32_t remainder;
    uint32_t temp;

    /* get the register number corresponding to the pin as one register can control 8 pins*/
    no = pin_number / 8;

    /* get the bit offset within the register as one register can control 8 pins*/
    remainder = pin_number % 8;

    temp = *(volatile unsigned int *)(GPIO_IOT_MODE0_ADD + (no * 0x4));

    temp &= (0xF << ( 4 * remainder ));
    *function_index = (temp >> ( 4 * remainder ));
    return HAL_GPIO_STATUS_OK;
}


hal_gpio_status_t hal_iot_gpio_get_pull(hal_gpio_pin_t pin_number, uint8_t *pull_state)
{
    uint8_t no;
    uint8_t remainder;
    uint32_t temp1, temp2;

    /* get the register number corresponding to the pin as one register can control 8 pins*/
    no = pin_number / 32;

    /* get the bit offset within the register as one register can control 8 pins*/
    remainder = pin_number % 32;

    temp1 = *(volatile unsigned int *)( GPIO_IOT_PULL_UP0_ADD + (no * 0x10) );
    temp1 &= (0x1 << remainder );
    temp1 = (temp1 >> remainder);

    temp2 = *(volatile unsigned int *)( GPIO_IOT_PULL_DOWN0_ADD + (no * 0x10) );
    temp2 &= (0x1 << remainder);
    temp2 = (temp2 >> remainder);
    if (0 == temp1 && 0 == temp2) {
        *pull_state = 2;
    } else if (1 == temp1) {
        *pull_state = 1;

    } else if (1 == temp2) {
        *pull_state = 0;
    }

    return HAL_GPIO_STATUS_OK;
}



static unsigned char gpio_set_pull(uint8_t len, char *param[])
{
    uint8_t type;
    uint8_t config_index[2];

    if ( 2 != len) {
        cli_putln();
        cli_puts("parameter error\n");
        cli_putln();
        return 1;
    }

    config_index[0] = toi(param[0], &type);       //  pin
    config_index[1] = toi(param[1], &type);       //  pull stat


    if ((config_index[0] >= 72) || (config_index[1] > 2) ) {
        cli_putln();
        cli_puts("parameter error\n");
        cli_putln();
        return 3;
    }

    /* set pull */
    if (2 == config_index[1]) {
        hal_gpio_disable_pull((hal_gpio_pin_t)config_index[0]);
    } else if (1 == config_index[1]) {
        hal_gpio_pull_up((hal_gpio_pin_t)config_index[0]);
    } else if (0 == config_index[1]) {
        hal_gpio_pull_down((hal_gpio_pin_t)config_index[0]);
    }

    cli_putln();
    cli_puts("configure done\r\n");
    cli_puts("0: pull down\r\n1: pull up\r\n2: disable pull\r\n");
    cli_putln();
    return 0;
}




static unsigned char gpio_set_od(uint8_t len, char *param[])
{
    uint8_t type;
    uint8_t config_index[2];

    if ( 2 != len) {
        cli_putln();
        cli_puts("parameter error\n");
        cli_putln();
        return 1;
    }

    config_index[0] = toi(param[0], &type);       //  pin
    config_index[1] = toi(param[1], &type);      //  output data

    if ((config_index[0] >= 72) || (config_index[1] > 1) ) {
        cli_putln();
        cli_puts("parameter error\n");
        return 3;
    }

    /* set pinmux */
    hal_gpio_set_output((hal_gpio_pin_t)config_index[0], (hal_gpio_data_t)config_index[1]);

    cli_putln();
    cli_puts("configure done\n");
    cli_putln();
    return 0;
}



static unsigned char gpio_set_dir(uint8_t len, char *param[])
{
    uint8_t type;
    uint8_t config_index[2];

    if ( 2 != len) {
        cli_putln();
        cli_puts("parameter error\n");
        return 1;
    }

    config_index[0] = toi(param[0], &type);        //  pin
    config_index[1] = toi(param[1], &type);        //  dir

    if ((config_index[0] >= 72) || (config_index[1] > 1) ) {
        cli_putln();
        cli_puts("parameter error\n");
        cli_putln();
        return 3;
    }

    /* set pinmux */
    hal_gpio_set_direction((hal_gpio_pin_t)config_index[0], (hal_gpio_direction_t)config_index[1]);

    cli_putln();
    cli_puts("configure done\n");
    cli_putln();
    return 0;
}



static unsigned char gpio_set_mode(uint8_t len, char *param[])
{
    uint8_t type;
    uint8_t config_index[2];

    if ( 2 != len) {
        cli_putln();
        cli_puts("parameter error\n");
        cli_putln();
        return 1;
    }

    config_index[0] = toi(param[0], &type);           //  pin
    config_index[1] = toi(param[1], &type);           //  mode

    if ((config_index[0] >= 72) || (config_index[1] > 15) ) {
        cli_putln();
        cli_puts("parameter error\n");
        cli_putln();
        return 2;
    }

    /* set pinmux */
    hal_pinmux_set_function((hal_gpio_pin_t)config_index[0], config_index[1]);

    cli_putln();
    cli_puts("configure done\n");
    cli_putln();
    return 0;
}


static unsigned char gpio_set(uint8_t len, char *param[])
{
    uint8_t type;
    uint8_t config_index[7];

    if ( 5 != len) {
        cli_putln();
        cli_puts("parameter error\n");
        return 1;
    }

    config_index[0] = toi(param[0], &type);       //  pin
    config_index[1] = toi(param[1], &type);       //  mode
    config_index[2] = toi(param[2], &type);       //  dir
    config_index[3] = toi(param[3], &type);       //  pull
    config_index[4] = toi(param[4], &type);       //  od

    if ((config_index[0] > 72) || (config_index[1] > 15) || (config_index[2] > 1) || (config_index[3] > 2) || (config_index[4] > 1) ) {
        cli_putln();
        cli_puts("parameter error\n");
        cli_putln();
        return 3;
    }

    /* set pinmux */
    hal_pinmux_set_function((hal_gpio_pin_t)config_index[0], config_index[1]);

    /*set direction */
    hal_gpio_set_direction((hal_gpio_pin_t)config_index[0], (hal_gpio_direction_t)config_index[2]);

    /*set direction
      config_index[3] =
                        2: disable pull
                        0: pull down
                        1: pull up
    */
    if (config_index[3] == 2) { /*disabl pull*/
        hal_gpio_disable_pull((hal_gpio_pin_t)config_index[0]);
    } else if (1 == config_index[3]) {
        hal_gpio_pull_up((hal_gpio_pin_t)config_index[0]);
    } else if (0 == config_index[3]) {
        hal_gpio_pull_down((hal_gpio_pin_t)config_index[0]);
    }

    /*set output data */
    hal_gpio_set_output((hal_gpio_pin_t)config_index[0], (hal_gpio_data_t)config_index[4]);
    cli_putln();
    cli_puts("configure done\n");
    cli_putln();
    return 0;
}

static unsigned char gpio_get(uint8_t len, char *param[])
{

    hal_gpio_data_t input_gpio_data = HAL_GPIO_DATA_LOW;
    hal_gpio_data_t output_gpio_data = HAL_GPIO_DATA_LOW;
    hal_gpio_direction_t gpio_dir;
    uint8_t i, gpio_function_index, pull;

    cli_putln();
    cli_puts("PIN:[mode][dir][pull][output][input]\n");
    cli_putln();

    for (i = 0; i < 73; i ++) {
        hal_iot_gpio_get_pinmux((hal_gpio_pin_t)i, &gpio_function_index);

        hal_gpio_get_direction((hal_gpio_pin_t)i, &gpio_dir);
        if (HAL_GPIO_DIRECTION_OUTPUT == gpio_dir) {
            hal_gpio_get_output((hal_gpio_pin_t)i, &output_gpio_data);
        } else {
            hal_gpio_get_input((hal_gpio_pin_t)i, &input_gpio_data);
        }

        hal_iot_gpio_get_pull((hal_gpio_pin_t)i, &pull);
        printf("%-6d%-6d%-6d%-6d%-7d%d\n", i, gpio_function_index, gpio_dir, pull, output_gpio_data, input_gpio_data);
        cli_putln();
    }

    cli_puts("done\r\n");
    cli_puts("0: pull down\r\n1: pull up\r\n2: disable pull\r\n");
    return 0;
}


cmd_t gpio_cli_cmds[] = {
    { "get", "get configurations of all pins", gpio_get, NULL },
    { "set", "set serveral configurations of one pins", gpio_set, NULL },
    { "set_mode", "set serveral configurations of one pins", gpio_set_mode, NULL },
    { "set_dir", "set serveral configurations of one pins", gpio_set_dir, NULL },
    { "set_pull", "set serveral configurations of one pins", gpio_set_pull, NULL },
    { "set_od", "set serveral configurations of one pins", gpio_set_od, NULL },
    { NULL, NULL, NULL, NULL }
};

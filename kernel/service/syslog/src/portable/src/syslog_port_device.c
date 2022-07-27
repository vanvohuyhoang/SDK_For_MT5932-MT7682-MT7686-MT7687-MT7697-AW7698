/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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

#include "syslog_port_device.h"

extern port_syslog_device_ops_t g_port_syslog_uart_ops;
extern port_syslog_device_ops_t g_port_syslog_usb_ops;
extern port_syslog_device_ops_t g_port_syslog_ram_flash_ops;

port_syslog_device_ops_t *g_syslog_device_ops_array[] = {
    &g_port_syslog_uart_ops,
    &g_port_syslog_usb_ops,
    &g_port_syslog_ram_flash_ops,
};

#ifdef MTK_SAVE_LOG_TO_FLASH_ENABLE

extern void port_syslog_device_get_setting(log_port_type_t *port_type, uint8_t *port_index);

#else

#ifdef MTK_PORT_SERVICE_ENABLE
#include "serial_port.h"

void port_syslog_device_get_setting(log_port_type_t *port_type, uint8_t *port_index)
{
    bool save_default_value;
    serial_port_dev_t port;
    serial_port_type_t type;
    serial_port_status_t status;

    save_default_value = false;
    status = serial_port_config_read_dev_number("syslog", &port);
    if (status == SERIAL_PORT_STATUS_OK) {
        type = serial_port_get_device_type(port);
        if ((type != SERIAL_PORT_TYPE_UART) && (type != SERIAL_PORT_TYPE_USB)) {
            save_default_value = true;
        }
    } else {
        save_default_value = true;
    }

    if (save_default_value == true) {
        port = *port_index;
        if (*port_type == LOG_PORT_TYPE_USB) {
            port += SERIAL_PORT_DEV_USB_TYPE_BEGIN;
            type = SERIAL_PORT_TYPE_USB;
        } else {
            type = SERIAL_PORT_TYPE_UART;
        }
        serial_port_config_write_dev_number("syslog", port);
    }

    *port_type = type;
    if (type == SERIAL_PORT_TYPE_USB) {
        port -= SERIAL_PORT_DEV_USB_TYPE_BEGIN;
    }
    *port_index = port;
}
#else
void port_syslog_device_get_setting(log_port_type_t *port_type, uint8_t *port_index)
{
    PORT_SYSLOG_UNUSED(port_type);
    PORT_SYSLOG_UNUSED(port_index);
}
#endif

#endif


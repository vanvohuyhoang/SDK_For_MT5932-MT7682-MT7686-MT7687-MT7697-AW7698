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

#ifndef __SYSLOG_PORT_DEVICE_H__
#define __SYSLOG_PORT_DEVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_uart.h"
#include "syslog_port_common.h"

/* declare functions to be called by porting layer */
typedef void (*pc_rx_handler_t)(void);

typedef struct {
#ifdef MTK_CPU_NUMBER_0
    void (*early_init)(uint8_t port_index, hal_uart_baudrate_t baudrate);
    void (*early_send)(uint8_t port_index, uint8_t *buffer, uint32_t size);
    void (*post_init)(uint8_t port_index, uint32_t share_buffer_addr, uint32_t share_buffer_size, pc_rx_handler_t rx_handler);
    void (*exception_init)(uint8_t port_index);
    void (*exception_send)(uint8_t port_index, uint8_t *buffer, uint32_t size);
    uint32_t (*receive_data)(uint8_t port_index, uint8_t *buffer, uint32_t size);
#endif
    void (*init_hook)(uint8_t port_index);
    void (*logging_hook)(uint8_t port_index);
    uint32_t (*get_hw_rptr)(uint8_t port_index);
    uint32_t (*get_hw_wptr)(uint8_t port_index);
    void (*set_hw_wptr)(uint8_t port_index, uint32_t move_bytes);
} port_syslog_device_ops_t;

/* syslog device porting interface, anyone who want to support syslog should implement those APIs */
typedef enum {
    LOG_PORT_TYPE_UART = 0,
    LOG_PORT_TYPE_USB,
    LOG_PORT_TYPE_RAW_FLASH,
    LOG_PORT_TYPE_MAX,
} log_port_type_t;

#ifdef MTK_CPU_NUMBER_0
#ifdef MTK_SAVE_LOG_TO_FLASH_ENABLE
#define port_syslog_device_early_init(port_type, port_index, baudrate)          g_syslog_device_ops_array[LOG_PORT_TYPE_RAW_FLASH]->early_init(port_index, baudrate)
#define port_syslog_device_early_send(port_type, port_index, buffer, size)      g_syslog_device_ops_array[LOG_PORT_TYPE_RAW_FLASH]->early_send(port_index, buffer, size)
#else
#define port_syslog_device_early_init(port_type, port_index, baudrate)          g_syslog_device_ops_array[port_type]->early_init(port_index, baudrate)
#define port_syslog_device_early_send(port_type, port_index, buffer, size)      g_syslog_device_ops_array[port_type]->early_send(port_index, buffer, size)
#endif
#define port_syslog_device_exception_init(port_type, port_index)                g_syslog_device_ops_array[port_type]->exception_init(port_index)
#define port_syslog_device_exception_send(port_type, port_index, buffer, size)  g_syslog_device_ops_array[port_type]->exception_send(port_index, buffer, size)
#define port_syslog_device_post_init(port_type, port_index, share_buffer_addr, share_buffer_size, rx_handler)   g_syslog_device_ops_array[port_type]->post_init(port_index, share_buffer_addr, share_buffer_size, rx_handler)
#define port_syslog_device_receive_data(port_type, port_index, buffer, size)    g_syslog_device_ops_array[port_type]->receive_data(port_index, buffer, size)
#endif
#define port_syslog_device_init_hook(port_type, port_index)                     g_syslog_device_ops_array[port_type]->init_hook(port_index)
#define port_syslog_device_logging_hook(port_type, port_index)                  g_syslog_device_ops_array[port_type]->logging_hook(port_index)
#define port_syslog_device_get_hw_rptr(port_type, port_index)                   g_syslog_device_ops_array[port_type]->get_hw_rptr(port_index)
#define port_syslog_device_get_hw_wptr(port_type, port_index)                   g_syslog_device_ops_array[port_type]->get_hw_wptr(port_index)
#define port_syslog_device_set_hw_wptr(port_type, port_index, move_bytes)       g_syslog_device_ops_array[port_type]->set_hw_wptr(port_index, move_bytes)

#if defined(MTK_DEBUG_LEVEL_PRINTF)
extern void port_syslog_uart_early_send(uint8_t port_index, uint8_t *buffer, uint32_t size);
#define port_syslog_device_printf_send(port_type, port_index, buffer, size)     port_syslog_uart_early_send(port_index, buffer, size)
#endif

void port_syslog_device_get_setting(log_port_type_t *port_type, uint8_t *port_index);

#ifdef __cplusplus
}
#endif

#endif


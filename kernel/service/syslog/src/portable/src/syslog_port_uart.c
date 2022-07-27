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
#include "hal_uart_internal.h"

#ifdef MTK_CPU_NUMBER_0

#include "hal_gpt.h"

#define SYSLOG_RX_TIMEOUT 200000

ATTR_ZIDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN static uint8_t g_uart_rx_buffer[512];
uint8_t g_sylog_eraly_port_index;
hal_uart_baudrate_t g_sylog_uart_baudrate;
static pc_rx_handler_t g_syslog_pc_rx_handler;

void port_syslog_uart_early_init(uint8_t port_index, hal_uart_baudrate_t baudrate)
{
    hal_uart_config_t uart_config;

    uart_config.baudrate = baudrate;
    uart_config.parity = HAL_UART_PARITY_NONE;
    uart_config.stop_bit = HAL_UART_STOP_BIT_1;
    uart_config.word_length = HAL_UART_WORD_LENGTH_8;
    hal_uart_init(port_index, &uart_config);

#if !defined(MTK_DEBUG_LEVEL_PRINTF)
    hal_uart_set_software_flowcontrol(port_index, 0x11, 0x13, 0x77);
#endif

    g_sylog_eraly_port_index = port_index;
    g_sylog_uart_baudrate = baudrate;
}

void port_syslog_uart_exception_init(uint8_t port_index)
{
    hal_uart_config_t uart_config;

    hal_uart_deinit(port_index);
    uart_config.baudrate = g_sylog_uart_baudrate;
    uart_config.parity = HAL_UART_PARITY_NONE;
    uart_config.stop_bit = HAL_UART_STOP_BIT_1;
    uart_config.word_length = HAL_UART_WORD_LENGTH_8;
    hal_uart_init(port_index, &uart_config);

#if !defined(MTK_DEBUG_LEVEL_PRINTF)
    hal_uart_set_software_flowcontrol(port_index, 0x11, 0x13, 0x77);
#endif
}

void port_syslog_uart_exception_send(uint8_t port_index, uint8_t *buffer, uint32_t size)
{
    uart_exception_send_polling(port_index, buffer, size);
}


static void user_uart_callback(hal_uart_callback_event_t event, void *user_data)
{
    PORT_SYSLOG_UNUSED(user_data);

    if (event == HAL_UART_EVENT_READY_TO_READ) {
        g_syslog_pc_rx_handler();
    }
}

void port_syslog_uart_post_init(uint8_t port_index, uint32_t share_buffer_addr, uint32_t share_buffer_size, pc_rx_handler_t rx_handler)
{
    hal_uart_config_t uart_config;
    hal_uart_dma_config_t dma_config;

    g_syslog_pc_rx_handler = rx_handler;

    uart_config.baudrate = g_sylog_uart_baudrate;
    uart_config.parity = HAL_UART_PARITY_NONE;
    uart_config.stop_bit = HAL_UART_STOP_BIT_1;
    uart_config.word_length = HAL_UART_WORD_LENGTH_8;
    dma_config.send_vfifo_buffer              = (uint8_t *)share_buffer_addr;
    dma_config.send_vfifo_buffer_size         = share_buffer_size;
    dma_config.send_vfifo_threshold_size      = share_buffer_size / 8;
    dma_config.receive_vfifo_buffer           = g_uart_rx_buffer;
    dma_config.receive_vfifo_buffer_size      = sizeof(g_uart_rx_buffer);
    dma_config.receive_vfifo_threshold_size   = (sizeof(g_uart_rx_buffer) * 9) / 10;
    dma_config.receive_vfifo_alert_size       = 16;
    hal_uart_deinit(g_sylog_eraly_port_index);
    uart_init_for_logging(port_index, &uart_config, &dma_config, user_uart_callback, NULL);
    hal_uart_set_software_flowcontrol(port_index, 0x11, 0x13, 0x77);
}

uint32_t port_syslog_uart_receive_data(uint8_t port_index, uint8_t *buffer, uint32_t size)
{
    uint8_t *p_buffer;
    uint32_t curr_size, left_size;
    uint32_t start_time, end_time, during_time;

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &start_time);

    p_buffer = buffer;
    left_size = size;
    do {
        curr_size = hal_uart_receive_dma(port_index, p_buffer, left_size);
        left_size -= curr_size;
        p_buffer += curr_size;
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &end_time);
        hal_gpt_get_duration_count(start_time, end_time, &during_time);
        if (during_time > SYSLOG_RX_TIMEOUT) {
            break;
        }
    } while (left_size);

    return size - left_size;
}

#endif

#if defined(MTK_CPU_NUMBER_0) || defined(MTK_DEBUG_LEVEL_PRINTF)
void port_syslog_uart_early_send(uint8_t port_index, uint8_t *buffer, uint32_t size)
{
    uart_send_polling(port_index, (const uint8_t *)buffer, size);
}
#endif

void port_syslog_uart_init_hook(uint8_t port_index)
{
    PORT_SYSLOG_UNUSED(port_index);
}

void port_syslog_uart_logging_hook(uint8_t port_index)
{
    PORT_SYSLOG_UNUSED(port_index);
}

uint32_t port_syslog_uart_get_hw_rptr(uint8_t port_index)
{
    return uart_get_hw_rptr(port_index);
}

uint32_t port_syslog_uart_get_hw_wptr(uint8_t port_index)
{
    return uart_get_hw_wptr(port_index);
}

void port_syslog_uart_set_hw_wptr(uint8_t port_index, uint32_t move_bytes)
{
    uart_set_sw_move_byte(port_index, move_bytes);
}

port_syslog_device_ops_t g_port_syslog_uart_ops = {
#ifdef MTK_CPU_NUMBER_0
    port_syslog_uart_early_init,
    port_syslog_uart_early_send,
    port_syslog_uart_post_init,
    port_syslog_uart_exception_init,
    port_syslog_uart_exception_send,
    port_syslog_uart_receive_data,
#endif
    port_syslog_uart_init_hook,
    port_syslog_uart_logging_hook,
    port_syslog_uart_get_hw_rptr,
    port_syslog_uart_get_hw_wptr,
    port_syslog_uart_set_hw_wptr,
};


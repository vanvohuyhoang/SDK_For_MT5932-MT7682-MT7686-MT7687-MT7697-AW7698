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

#ifdef MTK_USB_DEMO_ENABLED

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "hal_usb.h"
#include "hal_resource_assignment.h"
#include "syslog_port_device.h"
#include "hal_uart_internal.h"
#include "hal_gpt.h"



#ifdef HAL_USB_MODULE_ENABLED

#include "hal_platform.h"
/** @brief
 * This enum defines the events that trigger the user's callback.
 */

typedef void (*usb_syslog_register_callback_t)(uint8_t port);

typedef struct {
    uint8_t *data;                   /**< The data to write to the serial port. */
    uint32_t size;                   /**< Size of the data to write to the serial port. */
    uint32_t ret_size;               /**< The actual size of data written to the serial port. */
} usb_log_write_data_t;

typedef struct {
    uint32_t *USB2Uart_shortpkg_addr[2];
    uint32_t USB2Uart_shortpkg_force[2];
    uint32_t port_index;
    uint32_t vdma_addr;
    uint32_t vdma_size;
    uint32_t vdma_rptr;
    uint32_t vdma_wptr;
    uint32_t pdma_rptr;
} usb_syslog_saved_data_t;

#endif


static volatile usb_syslog_saved_data_t *usb_syslog_saved_data = (volatile usb_syslog_saved_data_t *)(HW_SYSRAM_PRIVATE_MEMORY_SYSLOG_USB_VAR_START - 0x40);

#ifdef MTK_CPU_NUMBER_0

#include "usbacm_drv.h"
#include "serial_port.h"
#include "usbacm_adap.h"
#include "usb_custom_def.h"


typedef void (*usb_syslog_callback_t)(serial_port_dev_t device, serial_port_callback_event_t event, void *parameter);
extern serial_port_register_callback_t g_serial_port_usb_callback[];

static bool g_usb_is_initialized = false;
static pc_rx_handler_t g_syslog_pc_rx_handler;
static uint32_t g_pc_command_buffer_length;

extern UsbUARTStruct USB2UARTPort[];
extern uint8_t g_sylog_eraly_port_index;

bool usb_syslog_init(uint8_t usb_port, usb_syslog_callback_t callback)
{
    uint32_t length_tx = 0 , length_rx = 0;

    USB2UART_init();

    if (usb_port == 0) {
        length_tx = USB_TX_RING_BUFFER_1_SIZE;
        length_rx = USB_RX_RING_BUFFER_1_SIZE;
    } else if (usb_port == 1) {
        length_tx = USB_TX_RING_BUFFER_2_SIZE;
        length_rx = USB_RX_RING_BUFFER_2_SIZE;
    } else {
        //LOG_I(hal, "ASSERT");
    }

    if (USB2UARTPort[usb_port].RingBuffers.rx_buffer == NULL) {
        USB2UARTPort[usb_port].RingBuffers.rx_buffer = (uint8_t *)USB_Get_Memory(length_rx);    //(uint8_t *)g_UsbACM_Comm.acm_param->rx_ringbuff_2;
    }

    Buf_init(&(USB2UARTPort[usb_port].Rx_Buffer), (uint8_t *)(USB2UARTPort[usb_port].RingBuffers.rx_buffer), length_rx);

    if (USB2UARTPort[usb_port].RingBuffers.tx_buffer == NULL) {
        USB2UARTPort[usb_port].RingBuffers.tx_buffer = (uint8_t *)USB_Get_Memory(length_tx);    //(uint8_t *)g_UsbACM_Comm.acm_param->tx_ringbuff_2;
    }
    Buf_init(&(USB2UARTPort[usb_port].Tx_Buffer), (uint8_t *)(USB2UARTPort[usb_port].RingBuffers.tx_buffer), length_tx);

    USB2UARTPort[usb_port].initialized = true;

    g_UsbACM[usb_port].send_Txilm = false;
    g_UsbACM[usb_port].send_Rxilm = true;
    g_UsbACM[usb_port].transfer_type = CDC_NORMAL;
    g_UsbACM[usb_port].config_send_Txilm = false;

    g_serial_port_usb_callback[usb_port] = callback;

    if (USB_Get_Device_State() != DEVSTATE_CONFIG) {
        return false;
    }

    if (g_UsbACM[usb_port].txpipe != NULL) {
        /* Only RX EP needs to be enabled since TX EP will use DMA polling */
        hal_usb_enable_tx_endpoint(g_UsbACM[usb_port].txpipe->byEP, HAL_USB_EP_TRANSFER_BULK, HAL_USB_EP_USE_ONLY_DMA, false);
        hal_usb_enable_rx_endpoint(g_UsbACM[usb_port].rxpipe->byEP, HAL_USB_EP_TRANSFER_BULK, HAL_USB_EP_USE_NO_DMA, false);

        /* Open intr */
        hal_usb_enable_rx_endpoint(g_UsbACM[usb_port].rxpipe->byEP, HAL_USB_EP_TRANSFER_BULK, HAL_USB_EP_USE_NO_DMA, true);
    }

    return true;
}

static void pc_log_usb_callback(serial_port_dev_t device, serial_port_callback_event_t event, void *parameter)
{
    uint32_t usb_port = USB_PORT[device];
    BUFFER_INFO *rx_buf_info;

    rx_buf_info = &(USB2UARTPort[usb_port].Rx_Buffer);

    PORT_SYSLOG_UNUSED(usb_port);

    g_pc_command_buffer_length = USB2UART_GetBufAvail(rx_buf_info);

    g_syslog_pc_rx_handler();
}

static bool usb_syslog_dump_data(uint8_t usb_port, void *start_address, uint32_t size)
{
    uint32_t Length = 0;
    bool is_unalign_data = false;
    uint8_t data[4];
    uint32_t i = 0;

    while (size > 0) {
        if (size > 64) {
            Length = 64;
        } else if (size & 0x03) {
            Length = size & (~0x03);
            is_unalign_data = true;
        } else {
            Length = size;
        }

        hal_usb_set_endpoint_tx_ready(usb_port + 1);
        while (!hal_usb_is_endpoint_tx_empty(usb_port + 1));
        hal_usb_write_endpoint_fifo(usb_port + 1, Length, start_address);
        hal_usb_set_endpoint_tx_ready(usb_port + 1);
        while (!hal_usb_is_endpoint_tx_empty(usb_port + 1));

        size = size - Length;
        start_address = start_address + Length;

        if (is_unalign_data) {
            for (i = 0; i < (size & 0x03); i++) {
                data[i] = *(uint8_t *)(start_address + i);
            }
            hal_usb_set_endpoint_tx_ready(usb_port + 1);
            while (!hal_usb_is_endpoint_tx_empty(usb_port + 1));
            hal_usb_write_endpoint_fifo(usb_port + 1, size & 0x03, data);
            hal_usb_set_endpoint_tx_ready(usb_port + 1);
            while (!hal_usb_is_endpoint_tx_empty(usb_port + 1));
            size = size - (size & 0x03);
            start_address = start_address + (size & 0x03);
        }
    }
    return true;
}

void port_syslog_usb_early_init(uint8_t port_index, hal_uart_baudrate_t baudrate)
{
    hal_uart_config_t uart_config;

    uart_config.baudrate = baudrate;
    uart_config.parity = HAL_UART_PARITY_NONE;
    uart_config.stop_bit = HAL_UART_STOP_BIT_1;
    uart_config.word_length = HAL_UART_WORD_LENGTH_8;
    hal_uart_init(port_index, &uart_config);
}

void port_syslog_usb_early_send(uint8_t port_index, uint8_t *buffer, uint32_t size)
{
    uart_send_polling(port_index, (const uint8_t *)buffer, size);
}

void port_syslog_usb_exception_init(uint8_t port_index)
{
    PORT_SYSLOG_UNUSED(port_index);
}

void port_syslog_usb_exception_send(uint8_t port_index, uint8_t *buffer, uint32_t size)
{
    if (g_usb_is_initialized == false) {
        uart_exception_send_polling(port_index, buffer, size);
    } else {
        usb_syslog_dump_data(port_index, buffer, size);
    }
}

void port_syslog_usb_post_init(uint8_t port_index, uint32_t share_buffer_addr, uint32_t share_buffer_size, pc_rx_handler_t rx_handler)
{
    usb_syslog_saved_data->port_index = port_index;
    usb_syslog_saved_data->vdma_addr = share_buffer_addr;
    usb_syslog_saved_data->vdma_size = share_buffer_size;

    g_syslog_pc_rx_handler = rx_handler;

    usb_syslog_init(port_index, pc_log_usb_callback);

    g_usb_is_initialized = true;

    hal_uart_deinit(g_sylog_eraly_port_index);
}

uint32_t port_syslog_usb_receive_data(uint8_t port_index, uint8_t *buffer, uint32_t size)
{
    uint16_t count;

    PORT_SYSLOG_UNUSED(port_index);

    if (g_pc_command_buffer_length < size) {
        size = g_pc_command_buffer_length;
    }

    count = USB2UART_GetBytes(port_index, buffer, (uint16_t)size);
    if (count != size) {
        /**/
    }

    g_pc_command_buffer_length -= size;

    return size;
}

#endif

#ifdef MTK_SYSLOG_SUB_FEATURE_USB_ACTIVE_MODE

#define USB_GPT_TIMEOUT 100

static uint32_t g_usb_gpt_handler;
static uint32_t g_usb_irq_mask;

bool usb_syslog_start_pdma(uint8_t usb_port, void *start_address, uint32_t size)
{
#if 0
    if (size != 0) {
        if (size % HAL_USB_MAX_PACKET_SIZE_ENDPOINT_BULK_FULL_SPEED == 0) {
            usb_syslog_saved_data->USB2Uart_shortpkg_force[usb_port] = true;
            usb_syslog_saved_data->USB2Uart_shortpkg_addr[usb_port] = start_address + size - 4;
            size -= 4;
        }

        hal_usb_start_dma_channel(usb_port + 1, HAL_USB_EP_DIRECTION_TX, HAL_USB_EP_TRANSFER_BULK, (void *)start_address, size, (hal_usb_dma_handler_t)usb_syslog_callback, false, HAL_USB_DMA1_TYPE);
        return true;
    } else {
        return false;
    }
#else
    if (size != 0) {
        hal_usb_start_dma_channel(usb_port + 1, HAL_USB_EP_DIRECTION_TX, HAL_USB_EP_TRANSFER_BULK, (void *)start_address, size, NULL, false, HAL_USB_DMA1_TYPE);
        return true;
    } else {
        return false;
    }
#endif
}

bool usb_syslog_query_pdma_status(uint8_t usb_port)
{
    return hal_usb_is_dma_running(usb_port + 1, HAL_USB_EP_DIRECTION_TX);
}

static void usb_log_enter_critical(void)
{
    hal_nvic_save_and_set_interrupt_mask(&g_usb_irq_mask);
    while (hal_hw_semaphore_take(HW_SEMAPHORE_SYSLOG_WRAP_LAYER) != HAL_HW_SEMAPHORE_STATUS_OK);
}

static void usb_log_exit_critical(void)
{
    while (hal_hw_semaphore_give(HW_SEMAPHORE_SYSLOG_WRAP_LAYER) != HAL_HW_SEMAPHORE_STATUS_OK);
    hal_nvic_restore_interrupt_mask(g_usb_irq_mask);
}

static void usb_gpt_callback(void *user_data)
{
    user_data = user_data;

    /* Diable IRQ and take HW semaphore. */
    usb_log_enter_critical();

#ifdef MTK_CPU_NUMBER_0
    if (USB_Get_Device_State() != DEVSTATE_CONFIG) {
        /* Start a timer to continue next query of PDMA status. */
        hal_gpt_sw_start_timer_ms(g_usb_gpt_handler,
                                  USB_GPT_TIMEOUT,
                                  usb_gpt_callback,
                                  NULL);
        /* Release HW semaphore and enable IRQ */
        usb_log_exit_critical();
        return;
    }
#endif

    /* Check whether current PDMA transmition has finished? */
    if (usb_syslog_query_pdma_status(usb_syslog_saved_data->port_index) == true) {
        /* Start a timer to continue next query of PDMA status. */
        hal_gpt_sw_start_timer_ms(g_usb_gpt_handler,
                                  USB_GPT_TIMEOUT,
                                  usb_gpt_callback,
                                  NULL);
        /* Release HW semaphore and enable IRQ */
        usb_log_exit_critical();
        return;
    }

    /* Update the HW RPTR to the HW WPTR. */
    usb_syslog_saved_data->vdma_rptr = usb_syslog_saved_data->pdma_rptr;

    /* If there is NO more data ready, No need to start the GPT. */
    if (usb_syslog_saved_data->vdma_rptr == usb_syslog_saved_data->vdma_wptr) {
        /* Start a timer to continue next query of PDMA status. */
        hal_gpt_sw_start_timer_ms(g_usb_gpt_handler,
                                  USB_GPT_TIMEOUT,
                                  usb_gpt_callback,
                                  NULL);
        /* Release HW semaphore and enable IRQ */
        usb_log_exit_critical();
        return;
    }

    /* check whether wrap happen */
    if (usb_syslog_saved_data->vdma_wptr > usb_syslog_saved_data->vdma_rptr) {
        /* start the frist part transmition */
        usb_syslog_start_pdma(usb_syslog_saved_data->port_index,
                              (void *)(usb_syslog_saved_data->vdma_rptr + usb_syslog_saved_data->vdma_addr),
                              usb_syslog_saved_data->vdma_wptr - usb_syslog_saved_data->vdma_rptr);
        /* update the WPTR to begin of the buffer */
        usb_syslog_saved_data->pdma_rptr = usb_syslog_saved_data->vdma_wptr;
    } else {
        /* start the frist part transmition */
        usb_syslog_start_pdma(usb_syslog_saved_data->port_index,
                              (void *)(usb_syslog_saved_data->vdma_rptr + usb_syslog_saved_data->vdma_addr),
                              usb_syslog_saved_data->vdma_size - usb_syslog_saved_data->vdma_rptr);
        /* update the WPTR to begin of the buffer */
        usb_syslog_saved_data->pdma_rptr = 0;
    }

    /* start GPT to query PDMA status */
    hal_gpt_sw_start_timer_ms(g_usb_gpt_handler,
                              USB_GPT_TIMEOUT,
                              usb_gpt_callback,
                              NULL);

    /* Release HW semaphore and enable IRQ */
    usb_log_exit_critical();
}

#endif

void port_syslog_usb_init_hook(uint8_t port_index)
{
    PORT_SYSLOG_UNUSED(port_index);

#ifdef MTK_SYSLOG_SUB_FEATURE_USB_ACTIVE_MODE
    if (hal_gpt_sw_get_timer((uint32_t *)&g_usb_gpt_handler) != HAL_GPT_STATUS_OK) {
        assert(0);
    }

    /* start GPT to query PDMA status */
    hal_gpt_sw_start_timer_ms(g_usb_gpt_handler,
                              USB_GPT_TIMEOUT,
                              usb_gpt_callback,
                              NULL);
#endif
}

void port_syslog_usb_logging_hook(uint8_t port_index)
{
#ifdef MTK_SYSLOG_SUB_FEATURE_USB_ACTIVE_MODE
    /* Diable IRQ and take HW semaphore. */
    usb_log_enter_critical();

    /* check whether PDMA is busy */
#ifdef MTK_CPU_NUMBER_0
    if (USB_Get_Device_State() != DEVSTATE_CONFIG) {
        usb_log_exit_critical();
        return;
    }
#endif
    if (usb_syslog_query_pdma_status(port_index) == false) {
        /* update the RPTR to the position of WPTR */
        usb_syslog_saved_data->vdma_rptr = usb_syslog_saved_data->pdma_rptr;

        /* check whether wrap happen */
        if (usb_syslog_saved_data->vdma_wptr > usb_syslog_saved_data->vdma_rptr) {
            /* start the frist part transmition */
            usb_syslog_start_pdma(port_index,
                                  (void *)(usb_syslog_saved_data->vdma_rptr + usb_syslog_saved_data->vdma_addr),
                                  usb_syslog_saved_data->vdma_wptr - usb_syslog_saved_data->vdma_rptr);
            /* update the WPTR to begin of the buffer */
            usb_syslog_saved_data->pdma_rptr = usb_syslog_saved_data->vdma_wptr;
        } else if (usb_syslog_saved_data->vdma_wptr < usb_syslog_saved_data->vdma_rptr) {
            /* start the frist part transmition */
            usb_syslog_start_pdma(port_index,
                                  (void *)(usb_syslog_saved_data->vdma_rptr + usb_syslog_saved_data->vdma_addr),
                                  usb_syslog_saved_data->vdma_size - usb_syslog_saved_data->vdma_rptr);
            /* update the WPTR to begin of the buffer */
            usb_syslog_saved_data->pdma_rptr = 0;
        }
    }

    /* Release HW semaphore and enable IRQ */
    usb_log_exit_critical();
#endif
}

uint32_t port_syslog_usb_get_hw_rptr(uint8_t port_index)
{
    PORT_SYSLOG_UNUSED(port_index);

    return usb_syslog_saved_data->vdma_rptr;
}

uint32_t port_syslog_usb_get_hw_wptr(uint8_t port_index)
{
    PORT_SYSLOG_UNUSED(port_index);

    return usb_syslog_saved_data->vdma_wptr;
}

void port_syslog_usb_set_hw_wptr(uint8_t port_index, uint32_t move_bytes)
{
    PORT_SYSLOG_UNUSED(port_index);

    if ((usb_syslog_saved_data->vdma_size - usb_syslog_saved_data->vdma_wptr) > move_bytes) {
        usb_syslog_saved_data->vdma_wptr += move_bytes;
    } else {
        usb_syslog_saved_data->vdma_wptr = move_bytes - (usb_syslog_saved_data->vdma_size - usb_syslog_saved_data->vdma_wptr);
    }
}

#else

#include "syslog_port_device.h"

#ifdef MTK_CPU_NUMBER_0

void port_syslog_usb_early_init(uint8_t port_index, hal_uart_baudrate_t baudrate)
{
    PORT_SYSLOG_UNUSED(port_index);
    PORT_SYSLOG_UNUSED(baudrate);
}

void port_syslog_usb_early_send(uint8_t port_index, uint8_t *buffer, uint32_t size)
{
    PORT_SYSLOG_UNUSED(port_index);
    PORT_SYSLOG_UNUSED(buffer);
    PORT_SYSLOG_UNUSED(size);
}

void port_syslog_usb_exception_init(uint8_t port_index)
{
    PORT_SYSLOG_UNUSED(port_index);
}

void port_syslog_usb_exception_send(uint8_t port_index, uint8_t *buffer, uint32_t size)
{
    PORT_SYSLOG_UNUSED(port_index);
    PORT_SYSLOG_UNUSED(buffer);
    PORT_SYSLOG_UNUSED(size);
}

void port_syslog_usb_post_init(uint8_t port_index, uint32_t share_buffer_addr, uint32_t share_buffer_size, pc_rx_handler_t rx_handler)
{
    PORT_SYSLOG_UNUSED(port_index);
    PORT_SYSLOG_UNUSED(share_buffer_addr);
    PORT_SYSLOG_UNUSED(share_buffer_size);
    PORT_SYSLOG_UNUSED(rx_handler);
}

uint32_t port_syslog_usb_receive_data(uint8_t port_index, uint8_t *buffer, uint32_t size)
{
    PORT_SYSLOG_UNUSED(port_index);
    PORT_SYSLOG_UNUSED(buffer);
    PORT_SYSLOG_UNUSED(size);

    return 0;
}
#endif

void port_syslog_usb_init_hook(uint8_t port_index)
{
    PORT_SYSLOG_UNUSED(port_index);
}

void port_syslog_usb_logging_hook(uint8_t port_index)
{
    PORT_SYSLOG_UNUSED(port_index);
}

uint32_t port_syslog_usb_get_hw_rptr(uint8_t port_index)
{
    PORT_SYSLOG_UNUSED(port_index);

    return 0;
}

uint32_t port_syslog_usb_get_hw_wptr(uint8_t port_index)
{
    PORT_SYSLOG_UNUSED(port_index);

    return 0;
}

void port_syslog_usb_set_hw_wptr(uint8_t port_index, uint32_t move_bytes)
{
    PORT_SYSLOG_UNUSED(port_index);
    PORT_SYSLOG_UNUSED(move_bytes);
}

#endif

port_syslog_device_ops_t g_port_syslog_usb_ops = {
#ifdef MTK_CPU_NUMBER_0
    port_syslog_usb_early_init,
    port_syslog_usb_early_send,
    port_syslog_usb_post_init,
    port_syslog_usb_exception_init,
    port_syslog_usb_exception_send,
    port_syslog_usb_receive_data,
#endif
    port_syslog_usb_init_hook,
    port_syslog_usb_logging_hook,
    port_syslog_usb_get_hw_rptr,
    port_syslog_usb_get_hw_wptr,
    port_syslog_usb_set_hw_wptr,
};


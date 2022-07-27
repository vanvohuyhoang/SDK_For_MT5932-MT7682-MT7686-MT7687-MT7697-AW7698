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

#if !defined (MTK_DEBUG_LEVEL_NONE)

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "syslog.h"
#include "toi.h"
#include "hal_gpt.h"
#include "hal_nvic_internal.h"
#include "exception_handler.h"
#include "timers.h"
#if defined(MTK_PORT_SERVICE_ENABLE)
#include "serial_port.h"
#endif
#include "task_def.h"

/* max size of each block of normal log message */
#define MAX_LOG_SIZE (256)

/* max size of each block of hex dump message */
#define MAX_DUMP_MESSAGE_SIZE (128)

/* max size of each block of hex dump message */
#define MAX_DUMP_DATA_SIZE (128)

/* max hex dump length */
#define MAX_HEXDUMP_LENGTH (2048)

#if (PRODUCT_VERSION == 7687) || (PRODUCT_VERSION == 7697) || (PRODUCT_VERSION == 7686) || (PRODUCT_VERSION == 7698)|| (PRODUCT_VERSION == 7682) || (PRODUCT_VERSION == 5932) || defined(MTK_NO_PSRAM_ENABLE)

/* buffer pool items */
#define LOG_POOL_ITEMS (SYSLOG_QUEUE_LENGTH + 8)

/* maximum delay to wait for available log queue */
#define LOG_QUEUE_WAITING_TIME (portMAX_DELAY)

#elif (PRODUCT_VERSION == 2523) || (PRODUCT_VERSION == 2533)

/* For MT2523, the bt audio related feature generates lots of bt logs. */

/* buffer pool items */
#define LOG_POOL_ITEMS SYSLOG_QUEUE_LENGTH

/* maximum delay to wait for available log queue */
#define LOG_QUEUE_WAITING_TIME (0)

#endif

#define change_level_to_string(level) \
  ((level) - PRINT_LEVEL_DEBUG <= PRINT_LEVEL_ERROR) ? print_level_table[level] : "debug"

typedef enum {
    MESSAGE_TYPE_NORMAL_LOG,
    MESSAGE_TYPE_HEX_DUMP
} message_type_t;

typedef struct {
    const char *module_name;
    print_level_t print_level;
    const char *func_name;
    int line_number;
    uint32_t timestamp;
    char message[MAX_LOG_SIZE + 1]; //C string format
} normal_log_message_t;

typedef struct {
    const char *module_name;
    print_level_t print_level;
    const char *func_name;
    int line_number;
    uint32_t timestamp;
    char message[MAX_DUMP_MESSAGE_SIZE + 1]; //C string format
    uint32_t data_length;
    char data[MAX_DUMP_DATA_SIZE];
    char *additional_data;
} hex_dump_message_t;


/* log message definition */
typedef struct {
    message_type_t message_type;
    union {
        normal_log_message_t normal_log_message;
        hex_dump_message_t   hex_dump_message;
    } u;
} log_message_t;

typedef struct {
    uint32_t occupied;
    uint8_t  buffer[sizeof(log_message_t)];
} syslog_buffer_t;

/* syslog task handle */
static xTaskHandle syslog_task_handle;

/* syslog buffer pool */
static syslog_buffer_t syslog_buffer_pool[LOG_POOL_ITEMS];

/* log queue handle */
static xQueueHandle g_log_queue = NULL;

#define SYSLOG_UART_INIT_DONE    0x01
#define SYSLOG_GPT_INIT_DONE     0x02
#define SYSLOG_NVDM_INIT_DONE    0x04
#define SYSLOG_UART_DMA_MODE     0x08
#define SYSLOG_PORT_OPEN_DONE    0x10
#define SYSLOG_EXCEPTION_HAPPEN  0x20

static uint32_t syslog_init_time = 0;
static uint32_t syslog_init_done = 0;
static uint32_t syslog_task_ready = false;
static uint32_t syslog_drop_count = 0;

/* create common module for unspecified module to use */
log_create_module(common, PRINT_LEVEL_INFO);

static syslog_config_t syslog_config;

static const char *log_switch_table[]  = { "on", "off" };

static const char *print_level_table[] = { "debug", "info", "warning", "error" };

static hal_uart_port_t g_log_uart_port;

#if defined(MTK_PORT_SERVICE_ENABLE)
static serial_port_dev_t g_log_port;
static serial_port_handle_t g_log_port_handle;
static SemaphoreHandle_t tx_sem, rx_sem;
void syslog_serial_port_switch(serial_port_dev_t device);
#endif

#if  defined ( __GNUC__ )
#ifndef __weak
#define __weak   __attribute__((weak))
#endif /* __weak */
#endif /* __GNUC__ */

/*******************************************************************************
 *  syslog uart port management
 ******************************************************************************/
#if defined(MTK_SAVE_LOG_TO_FLASH_ENABLE)

#include "offline_dump.h"
#if defined(MTK_NVDM_ENABLE)
#include "nvdm.h"
#endif

#define SYSLOG_DUMP_BUFFER_SIZE OFFLINE_REGION_SYSLOG_CELL_VALID_SIZE

#define SYSLOG_NVDM_DUMP_GROUP_NAME "DUMP"
#define SYSLOG_NVDM_DUMP_DATA_NAME "SYSLOG"

typedef struct {
    bool log_to_flash;
    bool always_save;
} log_dump_data_t;

static bool g_syslog_dump_buffer_wrap;
static uint8_t *g_syslog_dump_buffer_pos;
static uint8_t g_syslog_dump_buffer[SYSLOG_DUMP_BUFFER_SIZE];
/*
 * true:  dump syslog to flash only when the user call log_trigger_write_to_flash()
 * false: always dump syslog to flash
 */
static volatile bool g_syslog_dump_is_passive = true;

/*
 * true:  log over flash
 * false: log over console
 */
static volatile bool g_syslog_log_to_flash = false;

/* This function is used to switch syslog between console and flash. */
bool log_path_switch(bool log_to_flash, bool save_to_nvdm, bool always_dump)
{
    if (log_to_flash == false) {
        g_syslog_log_to_flash = false;
    } else {
        g_syslog_log_to_flash = true;
        if (always_dump == true) {
            g_syslog_dump_is_passive = false;
        } else {
            g_syslog_dump_is_passive = true;
        }
    }
    if (save_to_nvdm == true) {
#if defined(MTK_NVDM_ENABLE)
        log_dump_data_t log_dump_data;

        log_dump_data.log_to_flash = g_syslog_log_to_flash;
        log_dump_data.always_save = g_syslog_dump_is_passive;
        nvdm_write_data_item(SYSLOG_NVDM_DUMP_GROUP_NAME, SYSLOG_NVDM_DUMP_DATA_NAME, NVDM_DATA_ITEM_TYPE_RAW_DATA, (const uint8_t *)&log_dump_data, sizeof(log_dump_data));
#endif
    }

    return true;
}

bool log_write_to_dump_buffer(char *buf, int32_t len)
{
    bool syslog_dump_is_passive;
    uint32_t curr_len, cpu_irq_mask;
    uint32_t left_size, curr_size, curr_addr;

    if (g_syslog_log_to_flash == false) {
        return false;
    }

    if (!(syslog_init_done & SYSLOG_NVDM_INIT_DONE)) {
        /* Before log_uart_init(), always write to buffer */
        syslog_dump_is_passive = true;
    } else {
        if (syslog_init_done & SYSLOG_EXCEPTION_HAPPEN) {
            /* After exception happen, always write to flash */
            syslog_dump_is_passive = false;
        } else {
            /* After log_init(), choose passive or active write to flash. */
            syslog_dump_is_passive = g_syslog_dump_is_passive;
        }
    }

    if (syslog_dump_is_passive == true) {
        hal_nvic_save_and_set_interrupt_mask(&cpu_irq_mask);

        if ((((uint32_t)g_syslog_dump_buffer + SYSLOG_DUMP_BUFFER_SIZE) - (uint32_t)g_syslog_dump_buffer_pos) <= len) {
            memcpy(g_syslog_dump_buffer_pos, buf, ((uint32_t)g_syslog_dump_buffer + SYSLOG_DUMP_BUFFER_SIZE) - (uint32_t)g_syslog_dump_buffer_pos);
            curr_len = ((uint32_t)g_syslog_dump_buffer + SYSLOG_DUMP_BUFFER_SIZE) - (uint32_t)g_syslog_dump_buffer_pos;
            memcpy(g_syslog_dump_buffer, &buf[curr_len], len - curr_len);
            g_syslog_dump_buffer_pos = g_syslog_dump_buffer + len - curr_len;
            if (g_syslog_dump_buffer_wrap == false) {
                g_syslog_dump_buffer_wrap = true;
            }
        } else {
            memcpy(g_syslog_dump_buffer_pos, buf, len);
            g_syslog_dump_buffer_pos += len;
        }

        hal_nvic_restore_interrupt_mask(cpu_irq_mask);
    } else {
        if (g_syslog_dump_buffer_wrap == true) {
            log_trigger_write_to_flash();
        }

        left_size = ((uint32_t)g_syslog_dump_buffer + SYSLOG_DUMP_BUFFER_SIZE) - (uint32_t)g_syslog_dump_buffer_pos;
        if (left_size > len) {
            memcpy(g_syslog_dump_buffer_pos, buf, len);
            g_syslog_dump_buffer_pos += len;
            return len;
        }

        curr_size = left_size;
        left_size = len - curr_size;
        memcpy(g_syslog_dump_buffer_pos, buf, curr_size);
        offline_dump_region_alloc(OFFLINE_REGION_SYSLOG, &curr_addr);
        offline_dump_region_write(OFFLINE_REGION_SYSLOG, curr_addr, g_syslog_dump_buffer, SYSLOG_DUMP_BUFFER_SIZE);
        offline_dump_region_write_end(OFFLINE_REGION_SYSLOG, SYSLOG_DUMP_BUFFER_SIZE);

        if (left_size) {
            memcpy(g_syslog_dump_buffer, &buf[curr_size], left_size);
        }

        g_syslog_dump_buffer_pos = g_syslog_dump_buffer + left_size;
    }

    return true;
}

bool log_trigger_write_to_flash(void)
{
    uint32_t curr_addr, total_length, cpu_irq_mask;

    if (!(syslog_init_done & SYSLOG_NVDM_INIT_DONE)) {
        return false;
    }

    if ((!(syslog_init_done & SYSLOG_EXCEPTION_HAPPEN)) &&
            (g_syslog_dump_is_passive == false)) {
        return false;
    }

    if (g_syslog_log_to_flash == false) {
        return false;
    }

    hal_nvic_save_and_set_interrupt_mask(&cpu_irq_mask);

    offline_dump_region_alloc(OFFLINE_REGION_SYSLOG, &curr_addr);

    if (g_syslog_dump_buffer_wrap == true) {
        offline_dump_region_write(OFFLINE_REGION_SYSLOG, curr_addr,
                                  g_syslog_dump_buffer_pos,
                                  ((uint32_t)g_syslog_dump_buffer + SYSLOG_DUMP_BUFFER_SIZE) - (uint32_t)g_syslog_dump_buffer_pos);
        curr_addr += ((uint32_t)g_syslog_dump_buffer + SYSLOG_DUMP_BUFFER_SIZE) - (uint32_t)g_syslog_dump_buffer_pos;
        offline_dump_region_write(OFFLINE_REGION_SYSLOG, curr_addr,
                                  g_syslog_dump_buffer,
                                  (uint32_t)g_syslog_dump_buffer_pos - (uint32_t)g_syslog_dump_buffer);
        total_length = SYSLOG_DUMP_BUFFER_SIZE;
    } else {
        offline_dump_region_write(OFFLINE_REGION_SYSLOG, curr_addr,
                                  g_syslog_dump_buffer,
                                  (uint32_t)g_syslog_dump_buffer_pos - (uint32_t)g_syslog_dump_buffer);
        total_length = (uint32_t)g_syslog_dump_buffer_pos - (uint32_t)g_syslog_dump_buffer;
    }

    offline_dump_region_write_end(OFFLINE_REGION_SYSLOG, total_length);

    /* reset the status of temp buffer */
    g_syslog_dump_buffer_wrap = false;
    g_syslog_dump_buffer_pos = g_syslog_dump_buffer;

    hal_nvic_restore_interrupt_mask(cpu_irq_mask);

    return true;
}
#endif

hal_uart_status_t log_uart_init(hal_uart_port_t port)
{
    hal_uart_status_t ret;
    hal_uart_config_t uart_config;

#if defined(MTK_SAVE_LOG_TO_FLASH_ENABLE)
    g_syslog_dump_buffer_wrap = false;
    g_syslog_dump_buffer_pos = g_syslog_dump_buffer;
#endif

    /* Configure UART PORT */
    uart_config.baudrate = HAL_UART_BAUDRATE_115200;
    uart_config.parity = HAL_UART_PARITY_NONE;
    uart_config.stop_bit = HAL_UART_STOP_BIT_1;
    uart_config.word_length = HAL_UART_WORD_LENGTH_8;

    ret = hal_uart_init(port, &uart_config);

    g_log_uart_port = port;
    syslog_init_done |= SYSLOG_UART_INIT_DONE;

    return ret;
}

void log_putchar(char byte)
{
    hal_uart_put_char(g_log_uart_port, byte);
}

#if defined (MTK_PORT_SERVICE_ENABLE)

__weak serial_port_status_t serial_port_open(serial_port_dev_t device,
        serial_port_open_para_t *para,
        serial_port_handle_t *handle)
{
    configASSERT(device - SERIAL_PORT_DEV_UART_0 <= SERIAL_PORT_DEV_UART_3);
    /* If port service feature is not enabled, use the one passed in log_uart_init. */
    return (log_uart_init(g_log_uart_port) == HAL_UART_STATUS_OK) ? SERIAL_PORT_STATUS_OK : SERIAL_PORT_STATUS_FAIL;
}

__weak serial_port_status_t serial_port_close(serial_port_handle_t handle)
{
    return SERIAL_PORT_STATUS_OK;
}

__weak serial_port_status_t serial_port_config_read_dev_number(const char *user_name, serial_port_dev_t *device)
{
    return SERIAL_PORT_STATUS_OK;
}

__weak serial_port_status_t serial_port_config_write_dev_number(const char *user_name, serial_port_dev_t device)
{
    return SERIAL_PORT_STATUS_OK;
}

__weak serial_port_status_t serial_port_control(serial_port_handle_t handle,
        serial_port_ctrl_cmd_t command,
        serial_port_ctrl_para_t *para)
{
    return SERIAL_PORT_STATUS_OK;
}

static void log_switch_to_dma_mode(void)
{
    syslog_init_done |= SYSLOG_UART_DMA_MODE | SYSLOG_PORT_OPEN_DONE;
}


int log_write(char *buf, int len)
{
    serial_port_status_t status;
    serial_port_ctrl_para_t send_data;
    int ret = 0;
    int i;

#ifdef MTK_SAVE_LOG_TO_FLASH_ENABLE
    if (log_write_to_dump_buffer(buf, len) == true) {
        return len;
    }
#endif

    if (syslog_init_done & SYSLOG_UART_INIT_DONE)  {
        send_data.serial_port_write_data.data = (uint8_t *)buf;
        send_data.serial_port_write_data.size = len;

        if (syslog_init_done & SYSLOG_UART_DMA_MODE) {
            while (1) {
                status = serial_port_control(g_log_port_handle, SERIAL_PORT_CMD_WRITE_DATA, &send_data);
                if (status == SERIAL_PORT_STATUS_OK) {
                    send_data.serial_port_write_data.size -= send_data.serial_port_write_data.ret_size;
                    send_data.serial_port_write_data.data += send_data.serial_port_write_data.ret_size;
                }
                if (send_data.serial_port_write_data.size == 0) {
                    break;
                }
                if ((xTaskGetSchedulerState() != taskSCHEDULER_SUSPENDED) && (hal_nvic_query_exception_number() == 0)) {
                    xSemaphoreTake(tx_sem, 1000);
                }
            }
            ret = (send_data.serial_port_write_data.size == 0) ? len : 0;
        } else {
            if (syslog_init_done & SYSLOG_PORT_OPEN_DONE) {
                status = serial_port_control(g_log_port_handle, SERIAL_PORT_CMD_WRITE_DATA_BLOCKING, &send_data);
                ret = (status == SERIAL_PORT_STATUS_OK) ? len : 0;
            } else {
                for (i = 0; i < len; i++) {
                    log_putchar(*buf++);
                }
                ret = len;
            }
        }
    }
    return ret;
}

static void syslog_port_service_callback(serial_port_dev_t device, serial_port_callback_event_t event, void *parameter)
{
    BaseType_t xHigherPriorityTaskWoken;

    switch (event) {
        case SERIAL_PORT_EVENT_READY_TO_READ:
            xSemaphoreGiveFromISR(rx_sem, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
            break;

        case SERIAL_PORT_EVENT_READY_TO_WRITE:
            xSemaphoreGiveFromISR(tx_sem, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
            break;

        case SERIAL_PORT_EVENT_USB_CONNECTION:
            if (g_log_port == device) {
                log_switch_to_dma_mode();
            }
            break;

        case SERIAL_PORT_EVENT_USB_DISCONNECTION:
            if (g_log_port == device) {
                g_log_port = (serial_port_dev_t)g_log_uart_port;
                syslog_serial_port_switch(g_log_port);
            }
            break;

        default:
            break;
    }
}

#endif /* MTK_PORT_SERVICE_ENABLE */

#define LOG_TS(p) (((log_message_t*)(p)->buffer)->u.normal_log_message.timestamp)

static syslog_buffer_t *sort_syslog_buffer(syslog_buffer_t *pList)
{
    syslog_buffer_t *pHead = NULL, *pCurr, *p;

    if (pList == NULL || pList->occupied == 0) {
        return pList;
    }

    while (pList != NULL) {
        pCurr = pList;
        pList = (syslog_buffer_t *)(pList->occupied);
        if ((pHead == NULL) || LOG_TS(pCurr) < LOG_TS(pHead)) {
            pCurr->occupied = (uint32_t)pHead;
            pHead = pCurr;
        } else {
            p = pHead;
            while (p != NULL) {
                if ((p->occupied == 0) ||
                        LOG_TS(pCurr) < LOG_TS((syslog_buffer_t *)(p->occupied))) {
                    pCurr->occupied = p->occupied;
                    p->occupied = (uint32_t)pCurr;
                    break;
                }
                p = (syslog_buffer_t *)(p->occupied);
            }
        }
    }
    return pHead;
}

void process_log_message(const log_message_t *log_message);

static void syslog_buffer_dump(void)
{
    uint32_t index;
    syslog_buffer_t *pHead = NULL, *pCurr;

    /* create a link list of syslog buffer(s) in use now */
    for (index = 0; index < LOG_POOL_ITEMS; index++) {
        /* reuse the 'occupied' field as a pointer */
        if (syslog_buffer_pool[index].occupied == 1) {
            syslog_buffer_pool[index].occupied = (uint32_t)pHead;
            pHead = &syslog_buffer_pool[index];
        }
    }

    /* sort the log by time */
    pHead = sort_syslog_buffer(pHead);

    printf("\n\r>>> dump syslog buffer\n\r");

    /* print the sorted log */
    while (pHead) {
        pCurr = pHead;
        pHead = (syslog_buffer_t *)(pHead->occupied);
        pCurr->occupied = 1;
        process_log_message((log_message_t *)(pCurr->buffer));
    }
}

static void syslog_exception_cb(void)
{
#if defined(MTK_PORT_SERVICE_ENABLE)
    /* switch to polling mode */
    syslog_init_done &= ~(SYSLOG_UART_DMA_MODE);
#endif
    syslog_task_ready = false;
    syslog_init_done |= SYSLOG_EXCEPTION_HAPPEN;
    syslog_buffer_dump();
#if defined(MTK_SAVE_LOG_TO_FLASH_ENABLE)
    log_trigger_write_to_flash();
#endif
}

/*******************************************************************************
 *  syslog memory pool management
 ******************************************************************************/

#define DEBUG_SYSLOG_BLOCK_ISSUE 0

static syslog_buffer_t *syslog_free_list;

#define SYSLOG_BUFFER_SANITY_CHECK 0

#if (SYSLOG_BUFFER_SANITY_CHECK == 1)

static void syslog_buffer_sanity_check(syslog_buffer_t *p)
{
    configASSERT (p >= syslog_buffer_pool &&
                  p <= syslog_buffer_pool + LOG_POOL_ITEMS - 1 &&
                  (((uint32_t)p - (uint32_t)syslog_buffer_pool) % sizeof(syslog_buffer_t)) == 0);
}

#else

#define syslog_buffer_sanity_check(p)

#endif

void buffer_pool_init()
{
    uint32_t i;

    syslog_free_list = syslog_buffer_pool;

    syslog_buffer_pool[LOG_POOL_ITEMS - 1].occupied = 0;

    for (i = 0; i < LOG_POOL_ITEMS - 1; i++) {
        syslog_buffer_pool[i].occupied = (uint32_t) & (syslog_buffer_pool[i + 1]);
    }
}

syslog_buffer_t *buffer_pool_alloc(void)
{
    UBaseType_t uxSavedInterruptStatus;
    syslog_buffer_t *syslog_buffer;

    uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
    if (syslog_free_list) {
        syslog_buffer = syslog_free_list;
        syslog_free_list = (syslog_buffer_t *)(syslog_free_list->occupied);
        syslog_buffer->occupied = 1;
        syslog_buffer_sanity_check(syslog_buffer);
    } else {
        syslog_buffer = NULL;
        syslog_drop_count++;
    }
    portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );

#if (DEBUG_SYSLOG_BLOCK_ISSUE == 1)
    configASSERT(syslog_drop_count < 3000);
#endif

    return syslog_buffer;
}

void buffer_pool_release(syslog_buffer_t *syslog_buffer)
{
    UBaseType_t uxSavedInterruptStatus;
    log_message_t *log_message = (log_message_t *)syslog_buffer->buffer;

    uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
    syslog_buffer_sanity_check(syslog_buffer);
    syslog_buffer->occupied = (uint32_t)syslog_free_list;
    syslog_free_list = syslog_buffer;
    if ((log_message->message_type == MESSAGE_TYPE_HEX_DUMP)
            && (log_message->u.hex_dump_message.additional_data) ) {
        vPortFree(log_message->u.hex_dump_message.additional_data);
    }
    portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );
}

static unsigned int get_current_time_in_ms(void)
{
    uint32_t count = 0;
    uint64_t count64 = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &count);
    count64 = ((uint64_t)count) * 1000 / 32768;
    return (unsigned int)count64;
}

/*******************************************************************************
 *  syslog queue management
 ******************************************************************************/
void log_queue_create(void)
{
    g_log_queue = xQueueCreate(SYSLOG_QUEUE_LENGTH, sizeof(syslog_buffer_t *));
    if (NULL == g_log_queue) {
        printf("log_queue_create: create log queue failed.\r\n");
    }
}

void log_queue_send(syslog_buffer_t **log_buffer)
{
    if (hal_nvic_query_exception_number() != 0) {
        BaseType_t xTaskWokenByPost = pdFALSE;
        if (xQueueSendFromISR(g_log_queue, log_buffer, &xTaskWokenByPost) ==  errQUEUE_FULL ) {
            buffer_pool_release(*log_buffer);
        } else {
            portYIELD_FROM_ISR(xTaskWokenByPost);
        }
    } else {
        xQueueSend(g_log_queue, log_buffer, LOG_QUEUE_WAITING_TIME);
    }
}

int log_queue_receive(syslog_buffer_t **log_buffer)
{
    BaseType_t ret_val = xQueueReceive(g_log_queue, log_buffer, portMAX_DELAY);
    return (pdPASS == ret_val) ? 0 : -1;
}

/*******************************************************************************
 *  syslog implementation
 ******************************************************************************/
void print_normal_log(const normal_log_message_t *normal_log_message)
{
    printf("[T: %u M: %s C: %s F: %s L: %d]: ",
           (unsigned int)normal_log_message->timestamp,
           normal_log_message->module_name,
           change_level_to_string(normal_log_message->print_level),
           normal_log_message->func_name,
           normal_log_message->line_number);
    printf("%s\r\n", (char *)normal_log_message->message);
}

void hex_dump_data(const char *data, int length)
{
    int index = 0;
    for (index = 0; index < length; index++) {
        printf("%02X", (int)(data[index]));
        if ((index + 1) % 16 == 0) {
            printf("\r\n");
            continue;
        }
        if (index + 1 != length) {
            printf(" ");
        }
    }
    if (0 != index && 0 != index % 16) {
        printf("\r\n");//add one more blank line
    }
}

void print_hex_dump(const hex_dump_message_t *hex_dump_message)
{
    printf("[T: %u M: %s C: %s F: %s L: %d]: ",
           (unsigned int)hex_dump_message->timestamp,
           hex_dump_message->module_name,
           change_level_to_string(hex_dump_message->print_level),
           hex_dump_message->func_name,
           hex_dump_message->line_number);
    printf("%s\r\n", (char *)hex_dump_message->message);

    if (hex_dump_message->data_length > MAX_DUMP_DATA_SIZE) {
        hex_dump_data(hex_dump_message->data, MAX_DUMP_DATA_SIZE);
        hex_dump_data(hex_dump_message->additional_data, hex_dump_message->data_length - MAX_DUMP_DATA_SIZE);
    } else {
        hex_dump_data(hex_dump_message->data, hex_dump_message->data_length);
    }
}

void process_log_message(const log_message_t *log_message)
{
    if (syslog_drop_count) {
        UBaseType_t uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();;
        printf("\t\t>>> log drop count = %u\r\n\n", (unsigned int)syslog_drop_count);
        syslog_drop_count = 0;
        portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );
    }

    if (MESSAGE_TYPE_NORMAL_LOG == log_message->message_type) {
        print_normal_log(&log_message->u.normal_log_message);
    } else {
        print_hex_dump(&log_message->u.hex_dump_message);
    }
}

#if defined(MTK_PORT_SERVICE_ENABLE)

static void syslog_serial_port_open(TimerHandle_t timer)
{
    serial_port_open_para_t serial_port_open_para;
    serial_port_status_t status;

    portENTER_CRITICAL();
    if (g_log_port == (serial_port_dev_t)g_log_uart_port) {
        hal_uart_deinit(g_log_uart_port);
    }
    serial_port_open_para.callback = syslog_port_service_callback;
    status = serial_port_open(g_log_port, &serial_port_open_para, &g_log_port_handle);
    portEXIT_CRITICAL();

    switch (status) {
        case SERIAL_PORT_STATUS_DEV_NOT_READY:
            break;

        case SERIAL_PORT_STATUS_OK:
            /* port service ready */
            log_switch_to_dma_mode();
            break;

        case SERIAL_PORT_STATUS_BUSY:
        case SERIAL_PORT_STATUS_UNINITIALIZED:
        case SERIAL_PORT_STATUS_UNSUPPORTED:
        case SERIAL_PORT_STATUS_INVALID_DEVICE:
        case SERIAL_PORT_STATUS_INVALID_PARAMETER:
        case SERIAL_PORT_STATUS_FAIL:
            /* not recoverable */
            break;
    }

    LOG_I(common, "open port status = %d", status);
}

void syslog_serial_port_switch(serial_port_dev_t device)
{
    syslog_init_done &= ~(SYSLOG_UART_DMA_MODE | SYSLOG_PORT_OPEN_DONE);
    serial_port_close(g_log_port_handle);
    if (g_log_port == (serial_port_dev_t)g_log_uart_port) {
        log_uart_init(g_log_uart_port);
    }
    g_log_port = device;
    syslog_serial_port_open(NULL);
}

#endif /* MTK_PORT_SERVICE_ENABLE */

void syslog_task_entry(void *args)
{
#if defined(MTK_PORT_SERVICE_ENABLE)
    tx_sem = xSemaphoreCreateBinary();
    rx_sem = xSemaphoreCreateBinary();
    /* The port setting is written during project initialization. Syslog reads the port setting. */
    if (serial_port_config_read_dev_number("syslog", &g_log_port) != SERIAL_PORT_STATUS_OK) {
        g_log_port = (serial_port_dev_t)g_log_uart_port;
    }
    LOG_I(common, "syslog port = %d", g_log_port);
    syslog_serial_port_open(NULL);
#endif

    syslog_task_ready = true;
    vTaskPrioritySet(NULL, TASK_PRIORITY_SYSLOG);

    for (;;) {
        syslog_buffer_t *log_buffer = NULL;
        if (0 == log_queue_receive(&log_buffer)) {
            process_log_message((log_message_t *)(log_buffer->buffer));
            buffer_pool_release(log_buffer);
        }
    }
}

void log_init(syslog_save_fn         save,
              syslog_load_fn         load,
              log_control_block_t  **entries)
{
    exception_config_type exception_config;

    if (syslog_init_done & SYSLOG_GPT_INIT_DONE) {
        /* avoid initializing syslog twice */
        return;
    }

    syslog_init_done |= SYSLOG_UART_INIT_DONE; /* for backward compatible */
    buffer_pool_init();
    log_queue_create();
    syslog_task_ready = false;

    exception_config.init_cb = syslog_exception_cb;
    exception_config.dump_cb = NULL;
    exception_register_callbacks(&exception_config);


    if (pdPASS != xTaskCreate(syslog_task_entry,
                              SYSLOG_TASK_NAME,
                              SYSLOG_TASK_STACKSIZE,
                              NULL,
                              TASK_PRIORITY_HARD_REALTIME,
                              &syslog_task_handle)) {
        printf("log_init: create syslog task failed.\r\n");
    }

    syslog_init_time = get_current_time_in_ms();
    syslog_init_done |= SYSLOG_GPT_INIT_DONE;
    (void)syslog_init_time;

    syslog_config.save_fn = save;
    syslog_config.filters = entries;

    if (load) {
        /* overwrite the rom code definition if any */
        if (load(&syslog_config) != 0) {
            LOG_E(common, "load syslog config failed\n");
        }
    }
    syslog_init_done |= SYSLOG_NVDM_INIT_DONE;

#if defined(MTK_SAVE_LOG_TO_FLASH_ENABLE)
#if defined(MTK_NVDM_ENABLE)
    {
        log_dump_data_t log_dump_data;
        nvdm_status_t status;
        uint32_t dump_size;

        dump_size = sizeof(log_dump_data_t);
        status = nvdm_read_data_item(SYSLOG_NVDM_DUMP_GROUP_NAME, SYSLOG_NVDM_DUMP_DATA_NAME, (uint8_t *)&log_dump_data, &dump_size);
        if (status == NVDM_STATUS_OK) {
            g_syslog_log_to_flash = log_dump_data.log_to_flash;
            g_syslog_dump_is_passive = log_dump_data.always_save;
        } else {
            g_syslog_log_to_flash = true;
            log_dump_data.log_to_flash = g_syslog_log_to_flash;
            g_syslog_dump_is_passive = true;
            log_dump_data.always_save = g_syslog_dump_is_passive;
            nvdm_write_data_item(SYSLOG_NVDM_DUMP_GROUP_NAME, SYSLOG_NVDM_DUMP_DATA_NAME, NVDM_DATA_ITEM_TYPE_RAW_DATA, (const uint8_t *)&log_dump_data, sizeof(log_dump_data));
        }

        offline_dump_region_init();
    }
#endif
#endif
}

void vprint_module_log(void *handle,
                       const char *func,
                       int line,
                       print_level_t level,
                       const char *message,
                       va_list list)
{
    log_control_block_t *context = (log_control_block_t *)handle;
    if ((context->log_switch == DEBUG_LOG_ON)
            && (context->print_level <= level)) {

        if (!syslog_task_ready
                || (xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED
                    && (hal_nvic_query_exception_number() == 0))) {
            if (syslog_init_done > 0) {
                UBaseType_t uxSavedInterruptStatus;
                uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
                if (syslog_init_done > SYSLOG_UART_INIT_DONE) {
                    /* gpt init done, time can be printed  */
                    printf("[T: %u M: %s C: %s F: %s L: %d]: ",
                           get_current_time_in_ms(),
                           context->module_name,
                           change_level_to_string(level),
                           func,
                           line);
                }
                vprintf(message, list);
                printf("\r\n");
                portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );
            }
        } else {
            /* syslog task is ready, send to queue to print */
            syslog_buffer_t *syslog_buffer = buffer_pool_alloc();
            log_message_t *log_message;
            normal_log_message_t *normal_log_message;

            if (NULL == syslog_buffer) {
                return;
            }
            log_message = (log_message_t *)syslog_buffer->buffer;
            log_message->message_type = MESSAGE_TYPE_NORMAL_LOG;
            normal_log_message = &log_message->u.normal_log_message;
            normal_log_message->module_name = context->module_name;
            normal_log_message->print_level = level;
            normal_log_message->func_name = func;
            normal_log_message->line_number = line;
            normal_log_message->timestamp = get_current_time_in_ms();

            /* This step might fail when log message is too long,
               but syslog will do it's best to print the log */
            (void)vsnprintf(normal_log_message->message, MAX_LOG_SIZE, message, list);
            log_queue_send(&syslog_buffer);
        }
    }
}

void print_module_log(void *handle,
                      const char *func,
                      int line,
                      print_level_t level,
                      const char *message, ...)
{
    va_list ap;
    va_start(ap, message);
    vprint_module_log(handle, func, line, level, message, ap);
    va_end(ap);
}

void vdump_module_buffer(void *handle,
                         const char *func,
                         int line,
                         print_level_t level,
                         const void *data,
                         int length,
                         const char *message,
                         va_list list)
{
    log_control_block_t *context = (log_control_block_t *)handle;
    int additional_length;

    if ((context->log_switch == DEBUG_LOG_ON)
            && (context->print_level <= level)) {

        if (!syslog_task_ready
                || (xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED
                    && (hal_nvic_query_exception_number() == 0))) {
            if (syslog_init_done > 0) {
                UBaseType_t uxSavedInterruptStatus;
                uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
                if (syslog_init_done > SYSLOG_UART_INIT_DONE) {
                    /* gpt init done, time can be printed  */
                    printf("[T: %u M: %s C: %s F: %s L: %d]: ",
                           get_current_time_in_ms(),
                           context->module_name,
                           change_level_to_string(level),
                           func,
                           line);
                }
                vprintf(message, list);
                printf("\r\n");
                hex_dump_data((char *)data, length);
                portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );
            }
        } else {
            /* syslog task is ready, send to queue to print */
            syslog_buffer_t *syslog_buffer = buffer_pool_alloc();
            log_message_t *log_message;
            hex_dump_message_t *hex_dump_message;

            if (NULL == syslog_buffer) {
                return;
            }

            log_message = (log_message_t *)syslog_buffer->buffer;
            log_message->message_type = MESSAGE_TYPE_HEX_DUMP;
            hex_dump_message = &log_message->u.hex_dump_message;
            hex_dump_message->module_name = context->module_name;
            hex_dump_message->print_level = level;
            hex_dump_message->func_name = func;
            hex_dump_message->line_number = line;
            hex_dump_message->timestamp = get_current_time_in_ms();

            /* This step might fail when log message is too long,
               but syslog will do it's best to print the log */
            (void)vsnprintf(hex_dump_message->message, MAX_DUMP_MESSAGE_SIZE, message, list);

            if (length > MAX_HEXDUMP_LENGTH) {
                length = MAX_HEXDUMP_LENGTH;
            }

            additional_length = 0;
            if (length > MAX_DUMP_DATA_SIZE) {
                if (hal_nvic_query_exception_number() == 0) {
                    additional_length = length - MAX_DUMP_DATA_SIZE;
                }
                length = MAX_DUMP_DATA_SIZE;
            }

            memcpy(hex_dump_message->data, data, length);
            hex_dump_message->data_length = length;

            hex_dump_message->additional_data = NULL;
            if (additional_length) {
                if ((hex_dump_message->additional_data = (char *) pvPortMalloc(additional_length)) != NULL) {
                    memcpy(hex_dump_message->additional_data, (char *)data + length, additional_length);
                    hex_dump_message->data_length += additional_length;
                }
            }

            log_queue_send(&syslog_buffer);
        }
    }
}

void dump_module_buffer(void *handle,
                        const char *func,
                        int line,
                        print_level_t level,
                        const void *data,
                        int length,
                        const char *message, ...)
{
    va_list ap;
    va_start(ap, message);
    vdump_module_buffer(handle, func, line, level, data, length, message, ap);
    va_end(ap);
}

void log_print_msgid(void *handle,
                        print_level_t level,
                        const char *message,
                        uint32_t arg_cnt,
                        va_list list)
{
    (void)arg_cnt;

    vprint_module_log(handle, " ", 0, level, message, list);
}

void syslog_get_config(syslog_config_t *config)
{
    *config = syslog_config;
}

/**
 * @param p  the string pointer.
 * @param pp the pointer be assigned the start of next string.
 *
 * @retval 0 No valid string found.
 * @return the length of the found string.
 */
static int syslog_get_a_string(const char *p, char **pp)
{
    const char *start;

    while (*p == ' ') {
        p++;
    }

    start = p;

    /* first non-space is '\0' */
    if (*p == '\0') {
        *pp = NULL;
        return 0;
    }

    *pp = (char *)p;

    while (*p != ' ' && *p != '\0') {
        p++;
    }

    return (p - start);
}

static int syslog_get_index_from_module_name(log_control_block_t **filters, const char *module_name)
{
    int i = 0;

    if (filters == NULL) {
        return -1;
    }

    while (filters[i]) {
        if (!strcmp(filters[i]->module_name, module_name)) {
            return i;
        }
        i++;
    }

    /* no match tag name found */
    return -1;
}

int syslog_set_filter(char *module_name, char *log_switch_str, char *print_level_str, int save)
{
    int         i;
    int         log_switch;
    int         print_level;

    log_switch = str_to_log_switch(log_switch_str);
    if (log_switch < 0) {
        return -1;
    }

    print_level = str_to_print_level(print_level_str);
    if (print_level < 0) {
        return -1;
    }

    if (strcmp(module_name, "*") == 0) {

        if (syslog_config.filters == NULL) {
            return -1;
        }

        i = 0;
        while (syslog_config.filters[i]) {
            syslog_config.filters[i]->log_switch = (log_switch_t)log_switch;
            syslog_config.filters[i]->print_level = (print_level_t)print_level;
            i++;
        }

    } else {

        i = syslog_get_index_from_module_name(syslog_config.filters, module_name);
        /* no match tag name found */
        if (i < 0) {
            return -1;
        }
        syslog_config.filters[i]->log_switch = (log_switch_t)log_switch;
        syslog_config.filters[i]->print_level = (print_level_t)print_level;

    }

    if (save) {
        syslog_config.save_fn(&syslog_config);
    }

    return 0;
}

int syslog_at_set_filter(char *module_name, int log_switch, int print_level, int save)
{
    int         i;

    if (strcmp(module_name, "*") == 0) {

        if (syslog_config.filters == NULL) {
            return -1;
        }

        i = 0;
        while (syslog_config.filters[i]) {
            syslog_config.filters[i]->log_switch = (log_switch_t)log_switch;
            syslog_config.filters[i]->print_level = (print_level_t)print_level;
            i++;
        }

    } else {

        i = syslog_get_index_from_module_name(syslog_config.filters, module_name);
        /* no match tag name found */
        if (i < 0) {
            return -1;
        }
        syslog_config.filters[i]->log_switch = (log_switch_t)log_switch;
        syslog_config.filters[i]->print_level = (print_level_t)print_level;

    }

    if (save) {
        syslog_config.save_fn(&syslog_config);
    }

    return 0;
}

/**
 * Find name and filter spec pair and convert into filter.
 */
int syslog_convert_filter_str2val(log_control_block_t **filters, char *buff)
{
    int     pos = 0;
    int     i;
    char    *module_name;
    char    *log_switch_str;
    char    *print_level_str;
    bool    done = false;
    uint32_t    log_switch;
    uint32_t    print_level;
    uint8_t     type;

    while (!done) {

        /*
         * 1. find module_name
         */

        i = syslog_get_a_string(&buff[pos], &module_name);
        if (i == 0 || buff[pos + i] == '\0') {
            /* premature end */
            return -1;
        }

        buff[pos + i]  = '\0';
        pos           += i + 1;

        /*
         * 2. find log_switch
         */

        i = syslog_get_a_string(&buff[pos], &log_switch_str);
        if (i == 0) {
            /* premature end */
            return -2;
        }

        done = (buff[i + pos] == '\0');

        buff[pos + i]  = '\0';
        pos           += i + 1;

        /*
         * 3. find print_level
         */

        i = syslog_get_a_string(&buff[pos], &print_level_str);
        if (i == 0) {
            /* premature end */
            return -2;
        }

        done = (buff[i + pos] == '\0');

        buff[pos + i]  = '\0';
        pos           += i + 1;

        /* 4. check and set */

        i = syslog_get_index_from_module_name(filters, module_name);

        /* no match tag found */
        if (i < 0) {
            return -3;
        }

        log_switch = toi(log_switch_str, &type);

        /* not a valid number */
        if (type == TOI_ERR) {
            return -4;
        }

        /* handle out of range */
        log_switch = (log_switch > DEBUG_LOG_OFF) ? DEBUG_LOG_OFF : log_switch;

        print_level = toi(print_level_str, &type);

        /* not a valid number */
        if (type == TOI_ERR) {
            return -4;
        }

        /* handle out of range */
        print_level = (print_level > PRINT_LEVEL_ERROR) ? PRINT_LEVEL_ERROR : print_level;

        filters[i]->log_switch = (log_switch_t)log_switch;
        filters[i]->print_level = (print_level_t)print_level;
    }

    return 0;
}

int syslog_convert_filter_val2str(const log_control_block_t **filters, char *buff)
{
    int i   = 0;
    int pos = 0;
    int n;

    while (filters && filters[i]) {
        n = snprintf(&buff[pos], SYSLOG_FILTER_LEN - pos, "%s%s 0x%01x 0x%01x",
                     (i == 0) ? "" : " ",
                     filters[i]->module_name,
                     filters[i]->log_switch,
                     filters[i]->print_level);
        pos += n;
        i++;
    }

    configASSERT (pos < SYSLOG_FILTER_LEN);

    return pos;
}

const char *log_switch_to_str(log_switch_t log_switch)
{
    configASSERT (log_switch - DEBUG_LOG_ON <= DEBUG_LOG_OFF);
    return  log_switch_table[log_switch];
}

const char *print_level_to_str(print_level_t print_level)
{
    configASSERT (print_level - PRINT_LEVEL_DEBUG <= PRINT_LEVEL_ERROR);
    return  print_level_table[print_level];
}

int str_to_log_switch(const char *log_switch_str)
{
    int i;

    for (i = 0; i < sizeof(log_switch_table) / sizeof(char *); i++) {
        if (!strcmp(log_switch_str, log_switch_table[i])) {
            return i;
        }
    }
    return -1;
}

int str_to_print_level(const char *print_level_str)
{
    int i;

    for (i = 0; i < sizeof(print_level_table) / sizeof(char *); i++) {
        if (!strcmp(print_level_str, print_level_table[i])) {
            return i;
        }
    }
    return -1;
}

int __wrap_printf( const char *format, ... )
{
    va_list ap;
    int ret;
    UBaseType_t uxSavedInterruptStatus;

    if ( !syslog_init_done ) {
        return 0;
    }

    uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    va_start(ap, format);
    ret = vprintf(format, ap);
    va_end(ap);

    portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );

    return ret;
}

#else

#include "syslog.h"

#if defined(MTK_PORT_SERVICE_ENABLE)
#include "serial_port.h"
#endif

/* create common module for unspecified module to use */
log_create_module(common, PRINT_LEVEL_INFO);

static hal_uart_port_t g_log_uart_port;

hal_uart_status_t log_uart_init(hal_uart_port_t port)
{
    hal_uart_config_t uart_config;
    hal_uart_status_t ret;

    /* Configure UART PORT */
    uart_config.baudrate = HAL_UART_BAUDRATE_115200;
    uart_config.parity = HAL_UART_PARITY_NONE;
    uart_config.stop_bit = HAL_UART_STOP_BIT_1;
    uart_config.word_length = HAL_UART_WORD_LENGTH_8;

    g_log_uart_port = port;
    ret = hal_uart_init(port, &uart_config);

    return ret;
}

void log_putchar(char byte)
{
    hal_uart_put_char(g_log_uart_port, byte);
}

void print_module_log(void *handle,
                      const char *func,
                      int line,
                      print_level_t level,
                      const char *message, ...)
{
}

void dump_module_buffer(void *handle,
                        const char *func,
                        int line,
                        print_level_t level,
                        const void *data,
                        int length,
                        const char *message, ...)
{
}

void log_print_msgid(void *handle,
                        print_level_t level,
                        const char *message,
                        uint32_t arg_cnt,
                        va_list list)
{
}

void vprint_module_log(void *handle,
                       const char *func,
                       int line,
                       print_level_t level,
                       const char *message,
                       va_list list)
{
}

void vdump_module_buffer(void *handle,
                         const char *func,
                         int line,
                         print_level_t level,
                         const void *data,
                         int length,
                         const char *message,
                         va_list list)
{
}

#if defined (MTK_PORT_SERVICE_ENABLE)

int log_write(char *buf, int len)
{
    return 0;
}

void syslog_serial_port_switch(serial_port_dev_t device)
{
}

#endif /* MTK_PORT_SERVICE_ENABLE */

#endif /* MTK_DEBUG_LEVEL_NONE */

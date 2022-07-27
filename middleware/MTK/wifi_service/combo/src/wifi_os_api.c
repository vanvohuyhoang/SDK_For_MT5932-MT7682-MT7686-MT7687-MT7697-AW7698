/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "portmacro.h"
#include "queue.h"
#include <timers.h>
#include <string.h>

#include "wifi_os_api.h"

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/

os_queue_handle_t wifi_os_queue_create( uint32_t queue_len, uint32_t item_size)
{
    return (os_queue_handle_t) xQueueCreate(queue_len, item_size);
}


// if use queue set, must set configUSE_QUEUE_SETS
os_queue_handle_t wifi_os_queue_set_create( uint32_t queue_len)
{
#if ( configUSE_QUEUE_SETS == 1 )
    return (os_queue_handle_t) xQueueCreateSet(queue_len);
#else
    return NULL;
#endif
}

// if use queue set, must set configUSE_QUEUE_SETS
os_queue_handle_t wifi_os_queue_set_receive(os_queue_handle_t queue, struct timeval *wait_time)
{
#if ( configUSE_QUEUE_SETS == 1 )
    assert(queue);

    uint32_t ticks;

    if(wait_time == NULL){
        ticks = portMAX_DELAY;
    }else{
        ticks = (wait_time->tv_sec * 1000 + wait_time->tv_usec / 1000) / portTICK_PERIOD_MS;
    }
    return xQueueSelectFromSet(queue, ticks);
#else
    return NULL;
#endif
}


uint32_t wifi_os_queue_messages_waiting(const QueueHandle_t xQueue)
{
    return uxQueueMessagesWaiting(xQueue);
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
void wifi_os_queue_delete(os_queue_handle_t queue)
{
    vQueueDelete(queue);
}

void wifi_os_queue_reset(os_queue_handle_t queue)
{
    xQueueReset(queue);
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
uint32_t wifi_os_queue_get_space(const os_queue_handle_t queue )
{
    assert(queue);

    return (uint32_t)uxQueueSpacesAvailable(queue);
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
int32_t wifi_os_queue_receive(os_queue_handle_t queue, void *buf, struct timeval *wait_time)
{
    assert(queue);

    TickType_t ticks;

    if(wait_time == NULL){
        ticks = portMAX_DELAY;
    }else{
        ticks = (wait_time->tv_sec * 1000 + wait_time->tv_usec / 1000) / portTICK_PERIOD_MS;
    }

    if(pdTRUE == xQueueReceive(queue, buf, ticks))
        return 0;

    return -1;
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
int32_t wifi_os_queue_send(os_queue_handle_t queue, void *item, struct timeval *wait_time)
{
    assert(queue);

    TickType_t ticks;

    if(wait_time == NULL){
        ticks = portMAX_DELAY;
    }else{
        ticks = (wait_time->tv_sec * 1000 + wait_time->tv_usec / 1000) / portTICK_PERIOD_MS;
    }

    if(pdTRUE == xQueueSend(queue, item, ticks))
        return 0;

    return -1;
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
int32_t wifi_os_queue_send_front(os_queue_handle_t queue, void *item, struct timeval *wait_time)
{
    assert(queue);

    TickType_t ticks;

    if(wait_time == NULL){
        ticks = portMAX_DELAY;
    }else{
        ticks = (wait_time->tv_sec * 1000 + wait_time->tv_usec / 1000) / portTICK_PERIOD_MS;
    }

    if(pdTRUE == xQueueSendToFront(queue, item, ticks))
        return 0;

    return -1;
}

int32_t wifi_os_queue_send_back(os_queue_handle_t queue, void *item, struct timeval *wait_time)
{
    assert(queue);

    uint32_t ticks;

    if(wait_time == NULL){
        ticks = portMAX_DELAY;
    }else{
        ticks = (wait_time->tv_sec * 1000 + wait_time->tv_usec / 1000) / portTICK_PERIOD_MS;
    }

    if(pdTRUE == xQueueSendToBack(queue, item, ticks))
        return 0;

    return -1;
}


/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
int32_t wifi_os_queue_send_from_isr(os_queue_handle_t queue, void *item)
{
    assert(queue);

    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    if(pdPASS == xQueueSendToBackFromISR(queue, item, &xHigherPriorityTaskWoken))
        return 0;

    return -1;
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
int32_t wifi_os_queue_send_front_from_isr(os_queue_handle_t queue, void *item)
{
    assert(queue);

    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    if(pdPASS == xQueueSendToFrontFromISR(queue, item, &xHigherPriorityTaskWoken))
        return 0;

    return -1;
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
int32_t wifi_os_receive_from_isr(os_queue_handle_t queue, void *buf)
{
    assert(queue);

    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    if(pdPASS == xQueueReceiveFromISR( queue, buf, &xHigherPriorityTaskWoken))
        return 0;

    return -1;
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
os_semaphore_t wifi_os_semphr_create_mutex(void)
{
    return (os_semaphore_t) xSemaphoreCreateMutex();
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
os_semaphore_t wifi_os_semphr_create_binary(void)
{
    return (os_semaphore_t) xSemaphoreCreateBinary();
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
int32_t wifi_os_semphr_take(os_semaphore_t semphr, struct timeval *wait_time)
{
    assert(semphr);

    TickType_t ticks;

    if(wait_time == NULL){
        ticks = portMAX_DELAY;
    }else{
        ticks = (wait_time->tv_sec * 1000 + wait_time->tv_usec / 1000) / portTICK_PERIOD_MS;
    }

    if(pdTRUE == xSemaphoreTake((SemaphoreHandle_t)semphr, ticks))
        return 0;

    return -1;
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
int32_t wifi_os_semphr_give(os_semaphore_t semphr)
{
    assert(semphr);

    if(pdTRUE == xSemaphoreGive((SemaphoreHandle_t)semphr))
        return 0;

    return -1;
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
int32_t wifi_os_semphr_take_from_isr(os_semaphore_t semphr)
{
    assert(semphr);

    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    if(pdTRUE == xSemaphoreTakeFromISR((SemaphoreHandle_t)semphr, &xHigherPriorityTaskWoken))
        return 0;

    return -1;    
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
int32_t wifi_os_semphr_give_from_isr(os_semaphore_t semphr)
{
    assert(semphr);
    
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    if(pdTRUE == xSemaphoreGiveFromISR((SemaphoreHandle_t)semphr, &xHigherPriorityTaskWoken))
        return 0;

    return -1;
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
void wifi_os_semphr_delete(os_semaphore_t semphr)
{
    assert(semphr);
    vSemaphoreDelete((SemaphoreHandle_t)semphr);
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
os_tmr_t wifi_os_timer_init(const char *name, 
                               struct timeval *expires,
                               unsigned long auto_reload,
                               timer_handle_t handler,
                               void *arg)
{
    uint32_t ticks;

    if(expires == NULL){
        return NULL;
    }else{
        ticks = (expires->tv_sec * 1000 + expires->tv_usec / 1000) / portTICK_PERIOD_MS;
    }

    return xTimerCreate(name, ticks, auto_reload, arg, handler);
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
int32_t wifi_os_timer_deinit(os_tmr_t timer)
{
    assert(timer);

    if(pdPASS == xTimerDelete(timer, (TickType_t)0))
        return -1;

    return 0;
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
int32_t wifi_os_timer_start(os_tmr_t timer)
{
    assert(timer);

    if(pdPASS == xTimerStart(timer, (TickType_t)0))
        return 0;

    return -1;
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
int32_t wifi_os_timer_stop(os_tmr_t timer)
{
    assert(timer);

    if(pdPASS == xTimerStop(timer, (TickType_t)0))
        return 0;

    return -1;
}

int32_t wifi_os_timer_reset(os_tmr_t timer)
{
    assert(timer);

    if(pdPASS == xTimerReset(timer, (uint32_t)0))
        return 0;

    return -1;
}

int32_t wifi_os_timer_change_period(os_tmr_t timer,  struct timeval *expires)
{
    uint32_t ticks;
    assert(timer != NULL && expires != NULL);
    

    ticks = (expires->tv_sec * 1000 + expires->tv_usec / 1000) / portTICK_PERIOD_MS;

    if (pdPASS == xTimerChangePeriod(timer, ticks, 0))
        return 0;

    return -1;
}



void* wifi_os_timer_get_user_data(os_tmr_t timer)
{
    assert(timer);

    return pvTimerGetTimerID(timer);
}


/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
void wifi_os_switch_context(void)
{
    taskYIELD();
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
void wifi_os_switch_context_from_isr(void)
{
    portYIELD_FROM_ISR(pdTRUE);
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
int32_t wifi_os_task_create(task_func_t handler,
                        char *name,
                        uint16_t stack_size,
                        void *params,
                        UBaseType_t prio,
                        os_task_t *task_handle
                        )
{
    if(pdPASS == xTaskCreate(handler,name,stack_size/4,params,prio,task_handle))
        return 0;

    return -1;
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
void wifi_os_task_delete(os_task_t task)
{
    vTaskDelete(task);
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
void wifi_os_task_sleep(uint32_t ms)
{
    vTaskDelay((TickType_t) (ms/portTICK_PERIOD_MS));
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
void wifi_os_task_enter_critical(void)
{
    taskENTER_CRITICAL();
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
void wifi_os_task_exit_critical(void)
{
    taskEXIT_CRITICAL();
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
os_task_t wifi_os_task_get_handle(void)
{
    return xTaskGetCurrentTaskHandle();
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
char * wifi_os_task_get_name(os_task_t task_handle)
{
    return pcTaskGetTaskName(task_handle);
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
BaseType_t wifi_os_task_get_scheduler_state(void)
{
    return xTaskGetSchedulerState();
}


/*alloc no cache mem*/
void *wifi_os_malloc_nc( size_t xWantedSize )
{
    return pvPortMallocNC(xWantedSize);
}

/*alloc no cache mem*/
void wifi_os_free_nc(void *pv)
{
    vPortFreeNC(pv);
}



/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
void* wifi_os_malloc(size_t size)
{
    return pvPortMalloc(size);
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
void* wifi_os_zalloc(size_t size)
{
    return pvPortCalloc(1, size);
}

/**
* @brief WiFi OS Adapter Layer API for internal usage, not open for other module or user
* @param
* @return
*/
void wifi_os_free(void *mem)
{
    vPortFree(mem);
}

size_t wifi_os_get_free_heap_size(void)
{
    return xPortGetFreeHeapSize();
}

#if 0
void* wifi_os_memcpy(void* dest, const void* src, uint32_t size)
{
    return memcpy(dest, src, size);
}

int wifi_os_memcmp(const void* buf1, const void* buf2, uint32_t size)
{
    return memcmp(buf1, buf2, size);
}

void* wifi_os_memset(void* buf, uint8_t ch, uint32_t size)
{
    return memset(buf, ch, size);
}

void* wifi_os_memmove(void* dest, const void* src, uint32_t size)
{
    return memmove(dest, src, size);
}
#endif


unsigned int wifi_os_task_get_tick(void)
{
    return xTaskGetTickCount();
}



os_event_group_t wifi_os_event_create_group(void)
{
    return xEventGroupCreate();
}

uint32_t wifi_os_event_group_wait_bits(os_event_group_t event_group, const uint32_t bits_to_waitFor, const long clear_on_xit, const long wait_for_all_bits, uint32_t ticks_to_wait)
{
    return xEventGroupWaitBits(event_group, bits_to_waitFor, clear_on_xit, wait_for_all_bits, ticks_to_wait);
}

  
uint32_t wifi_os_event_group_clear_bits(os_event_group_t event_group, const uint32_t bits_to_clear)
{
    return xEventGroupClearBits(event_group, bits_to_clear);
}


uint32_t wifi_os_event_group_set_bits( os_event_group_t event_group, const uint32_t bits_to_set )
{
    return xEventGroupSetBits(event_group, bits_to_set );
}



void wifi_os_queue_add_to_registry(os_queue_handle_t queue, const char *pc_queue_name)
{
    vQueueAddToRegistry(queue, pc_queue_name);
}


int32_t wifi_os_queue_add_to_set( os_queue_set_member_t queue_or_semaphore,  os_queue_set_t queue_set )
{

#if ( configUSE_QUEUE_SETS == 1 )
    if (pdPASS == xQueueAddToSet(queue_or_semaphore, queue_set))
        return 0;
#endif
    return -1;
}

#if 0
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
#endif

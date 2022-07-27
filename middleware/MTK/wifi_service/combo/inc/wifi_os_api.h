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

#ifndef __WIFI_OS_API_H__
#define __WIFI_OS_API_H__

/**
 * This header file describes the apis which might have different implementation if use differnt platform.
 * Once change the platform, these api need to be implemented by user.
 */

#include <stdbool.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "portmacro.h"
#include "queue.h"
#include <timers.h>
#include <string.h>
#include "timers.h"
#include "portmacro.h"
#include "event_groups.h"
#include "wifi_freertos_adapter.h"


typedef QueueHandle_t               os_queue_handle_t;
//typedef SemaphoreHandle_t           os_semaphore_t;
//typedef TimerHandle_t               os_tmr_t;
//typedef TaskHandle_t                os_task_t;
typedef EventGroupHandle_t          os_event_group_t;
typedef QueueSetMemberHandle_t      os_queue_set_member_t;
typedef QueueSetHandle_t            os_queue_set_t;


#if defined(__GNUC__)
#include <sys/time.h>
#elif defined(__ICCARM__) || defined(__CC_ARM)
#if !LWIP_TIMEVAL_PRIVATE
struct timeval {
  long      tv_sec;
  long      tv_usec;
};
#endif
#endif

typedef void (*timer_handle_t)(os_tmr_t);
typedef void (*task_func_t)(void *);

os_queue_handle_t wifi_os_queue_create( uint32_t queue_len, uint32_t item_size);
void wifi_os_queue_delete(os_queue_handle_t queue);
int32_t wifi_os_queue_receive(os_queue_handle_t queue, void *buf, struct timeval *wait_time);
int32_t wifi_os_queue_send(os_queue_handle_t queue, void *item, struct timeval *wait_time);
int32_t wifi_os_queue_send_front(os_queue_handle_t queue, void *item, struct timeval *wait_time);
int32_t wifi_os_queue_send_back(os_queue_handle_t queue, void *item, struct timeval *wait_time);
int32_t wifi_os_queue_send_from_isr(os_queue_handle_t queue, void *item);
int32_t wifi_os_queue_send_front_from_isr(os_queue_handle_t queue, void *item);
int32_t wifi_os_receive_from_isr(os_queue_handle_t queue, void *buf);
uint32_t wifi_os_queue_get_space( const os_queue_handle_t queue );


os_semaphore_t wifi_os_semphr_create_mutex(void);
os_semaphore_t wifi_os_semphr_create_binary(void);
int32_t wifi_os_semphr_take(os_semaphore_t semphr, struct timeval *wait_time);
int32_t wifi_os_semphr_give(os_semaphore_t semphr);
int32_t wifi_os_semphr_take_from_isr(os_semaphore_t semphr);
int32_t wifi_os_semphr_give_from_isr(os_semaphore_t semphr);
void wifi_os_semphr_delete(os_semaphore_t semphr);

os_tmr_t wifi_os_timer_init(const char * const name, struct timeval *expires, unsigned long auto_reload, timer_handle_t handler, void *arg);
int32_t wifi_os_timer_deinit(os_tmr_t timer);
int32_t wifi_os_timer_start(os_tmr_t timer);
int32_t wifi_os_timer_stop(os_tmr_t timer);
int32_t wifi_os_timer_reset(os_tmr_t timer);
int32_t wifi_os_timer_change_period(os_tmr_t timer,  struct timeval *expires);
void* wifi_os_timer_get_user_data(os_tmr_t timer);


int32_t wifi_os_task_create(task_func_t handler,
                        char *name,
                        uint16_t stack_size,
                        void *params,
                        UBaseType_t prio,
                        os_task_t *task_handle
                        );
void wifi_os_task_delete(os_task_t task);
void wifi_os_task_sleep(uint32_t ms);
void wifi_os_task_enter_critical(void);
void wifi_os_task_exit_critical(void);
os_task_t wifi_os_task_get_handle(void);
char * wifi_os_task_get_name(os_task_t task_handle);
BaseType_t wifi_os_task_get_scheduler_state(void);


void* wifi_os_malloc(size_t size);
void* wifi_os_zalloc(size_t size);
void wifi_os_free(void *mem);
void *wifi_os_malloc_nc( size_t xWantedSize );
void wifi_os_free_nc(void *pv);

unsigned int wifi_os_task_get_tick(void);
void wifi_os_queue_add_to_registry(os_queue_handle_t queue, const char *pc_queue_name);
int32_t wifi_os_queue_add_to_set( os_queue_set_member_t queue_or_semaphore,  os_queue_set_t queue_set );
os_queue_handle_t wifi_os_queue_set_create( uint32_t queue_len);
os_event_group_t wifi_os_event_create_group(void);
uint32_t wifi_os_event_group_wait_bits(os_event_group_t event_group, const uint32_t bits_to_waitFor, const long clear_on_xit, const long wait_for_all_bits, uint32_t ticks_to_wait);
uint32_t wifi_os_event_group_clear_bits(os_event_group_t event_group, const uint32_t bits_to_clear);
uint32_t wifi_os_event_group_set_bits( os_event_group_t event_group, const uint32_t bits_to_set );


size_t wifi_os_get_free_heap_size(void);

void wifi_os_switch_context(void);
void wifi_os_switch_context_from_isr(void);

#endif /* __WIFI_OS_API_H__ */


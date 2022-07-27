/*
 * FreeRTOS Kernel V10.1.1
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "os_port_callback.h"

#ifdef MTK_OS_CPU_UTILIZATION_ENABLE
#include "hal_gpt.h"
#endif

#if  defined ( __GNUC__ )
#ifndef __weak
#define __weak   __attribute__((weak))
#endif /* __weak */
#endif /* __GNUC__ */

#ifdef MTK_OS_CPU_UTILIZATION_ENABLE
extern uint32_t get_current_count(void);
uint32_t runtime_counter_base;

void vConfigureTimerForRunTimeStats(void)
{
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &runtime_counter_base);
}

uint32_t ulGetRunTimeCounterValue(void)
{
    uint32_t cur_count, duration;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K,&cur_count);

    hal_gpt_get_duration_count(runtime_counter_base, cur_count, &duration);
    return duration;
}
#endif /* MTK_OS_CPU_UTILIZATION_ENABLE */

#if( configCHECK_FOR_STACK_OVERFLOW > 0 )
__weak void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    printf("stack overflow: %x %s\r\n", (unsigned int)xTask, (portCHAR *)pcTaskName);
    configASSERT(0);
}
#endif


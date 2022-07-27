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

#include "FreeRTOS.h"
#include "task.h"
#include "memory_attribute.h"

#ifdef MTK_SWLA_ENABLE
#include "swla.h"
#endif


#if defined(MTK_SWLA_ENABLE)
ATTR_TEXT_IN_TCM void vTraceTaskSwitchIn(uint32_t pRio)
{
    uint8_t *pxTaskName;
    uint32_t xTaskName;
    void    *pxTask;
    (void)pRio;

    //to do: get the task tcb directly from the pxCurrentTCB
    pxTask     = xTaskGetCurrentTaskHandle();

    //to do: get the task name address directly from pxCurrentTCB
    pxTaskName = (uint8_t *)pcTaskGetTaskName(pxTask);
    /* only support 4-ascii character */
    xTaskName = (uint32_t)(pxTaskName[0] | (pxTaskName[1] << 8) | (pxTaskName[2] << 16) | (pxTaskName[3] << 24));

    SLA_RamLogging(xTaskName);
}
#else
void vTraceTaskSwitchIn(uint32_t pRio)
{
#if ((INCLUDE_xTaskGetCurrentTaskHandle == 1) && (INCLUDE_pcTaskGetTaskName == 1))
    printf("switch to %s\n", pcTaskGetTaskName(xTaskGetCurrentTaskHandle()));
#endif
    (void)pRio;
}
#endif /* MTK_SWLA_ENABLE */

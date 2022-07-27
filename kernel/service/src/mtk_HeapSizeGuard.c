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
#ifdef MTK_HEAP_SIZE_GUARD_ENABLE
#include "FreeRTOS.h"
#include "task.h"
#include "mtk_HeapSizeGuard.h"
#include "syslog.h"

#include <string.h>

#define MAX_HEAP_SIZE_GUARD_NUM 25
static HeapStatisticsInfo_t xHeapSizeGuardArr[MAX_HEAP_SIZE_GUARD_NUM];

/* Heap size guard statistic for system initialize stage */
static HeapStatisticsInfo_t xInitStageGuard = {NULL, "Init", 0, 0, pdTRUE};

/* Heap size guard internal feature option definition */
//#define HEAP_SYS_PEAK_PROFILING             /* keyword: H_PeakSys */
#define HEAP_SYS_CUR_PROFILING                /* keyword: H_CurSys_A,H_CurSys_F */

//#define HEAP_TASK_PEAK_PROFILING              /* keyword: H_PeakTask */
//#define HEAP_PRINT_PEAK_SPECIFIC_USER       /* sub feature of HEAP_TASK_PEAK_PROFILING, keyword: H_PeakTask_A */
//#define HEAP_PRINT_FREE_SPECIFIC_USER       /* sub feature of HEAP_TASK_PEAK_PROFILING, keyword: H_CurTask_F */
//#define HEAP_PRINT_WHOLE_BLOCK_STATUS       /* sub feature of HEAP_TASK_PEAK_PROFILING */

//#define HEAP_SPECIFIC_SIZE_USER_STATISTIC   /* keyword: H_Specific_A,H_Specific_F */
//#define SPECIFIC_SIZE (512)

#define __USE_SYSLOG_SERVICE
#ifdef __USE_SYSLOG_SERVICE
log_create_module(heapSG, PRINT_LEVEL_INFO);
#define HEAP_GUARD_LOGI(fmt,arg...) LOG_I(heapSG,fmt,##arg)
#define HEAP_GUARD_MSGID_I(fmt,cnt,arg...) LOG_MSGID_I(heapSG,fmt,cnt,##arg)
#else
#define HEAP_GUARD_LOGI(fmt,arg...) printf(fmt,##arg)
#define HEAP_GUARD_MSGID_I(fmt,cnt,arg...) printf(fmt,##arg)
#endif /* __USE_SYSLOG_SERVICE */



/* NOTE!!! NOTE !!! NOTE !!! */
/* This macro should be sync with heap_x.c */
#define heapBITS_PER_BYTE		( ( size_t ) 8 )
typedef struct A_BLOCK_LINK {
    struct A_BLOCK_LINK *pxNextFreeBlock;	/*<< The next free block in the list */
    size_t xBlockSize;						/*<< The size of the free block */
#ifdef MTK_SUPPORT_HEAP_DEBUG
    uint32_t xLinkRegAddr;
#endif
} BlockLink_t;
static const size_t xHeapStructSize	= ((sizeof(BlockLink_t) + (portBYTE_ALIGNMENT - 1)) & ~portBYTE_ALIGNMENT_MASK);
/* NOTE!!! NOTE !!! NOTE !!! */
/* Set heap block allocated bit, NOTE: it should sync with heap_x.c */
/*
 * The upper byte of xBlockSize is used as follows:
 * The upper bit is used to track whether block is allocated (see heap_x.c)
 * Remaining bits are used to track target count so we know what task allocated it at free time.
 */
#define ALLOCATED_BIT_SHIFT_CNT ((sizeof(size_t) * heapBITS_PER_BYTE) - 1) // 31
#define TARGET_SHIFT_CNT ((sizeof(size_t) * heapBITS_PER_BYTE) - 8) // 24
#define TARGET_MASK (0x7F)
#define GET_ACTUAL_BLOCK_SIZE(xBlockSize)  ((xBlockSize) & (uint32_t)0x00FFFFFF)    /* mask off top byte*/
#define GET_TARGET(xBlockSize) (((xBlockSize) & 0x7F000000) >> TARGET_SHIFT_CNT)
#define GET_ALLOCATED_BIT(xBlockSize) ((xBlockSize) & (((size_t)1) << ALLOCATED_BIT_SHIFT_CNT))
#define SET_TARGET(xBlockSize, xTargetCnt) ((xBlockSize) | (((uint32_t)xTargetCnt) << TARGET_SHIFT_CNT))
#define TARGET_TOO_LARGE(xTargetCnt) (((xTargetCnt) & ~TARGET_MASK) > 0)
#define CLEAR_TARGET(xBlockSize) ((xBlockSize) & 0x80FFFFFF)

#if defined(__GNUC__)
/* Real target functions which we wrap, linker will add prefix __real to target functions */
void *__real_pvPortMalloc(size_t xwanted_size);
void __real_vPortFree(void *pv);
#elif defined(__CC_ARM)
void *$Sub$$pvPortMalloc(size_t xwanted_size);
void $Sub$$vPortFree(void *pv);
void *$Super$$pvPortMalloc(size_t xwanted_size);
void $Super$$vPortFree(void *pv);
#endif /* __GNUC__ */

/* Wrap functions for heap guard, only support Keil and GCC */
#if defined(__GNUC__)
void *__wrap_pvPortMalloc(size_t xwanted_size)
#elif defined(__CC_ARM)
void *$Sub$$pvPortMalloc(size_t xwanted_size)
#endif /* __GNUC__ */
{
    void *pvReturn;
    BlockLink_t *pxLink;
    HeapStatisticsInfo_t *pxStatInfo;
    TaskHandle_t *pxCurrentTask;
    uint32_t i, xBlockSize = 0;
    int32_t xTargetCount = -1;

    if (xwanted_size != GET_ACTUAL_BLOCK_SIZE(xwanted_size)) {
        /* xwanted_size is too big */
        HEAP_GUARD_MSGID_I("mtk_HeapSizeGuard: too large of a request. Do not enable this feature\n", 0);
        return (NULL);
    }

#if defined(HEAP_SYS_PEAK_PROFILING) || defined(HEAP_PRINT_PEAK_SPECIFIC_USER)
    uint32_t xMiniEverTotalFree = 0U;
    static uint32_t xIsFirstMalloc = 1U;
#endif /* defined(HEAP_SYS_PEAK_PROFILING) || defined(HEAP_PRINT_PEAK_SPECIFIC_USER) */

#ifdef HEAP_SYS_PEAK_PROFILING
    uint32_t xMiniEverTotalFreeTmp = 0U;
#endif /* HEAP_SYS_PEAK_PROFILING */

#ifdef HEAP_PRINT_PEAK_SPECIFIC_USER
    BaseType_t flag = pdFALSE;
#endif /* HEAP_PRINT_PEAK_SPECIFIC_USER */

    /* Obtain the return address of caller from link register */
#if defined(__GNUC__)
    uint32_t xLinkRegAddr = (uint32_t)__builtin_return_address(0);
#elif defined(__CC_ARM)
    uint32_t xLinkRegAddr = __return_address();
#endif /* __GNUC__ */

#ifdef HEAP_SYS_PEAK_PROFILING
	if(!xIsFirstMalloc){
        /* Get heap minimum ever free size before allocation for statistic total heap peak after heap initialize done*/
        xMiniEverTotalFree = xPortGetMinimumEverFreeHeapSize();
        xIsFirstMalloc = 0U;
	}
#endif /* HEAP_SYS_PEAK_PROFILING */

    /* Call real pvPortMalloc */
#if defined(__GNUC__)
    pvReturn = __real_pvPortMalloc(xwanted_size);
#elif defined(__CC_ARM)
    pvReturn = (void *)$Super$$pvPortMalloc(xwanted_size);
#endif /* __GNUC__ */

    /* Return if alloc memory fail*/
    if (pvReturn == NULL) {
        return pvReturn;
    }

    pxLink = (BlockLink_t *)((uint8_t *)pvReturn - xHeapStructSize);
    xBlockSize = GET_ACTUAL_BLOCK_SIZE(pxLink->xBlockSize);

#ifdef HEAP_SYS_CUR_PROFILING
    /*H_CurSys_A:tick,addr,userSize,blockSize,lr,curSysSize*/
    HEAP_GUARD_MSGID_I("H_CurSys_A:%d,0x%x,%d,%d,0x%x,%d\r\n", 6,
         (unsigned int)xTaskGetTickCount(),
         (unsigned int)pvReturn,
         xwanted_size,
         xBlockSize,
         (unsigned int)xLinkRegAddr,
         configTOTAL_HEAP_SIZE - xPortGetFreeHeapSize());
#endif /* HEAP_SYS_CUR_PROFILING */

#ifdef HEAP_SYS_PEAK_PROFILING
    xMiniEverTotalFreeTmp = xPortGetMinimumEverFreeHeapSize();
    if (xMiniEverTotalFreeTmp < xMiniEverTotalFree) {
        /*H_PeakSys:tick,addr,userSize,blockSize,lr,peakSysSize*/
        HEAP_GUARD_MSGID_I("H_PeakSys:%d,0x%x,%d,%d,0x%x,%d\r\n", 6,
             (unsigned int)xTaskGetTickCount(),
             (unsigned int)pvReturn,
             xwanted_size,
             xBlockSize,
             (unsigned int)xLinkRegAddr,
             configTOTAL_HEAP_SIZE - xMiniEverTotalFreeTmp);
    }
#endif /* HEAP_SYS_PEAK_PROFILING */

#ifdef HEAP_TASK_PEAK_PROFILING
    /* Do heap size guard in initialization stage*/
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        if (xInitStageGuard.xHeapGuardFlag == pdTRUE) {
            xInitStageGuard.xCurAllocSize += xBlockSize;

            if (xInitStageGuard.xCurAllocSize > xInitStageGuard.xPeakAllocSize) {
                xInitStageGuard.xPeakAllocSize = xInitStageGuard.xCurAllocSize;
#ifdef HEAP_PRINT_PEAK_SPECIFIC_USER
                /*H_PeakTask_A:tick,addr,userSize,blockSize,taskName,lr,peakTask,curTask,peakSys*/
                HEAP_GUARD_MSGID_I("H_PeakTask_A:%d,0x%x,%d,%d,Init,0x%x,%d,%d,%d\r\n", 8,
                     (unsigned int)xTaskGetTickCount(),
                     (unsigned int)pvReturn,
                     xwanted_size,
                     xBlockSize,
                     (unsigned int)xLinkRegAddr,
                     xInitStageGuard.xPeakAllocSize,
                     xInitStageGuard.xCurAllocSize,
                     configTOTAL_HEAP_SIZE - xPortGetMinimumEverFreeHeapSize());
#endif /* HEAP_PRINT_PEAK_SPECIFIC_USER */
            }
        }
        return pvReturn;
    }

    vTaskSuspendAll();
    /* Update the guarded task heap statistic info*/
    pxCurrentTask =  xTaskGetCurrentTaskHandle();
    for (i = 0; i < MAX_HEAP_SIZE_GUARD_NUM; i++) {
        pxStatInfo = &xHeapSizeGuardArr[i];
        if ((pxStatInfo->xTaskHandle == pxCurrentTask) && (pxStatInfo->xTaskHandle != NULL)) {
            xTargetCount = (int32_t)i;
            break;
        }
    }
    if (xTargetCount != -1) {
        pxStatInfo = &xHeapSizeGuardArr[xTargetCount];
        if (pxStatInfo->xHeapGuardFlag == pdTRUE) {
            if (!TARGET_TOO_LARGE(xTargetCount)) {
                pxLink->xBlockSize = SET_TARGET(pxLink->xBlockSize, xTargetCount);
            }
            /* Update current allocated size */
            pxStatInfo->xCurAllocSize += xBlockSize;

            /* Update peak allocated size */
            if (pxStatInfo->xCurAllocSize > pxStatInfo->xPeakAllocSize) {
                pxStatInfo->xPeakAllocSize = pxStatInfo->xCurAllocSize;
#ifdef HEAP_PRINT_PEAK_SPECIFIC_USER
                flag = pdTRUE;
#endif /* HEAP_PRINT_PEAK_SPECIFIC_USER */
            }
#ifdef HEAP_PRINT_PEAK_SPECIFIC_USER
            /* Update total minimum ever free size and current free size */
            xMiniEverTotalFree = xPortGetMinimumEverFreeHeapSize();
#endif /* HEAP_PRINT_PEAK_SPECIFIC_USER */
        }
        xTaskResumeAll();

#ifdef HEAP_PRINT_PEAK_SPECIFIC_USER
        /* Print info if HeapSizeGuard is enabled, and peak is updated by this malloc */
        if (flag) {
            /*H_PeakTask_A:tick,addr,userSize,blockSize,taskName,lr,peakTask,curTask,peakSys */
            HEAP_GUARD_LOGI("H_PeakTask_A:%d,0x%x,%d,%d,%s,0x%x,%d,%d,%d\r\n",
                 (unsigned int)xTaskGetTickCount(),
                 (unsigned int)pvReturn,
                 xwanted_size,
                 xBlockSize,
                 xHeapSizeGuardArr[xTargetCount].ucTaskName,
                 (unsigned int)xLinkRegAddr,
                 pxStatInfo->xPeakAllocSize,
                 pxStatInfo->xCurAllocSize,
                 configTOTAL_HEAP_SIZE - xMiniEverTotalFree);
        }
#endif /* HEAP_PRINT_PEAK_SPECIFIC_USER */
    } else {
        xTaskResumeAll();
        HEAP_GUARD_LOGI("H_PeakTask_A: not registered task: %s,0x%x\r\n", pcTaskGetTaskName(xTaskGetCurrentTaskHandle()), (unsigned int)xLinkRegAddr);
    }
#endif /* HEAP_TASK_PEAK_PROFILING */

#ifdef HEAP_SPECIFIC_SIZE_USER_STATISTIC
    if (xBlockSize >= SPECIFIC_SIZE) {
        if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
            /*H_Specific_A:tick,addr,userSize,blockSize,taskName,lr*/
            HEAP_GUARD_MSGID_I("H_Specific_A:%d,0x%x,%d,%d,InitStage,0x%x\r\n", 5,
                 (unsigned int)xTaskGetTickCount(),
                 (unsigned int)pvReturn,
                 xwanted_size,
                 xBlockSize,
                 (unsigned int)xLinkRegAddr);
        } else {
            /*H_Specific_A:tick,addr,userSize,blockSize,taskName,lr*/
            HEAP_GUARD_LOGI("H_Specific_A:%d,0x%x,%d,%d,%s,0x%x\r\n",
                 (unsigned int)xTaskGetTickCount(),
                 (unsigned int)pvReturn,
                 xwanted_size,
                 xBlockSize,
                 pcTaskGetTaskName(xTaskGetCurrentTaskHandle()),
                 (unsigned int)xLinkRegAddr);
        }
    }
#endif /* HEAP_SPECIFIC_SIZE_USER_STATISTIC */
    return pvReturn;
}


/* vPortFree wrap function */
#if defined(__GNUC__)
void __wrap_vPortFree(void *pv)
#elif defined(__CC_ARM)
void $Sub$$vPortFree(void *pv)
#endif /* __GNUC__ */
{
    HeapStatisticsInfo_t *pxStatInfo;
    BlockLink_t *pxLink;
    TaskHandle_t *pxCurrentTask;
    uint32_t i, xBlockSize = 0, xOrigBlockSize = 0;
    int32_t xTargetCount = -1;

#if defined(HEAP_PRINT_FREE_SPECIFIC_USER) || defined(HEAP_SPECIFIC_SIZE_USER_STATISTIC)
    BaseType_t flag = pdFALSE;
#endif /* defined(HEAP_PRINT_FREE_SPECIFIC_USER) || defined(HEAP_SPECIFIC_SIZE_USER_STATISTIC) */

    if (pv == NULL) {
        return;
    }

#if defined(__GNUC__)
    uint32_t xLinkRegAddr = (uint32_t)__builtin_return_address(0);
#elif defined(__CC_ARM)
    uint32_t xLinkRegAddr = __return_address();
#endif /* __GNUC__ */

    pxLink = (BlockLink_t *)((uint8_t *)pv - xHeapStructSize);
    xOrigBlockSize = pxLink->xBlockSize;
    xBlockSize = GET_ACTUAL_BLOCK_SIZE(xOrigBlockSize);
    xTargetCount = GET_TARGET(xOrigBlockSize);
    pxLink->xBlockSize = CLEAR_TARGET(xOrigBlockSize);

#if defined(__GNUC__)
    __real_vPortFree(pv);
#elif defined(__CC_ARM)
    $Super$$vPortFree(pv);
#endif /* __GNUC__ */

#ifdef HEAP_SYS_CUR_PROFILING
    /*H_CurSys_F:tick,addr,--,blockSize,lr,curSysSize*/
    HEAP_GUARD_MSGID_I("H_CurSys_F:%d,0x%x,--,%d,0x%x,%d\r\n", 5,
         (unsigned int)xTaskGetTickCount(),
         (unsigned int)pv,
         xBlockSize,
         (unsigned int)xLinkRegAddr,
         configTOTAL_HEAP_SIZE - xPortGetFreeHeapSize());

#endif /* HEAP_SYS_CUR_PROFILING */

#ifdef HEAP_TASK_PEAK_PROFILING
    /* Do heap size guard at initialization stage*/
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        if (xInitStageGuard.xHeapGuardFlag == pdTRUE) {
            xInitStageGuard.xCurAllocSize -= xBlockSize;

#ifdef HEAP_PRINT_FREE_SPECIFIC_USER
            /*H_CurTask_F:tick,addr,userSize,blockSize,taskName,lr,peakTask,curTask*/
            HEAP_GUARD_MSGID_I("H_CurTask_F:%d,0x%x,--,%d,Init,0x%x,%d,%d\r\n", 6,
                 (unsigned int)xTaskGetTickCount(),
                 (unsigned int)pv,
                 xBlockSize,
                 (unsigned int)xLinkRegAddr,
                 xInitStageGuard.xPeakAllocSize,
                 xInitStageGuard.xCurAllocSize);
#endif /* HEAP_PRINT_FREE_SPECIFIC_USER */
        }

        return;
    }

    vTaskSuspendAll();
    /* Update the guarded task heap statistic info*/
    pxCurrentTask = xTaskGetCurrentTaskHandle();
    if (!xTargetCount) {
        /* if we don't have a target from alloc time use current task */
        xTargetCount = -1;
        for (i = 0; i < MAX_HEAP_SIZE_GUARD_NUM; i++) {
            pxStatInfo = &xHeapSizeGuardArr[i];
            if ((pxStatInfo->xTaskHandle == pxCurrentTask) && (pxStatInfo->xTaskHandle != NULL)) {
                xTargetCount = (int32_t)i;
                break;
            }
        }
    }
    xTaskResumeAll();

    if (xTargetCount != -1) {
        vTaskSuspendAll();

        pxStatInfo = &xHeapSizeGuardArr[xTargetCount];
        if (pxStatInfo->xHeapGuardFlag == pdTRUE) {

            /* Update current allocated size */
            if (pxStatInfo->xCurAllocSize < xBlockSize) {
#if defined(HEAP_PRINT_FREE_SPECIFIC_USER) || defined(HEAP_SPECIFIC_SIZE_USER_STATISTIC)
                /* Special case: allocate a block in system initialize stage, but free the block after system scheduling... */
                flag = pdTRUE;
#endif /* defined(HEAP_PRINT_FREE_SPECIFIC_USER) || defined(HEAP_SPECIFIC_SIZE_USER_STATISTIC) */
            } else {
                pxStatInfo->xCurAllocSize -= xBlockSize;
            }
        }
        xTaskResumeAll();

#ifdef HEAP_PRINT_FREE_SPECIFIC_USER
        if (pxStatInfo->xHeapGuardFlag == pdTRUE) {

            if (flag) {
                // Add '!' to prompt.
                /*!H_CurTask_F:tick,addr,blockSize,taskName,lr,peakTask,curTask*/
                HEAP_GUARD_LOGI("!H_CurTask_F:%d,0x%x,--,%d,%s,0x%x,%d,%d\r\n",
                     (unsigned int)xTaskGetTickCount(),
                     (unsigned int)pv,
                     xBlockSize,
                     xHeapSizeGuardArr[xTargetCount].ucTaskName,
                     (unsigned int)xLinkRegAddr,
                     pxStatInfo->xPeakAllocSize,
                     pxStatInfo->xCurAllocSize);
            } else {
                /*H_CurTask_F:tick,addr,blockSize,taskName,lr,peakTask,curTask*/
                HEAP_GUARD_LOGI("H_CurTask_F:%d,0x%x,--,%d,%s,0x%x,%d,%d\r\n",
                     (unsigned int)xTaskGetTickCount(),
                     (unsigned int)pv,
                     xBlockSize,
                     xHeapSizeGuardArr[xTargetCount].ucTaskName,
                     (unsigned int)xLinkRegAddr,
                     pxStatInfo->xPeakAllocSize,
                     pxStatInfo->xCurAllocSize);
            }
        }
#endif /* HEAP_PRINT_FREE_SPECIFIC_USER */
    } else {
        HEAP_GUARD_LOGI("H_CurTask_F: not registered task: %s,0x%x\r\n", pcTaskGetTaskName(xTaskGetCurrentTaskHandle()), (unsigned int)xLinkRegAddr);
    }
#endif /* HEAP_TASK_PEAK_PROFILING */
#ifdef HEAP_SPECIFIC_SIZE_USER_STATISTIC
    if (xBlockSize >= SPECIFIC_SIZE) {
        if (flag) {
            // Add '!' to prompt.
            /*!H_Specific_F:tick,addr,blockSize,taskName,lr*/
            HEAP_GUARD_LOGI("!H_Specific_F:%d,--,0x%x,%d,%s,0x%x\r\n",
                 (unsigned int)xTaskGetTickCount(),
                 (unsigned int)pv,
                 xBlockSize,
                 xHeapSizeGuardArr[xTargetCount].ucTaskName,
                 (unsigned int)xLinkRegAddr);
        } else {
            /*H_Specific_F:tick,addr,blockSize,taskName,lr*/
            HEAP_GUARD_LOGI("H_Specific_F:%d,--,0x%x,%d,%s,0x%x\r\n",
                 (unsigned int)xTaskGetTickCount(),
                 (unsigned int)pv,
                 xBlockSize,
                 xHeapSizeGuardArr[xTargetCount].ucTaskName,
                 (unsigned int)xLinkRegAddr);
        }
    }
#endif /* HEAP_SPECIFIC_SIZE_USER_STATISTIC */
}

#ifdef HEAP_TASK_PEAK_PROFILING
/**
 * Register heap size guard runtime
 * This function is for runtime created tasks, which has heap size statistics requirement
**/
BaseType_t xHeapSizeGuard_Register(TaskHandle_t xTaskHandle)
{
    uint32_t i = 0;
    BaseType_t xReturn = pdFAIL;
    HeapStatisticsInfo_t *pxStatInfo;

    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        vTaskSuspendAll();
    }

    for (; i < MAX_HEAP_SIZE_GUARD_NUM; i++) {
        pxStatInfo = &xHeapSizeGuardArr[i];
        if (pxStatInfo->xTaskHandle == xTaskHandle) {
            xReturn = pdPASS;
            break; /* already in HeapSizeGuard table */
        } else if (pxStatInfo->xTaskHandle == NULL) {
            /* find a free table, and insert in */
            pxStatInfo->xTaskHandle = xTaskHandle;
            pxStatInfo->xPeakAllocSize = (size_t)0;
            pxStatInfo->xCurAllocSize = (size_t)0;
            pxStatInfo->xHeapGuardFlag = pdTRUE ;
            strncpy(pxStatInfo->ucTaskName, pcTaskGetTaskName(xTaskHandle), configMAX_TASK_NAME_LEN);
            xReturn = pdPASS;
            break;
        }
    }

    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xTaskResumeAll();
    }
    return xReturn;
}



/**
 * Start heap size guard
 * note1: history statistic data will be cleard
 * note2: if input NULL,  all tasks heap size guard are restarted
**/

BaseType_t xHeapSizeGuard_Restart(TaskHandle_t xTaskHandle)
{
    uint32_t i = 0;
    BaseType_t xReturn = pdFALSE;
    HeapStatisticsInfo_t *pxStatInfo;

    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        vTaskSuspendAll();
    }

    for (; i < MAX_HEAP_SIZE_GUARD_NUM; i++) {
        pxStatInfo = &xHeapSizeGuardArr[i];
        if (xTaskHandle == NULL) {
            if (pxStatInfo->xTaskHandle != NULL) {
                pxStatInfo->xPeakAllocSize = (size_t)0;
                pxStatInfo->xCurAllocSize = (size_t)0;
                pxStatInfo->xHeapGuardFlag = pdTRUE ;
            }
            xReturn = pdTRUE;
        } else {
            if (pxStatInfo->xTaskHandle == xTaskHandle) {
                pxStatInfo->xPeakAllocSize = (size_t)0;
                pxStatInfo->xCurAllocSize = (size_t)0;
                pxStatInfo->xHeapGuardFlag = pdTRUE ;

                xReturn = pdTRUE;
                break;
            }
        }

    }

    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xTaskResumeAll();
    }


    return xReturn;
}

/**
 * Stop heap size guard
 * note1: if input NULL, all tasks heap size guard are stoped
**/
BaseType_t xHeapSizeGuard_Stop(TaskHandle_t xTaskHandle)
{
    uint32_t i = 0;
    BaseType_t xReturn = pdFALSE;
    HeapStatisticsInfo_t *pxStatInfo;

    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        vTaskSuspendAll();
    }

    for (; i < MAX_HEAP_SIZE_GUARD_NUM; i++) {
        pxStatInfo = &xHeapSizeGuardArr[i];
        if (xTaskHandle == NULL) {
            if (pxStatInfo->xTaskHandle != NULL) {
                pxStatInfo->xHeapGuardFlag = pdFALSE;
                xReturn = pdTRUE;
            }
        } else {
            if (pxStatInfo->xTaskHandle == xTaskHandle) {
                pxStatInfo->xHeapGuardFlag = pdFALSE ;

                xReturn = pdTRUE;
                break;
            }
        }
    }

    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xTaskResumeAll();
    }

    return xReturn;
}

HeapStatisticsInfo_t *pxHeapSizeGuard_GetRawInfo(TaskHandle_t xTaskHandle)
{
    uint32_t i = 0;
    HeapStatisticsInfo_t *pxStatInfo;

    pxStatInfo = &xHeapSizeGuardArr[0];

    if (xTaskHandle == NULL) {
        return pxStatInfo;
    } else {
        for (; i < MAX_HEAP_SIZE_GUARD_NUM; i++) {
            pxStatInfo = &xHeapSizeGuardArr[i];
            /* Only dump special task statistic info */
            if (pxStatInfo->xTaskHandle == xTaskHandle) {
                return pxStatInfo;
            }
        }
    }

    return NULL;
}

/**
 * Dump heap size guard statistic information
 * note1: if input NULL, all tasks heap size guard statistic info are dumped
**/
void vHeapSizeGuard_DumpInfo(TaskHandle_t xTaskHandle)
{
    uint32_t i = 0, xCount = 0, xStrLen = 0;
    HeapStatisticsInfo_t *pxStatInfo;
    char *pxResultStr = NULL;

    /* allocate a buffer to save statistic infomation in order to reduce syslog channel usage*/
#define BUFF_SIZE 256
#define SINGLE_STR_SIZE 64

    if (xTaskHandle == NULL) {
        vHeapSizeGuard_DumpSysInitInfo();

        pxResultStr = pvPortMalloc((size_t)BUFF_SIZE);
        if (pxResultStr == NULL) {
            HEAP_GUARD_MSGID_I("allocate result buffer fail.\r\n", 0);
            return;
        } else {
            memset(pxResultStr, 0, BUFF_SIZE);
        }
    }

    for (; i < MAX_HEAP_SIZE_GUARD_NUM; i++) {
        pxStatInfo = &xHeapSizeGuardArr[i];

        /* Need dump all statistic info */
        if (xTaskHandle == NULL) {
            if (pxStatInfo->xTaskHandle != NULL) {
                /* save in temp buffer */
                snprintf(pxResultStr + xStrLen, SINGLE_STR_SIZE, "task=%s, peak=%d, cur=%d, flag=%d\r\n",
                         pxStatInfo->ucTaskName,
                         (int)pxStatInfo->xPeakAllocSize,
                         (int)pxStatInfo->xCurAllocSize,
                         (int)pxStatInfo->xHeapGuardFlag);

                xCount++;
                xStrLen = strlen(pxResultStr);

                /* output by syslog */
                if (xCount == (BUFF_SIZE / SINGLE_STR_SIZE)) {
                    HEAP_GUARD_LOGI("%s", pxResultStr);
                    memset(pxResultStr, 0, BUFF_SIZE);
                    xCount = 0;
                    xStrLen = 0;
                }
            }
        } else {
            /* Only dump special task statistic info */
            if (pxStatInfo->xTaskHandle == xTaskHandle) {
                HEAP_GUARD_LOGI("\ntask=%s, peak=%d, cur=%d, flag=%d\r\n",
                     pxStatInfo->ucTaskName,
                     (int)pxStatInfo->xPeakAllocSize,
                     (int)pxStatInfo->xCurAllocSize,
                     (int)pxStatInfo->xHeapGuardFlag);
                break;
            }
        }
    }

    if (xTaskHandle == NULL) {
        /* Output left buffer by syslog */
        if (xCount != 0) {
            HEAP_GUARD_LOGI("%s", pxResultStr);
        }
        vPortFree(pxResultStr);

        /* Output heap total size info*/
        HEAP_GUARD_MSGID_I("\n[HeapUsageFromSysView] peak=%d, cur=%d\r\n", 2,
             configTOTAL_HEAP_SIZE - xPortGetMinimumEverFreeHeapSize(),
             configTOTAL_HEAP_SIZE - xPortGetFreeHeapSize());
    }

#ifdef HEAP_PRINT_WHOLE_BLOCK_STATUS
    extern void vDumpHeapStatus(void);
    vDumpHeapStatus();
#endif /* HEAP_PRINT_WHOLE_BLOCK_STATUS */

}

/**
 * Dump system initialization stage heap size usage information
**/
void vHeapSizeGuard_DumpSysInitInfo(void)
{
    HEAP_GUARD_MSGID_I("\n[SysInit] peak=%d, cur=%d, flag=%d\r\n", 3,
         (int)xInitStageGuard.xPeakAllocSize,
         (int)xInitStageGuard.xCurAllocSize,
         (int)xInitStageGuard.xHeapGuardFlag);

}
#endif /* HEAP_TASK_PEAK_PROFILING */

#endif /* MTK_HEAP_SIZE_GUARD_ENABLE */

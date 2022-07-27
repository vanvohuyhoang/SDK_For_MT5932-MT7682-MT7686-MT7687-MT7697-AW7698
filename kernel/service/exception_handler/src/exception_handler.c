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

/* Includes ------------------------------------------------------------------*/
#include "memory_attribute.h"
#include "exception_handler.h"
#include "hal_dwt.h"
#include "hal_uart.h"
#include "hal_gpt.h"

#ifdef HAL_WDT_MODULE_ENABLED
#include "hal_wdt.h"
#endif

#ifdef MTK_SYSTEM_HANG_TRACER_ENABLE
#include "systemhang_tracer.h"
#endif /* MTK_SYSTEM_HANG_TRACER_ENABLE */

#ifdef MTK_BOOTREASON_CHECK_ENABLE
#include "bootreason_check.h"
#endif /* MTK_BOOTREASON_CHECK_ENABLE */

#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
#include "offline_dump.h"
#include "memory_map.h"
#ifdef MTK_FOTA_VIA_RACE_CMD
#include "fota_multi_info_util.h"
#endif /* MTK_FOTA_VIA_RACE_CMD */
#endif /* EXCEPTION_MEMDUMP_MODE */

#if (PRODUCT_VERSION == 2523 || PRODUCT_VERSION == 2533)
#include "hal_flash_mtd.h"
#endif /* PRODUCT_VERSION */

#if (PRODUCT_VERSION == 7687) || (PRODUCT_VERSION == 7697)
#include "flash_sfc.h"
#endif /* PRODUCT_VERSION */

#if (PRODUCT_VERSION == 7686) || (PRODUCT_VERSION == 7682)
#include "flash_sfc.h"
#endif /* PRODUCT_VERSION */

#if (PRODUCT_VERSION == 5932)
#endif /* PRODUCT_VERSION */

#if (PRODUCT_VERSION == 1552)
#include "hal_flash_mtd.h"
#include "hal_resource_assignment.h"
#include "hal_ccni.h"
#include "hal_ccni_config.h"
#include "hal_hw_semaphore.h"
#include "exception_handler_dsp.h"
#include "exception_handler_n9.h"
#endif /* PRODUCT_VERSION */

#ifdef MTK_SWLA_ENABLE
#include "swla.h"
#endif /* MTK_SWLA_ENABLE */


/* Private define ------------------------------------------------------------*/
#if  defined ( __GNUC__ )
  #ifndef __weak
    #define __weak   __attribute__((weak))
  #endif /* __weak */
#endif /* __GNUC__ */

/*
 * Debug scenarios:
 *
 * (1) debug with debugger, stop in first exception.
 *     Print the exception context, and halt cpu.
 *
 *     DEBUGGER_ON: 1
 *
 * (2) debug with uart, stop in first exception.
 *     Print the exception context, and enter an infinite loop.
 *
 *     DEBUGGER_ON: 0
 */
#define DEBUGGER_ON                                     0

#define EXCEPTION_CONFIGURATIONS_MAX                    6

#define EXCEPTION_STACK_WORDS                           512

#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
#if (PRODUCT_VERSION == 1552)
#define MINIDUMP_TOTAL_SIZE                             (16 * 1024)
#define MINIDUMP_ADDRESS_OFFSET_CM4                     (0)
#define MINIDUMP_HEADER_LENGTH                          (512)
#define MINIDUMP_CONTEXT_LENGTH                         (512)
#define MINIDUMP_MSPSTACK_LENGTH                        (2 * 1024)
#define MINIDUMP_PSPSTACK_LENGTH                        (3 * 1024)
#define MINIDUMP_DATA_SIZE_CM4                          (6144)
#else
#define MINIDUMP_TOTAL_SIZE                             (6 * 1024)
#define MINIDUMP_ADDRESS_OFFSET_CM4                     (0)
#define MINIDUMP_HEADER_LENGTH                          (512)
#define MINIDUMP_CONTEXT_LENGTH                         (512)
#define MINIDUMP_MSPSTACK_LENGTH                        (2 * 1024)
#define MINIDUMP_PSPSTACK_LENGTH                        (3 * 1024)
#define MINIDUMP_DATA_SIZE_CM4                          (6144)
#endif /* PRODUCT_VERSION */
#endif /* EXCEPTION_MEMDUMP_MODE */


/* Private typedef -----------------------------------------------------------*/
typedef enum
{
    EXCEPTION_RESET     = 1,
    EXCEPTION_NMI       = 2,
    EXCEPTION_HARD_FAULT = 3,
    EXCEPTION_MEMMANAGE_FAULT = 4,
    EXCEPTION_BUS_FAULT = 5,
    EXCEPTION_USAGE_FAULT = 6,
    EXCEPTION_DEBUGMON_FAULT = 12,
} exception_type_t;

typedef struct
{
  int items;
  exception_config_type configs[EXCEPTION_CONFIGURATIONS_MAX];
} exception_config_t;

typedef struct
{
  uint32_t items;
  memory_region_type regions[EXCEPTION_CONFIGURATIONS_MAX];
} exception_user_regions_t;

typedef struct
{
    uint32_t count;
    uint32_t timestamp;
    uint32_t reason;
    assert_expr_t *assert_expr;
} exception_info_t;

enum { r0, r1, r2, r3, r12, lr, pc, psr,
       s0, s1, s2, s3, s4, s5, s6, s7,
       s8, s9, s10, s11, s12, s13, s14, s15,
       fpscr
     };

typedef struct
{
    unsigned int r0;
    unsigned int r1;
    unsigned int r2;
    unsigned int r3;
    unsigned int r4;
    unsigned int r5;
    unsigned int r6;
    unsigned int r7;
    unsigned int r8;
    unsigned int r9;
    unsigned int r10;
    unsigned int r11;
    unsigned int r12;
    unsigned int sp;              /* after pop r0-r3, lr, pc, xpsr                   */
    unsigned int lr;              /* lr before exception                             */
    unsigned int pc;              /* pc before exception                             */
    unsigned int psr;             /* xpsr before exeption                            */
    unsigned int control;         /* nPRIV bit & FPCA bit meaningful, SPSEL bit = 0  */
    unsigned int exc_return;      /* current lr                                      */
    unsigned int msp;             /* msp                                             */
    unsigned int psp;             /* psp                                             */
    unsigned int fpscr;
    unsigned int s0;
    unsigned int s1;
    unsigned int s2;
    unsigned int s3;
    unsigned int s4;
    unsigned int s5;
    unsigned int s6;
    unsigned int s7;
    unsigned int s8;
    unsigned int s9;
    unsigned int s10;
    unsigned int s11;
    unsigned int s12;
    unsigned int s13;
    unsigned int s14;
    unsigned int s15;
    unsigned int s16;
    unsigned int s17;
    unsigned int s18;
    unsigned int s19;
    unsigned int s20;
    unsigned int s21;
    unsigned int s22;
    unsigned int s23;
    unsigned int s24;
    unsigned int s25;
    unsigned int s26;
    unsigned int s27;
    unsigned int s28;
    unsigned int s29;
    unsigned int s30;
    unsigned int s31;
} exception_context_t;

#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
typedef struct
{
    uint32_t address;
    uint32_t size;
} exception_minidump_region_t;

typedef struct
{
    uint32_t is_match;
    uint32_t overflow_address;
} exception_minidump_overflow_t;

typedef struct
{
    uint32_t core_num;
    uint32_t core_size[1 + EXCEPTION_SLAVES_TOTAL];
    uint32_t length;
    char name[20];
    uint32_t reason;
    assert_expr_t assert_info;
    exception_minidump_overflow_t overflow_info;
    uint32_t context_size;
    exception_minidump_region_t regions[8];
    uint32_t data_checksum;
} exception_minidump_header_t;
#endif /* EXCEPTION_MEMDUMP_MODE */

#if (EXCEPTION_SLAVES_TOTAL > 0)
typedef void (*f_exception_slave_alert_callback_t)(void);
typedef exception_slave_status_t (*f_exception_slave_checkstatus_callback_t)(void);
typedef void (*f_exception_slave_dump_callback_t)(void);
typedef void (*f_exception_slave_forceddump_callback_t)(void);
typedef struct
{
    const char* slave_name;
    f_exception_slave_alert_callback_t slave_alert;
    f_exception_slave_checkstatus_callback_t slave_checkstatus;
    f_exception_slave_dump_callback_t slave_dump;
    f_exception_slave_forceddump_callback_t slave_forceddump;
    unsigned int is_dump;
} exception_slaves_dump_t;
#endif /* EXCEPTION_SLAVES_TOTAL */


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* exception handler's stack */
static unsigned int xExceptionStack[EXCEPTION_STACK_WORDS] = {0};
unsigned int *pxExceptionStack = &xExceptionStack[EXCEPTION_STACK_WORDS-1];

/* exception user configuration area */
static exception_config_t exception_config = {0};
static exception_user_regions_t exception_user_regions = {0};

/* exception reboot configuration area */
static int reboot_flag = 0;

/* assert information area */
static assert_expr_t assert_expr = {0};

/* exception information area */
static exception_info_t exceptionInfo = {0};
exception_info_t *pExceptionInfo = &exceptionInfo;

/* exception context area */
static exception_context_t exceptionContext = {0};
exception_context_t *pExceptionContext = &exceptionContext;

/* exception dump memory regions in region_init.c */
extern memory_region_type memory_regions[];

#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
uint32_t minidump_base_address = 0;
exception_minidump_header_t minidump_header = {0};
#endif /* EXCEPTION_MEMDUMP_MODE */

#if (EXCEPTION_SLAVES_TOTAL > 0)
/* exception slaves' status */
exception_slave_status_t exceptionSlaveStatus[EXCEPTION_SLAVES_TOTAL] = {0};

/* exception slaves' configurations */
#if (PRODUCT_VERSION == 1552)
const exception_slaves_dump_t exceptionSlavesDump[EXCEPTION_SLAVES_TOTAL] =
{
    {"dsp0", exception_alert_dsp0, exception_check_status_dsp0, exception_dump_dsp0, NULL, 1},
    {"dsp1", exception_alert_dsp1, exception_check_status_dsp1, exception_dump_dsp1, NULL, 1},
    {"n9", exception_alert_n9, exception_check_status_n9, exception_dump_n9, exception_forceddump_n9, 1},
    // {0, 0, 0, 0, 0}
};
#else
const exception_slaves_dump_t exceptionSlavesDump[EXCEPTION_SLAVES_TOTAL] =
{
    {0, 0, 0, 0, 0, 0}
}
#endif /* PRODUCT_VERSION */

#endif /* EXCEPTION_SLAVES_TOTAL */


/* Private functions ---------------------------------------------------------*/
/******************************************************************************/
/*            Exception's assert Functions                                    */
/******************************************************************************/
void abort(void)
{
    __asm("cpsid i");
    SCB->CCR |=  SCB_CCR_UNALIGN_TRP_Msk;
    *((volatile unsigned int *) 0xFFFFFFF1) = 1;
    for (;;);
}

void platform_assert(const char *expr, const char *file, int line)
{
    __asm("cpsid i");

#ifdef MTK_BOOTREASON_CHECK_ENABLE
    bootreason_set_flag_assert_reset();
#endif /* MTK_BOOTREASON_CHECK_ENABLE */

    SCB->CCR |=  SCB_CCR_UNALIGN_TRP_Msk;
    assert_expr.is_valid = true;
    assert_expr.expr = expr;
    assert_expr.file = file;
    assert_expr.line = line;
    *((volatile unsigned int *) 0xFFFFFFF1) = 1;
    for (;;);
}

#if defined (__CC_ARM) || defined (__ICCARM__)
void __aeabi_assert(const char *expr, const char *file, int line)
{
    platform_assert(expr, file, line);
}
#endif /* __CC_ARM */

void exception_get_assert_expr(const char **expr, const char **file, int *line)
{
    if (assert_expr.is_valid) {
        *expr = assert_expr.expr;
        *file = assert_expr.file;
        *line = assert_expr.line;
    } else {
        *expr = NULL;
        *file = NULL;
        *line = 0;
    }

}

void exception_print_assert_info(void)
{
    if (assert_expr.is_valid) {
        exception_printf("CM4 assert failed: %s, file: %s, line: %d\r\n",
                        assert_expr.expr,
                        assert_expr.file,
                        (int)assert_expr.line);
    }
}

/******************************************************************************/
/*            Exception's Reboot Functions                                    */
/******************************************************************************/
__weak void exception_reboot(void)
{
    /* It is defined as a weak function.
     * It needs to be implemented in project.
     */
#ifdef HAL_WDT_MODULE_ENABLED
    hal_wdt_config_t wdt_config;
    wdt_config.mode = HAL_WDT_MODE_RESET;
    wdt_config.seconds = 3;
    hal_wdt_disable(HAL_WDT_DISABLE_MAGIC);
    hal_wdt_init(&wdt_config);
    hal_wdt_enable(HAL_WDT_ENABLE_MAGIC);
    // hal_wdt_software_reset();
    while(1);
#endif
    return;
}

void exception_dump_config(int flag)
{
    reboot_flag = flag;
}

static int reboot_check(void)
{
    return reboot_flag;
}

/******************************************************************************/
/*            Exception's regitser callbacks Functions                        */
/******************************************************************************/
exception_status_t exception_register_callbacks(exception_config_type *cb)
{
    int i;

    if (exception_config.items >= EXCEPTION_CONFIGURATIONS_MAX)
    {
       return EXCEPTION_STATUS_ERROR;
    }
    else
    {
       /* check if it is already registered */
       for (i = 0; i < exception_config.items; i++)
       {
           if ( exception_config.configs[i].init_cb == cb->init_cb
             && exception_config.configs[i].dump_cb == cb->dump_cb)
            {
                return EXCEPTION_STATUS_ERROR;
            }
       }
       exception_config.configs[exception_config.items].init_cb = cb->init_cb;
       exception_config.configs[exception_config.items].dump_cb = cb->dump_cb;
       exception_config.items++;
       return EXCEPTION_STATUS_OK;
    }
}

exception_status_t exception_register_regions(memory_region_type *region)
{
    if (exception_user_regions.items >= EXCEPTION_CONFIGURATIONS_MAX)
    {
       return EXCEPTION_STATUS_ERROR;
    }
    else
    {
       exception_user_regions.regions[exception_user_regions.items].region_name = region->region_name;
       exception_user_regions.regions[exception_user_regions.items].start_address = region->start_address;
       exception_user_regions.regions[exception_user_regions.items].end_address = region->end_address;
       exception_user_regions.regions[exception_user_regions.items].is_dumped = region->is_dumped;
       exception_user_regions.items++;
       return EXCEPTION_STATUS_OK;
    }
}

/******************************************************************************/
/*            Exception's minidump Functions                              */
/******************************************************************************/
#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
void printf_dummy(const char *message, ...)
{
}

exception_status_t exception_minidump_region_query_info(uint32_t index, uint32_t *address, uint32_t *length)
{
    uint32_t start_num, end_num = 0;

    if((address == NULL) || (length == NULL) || (index == 0))
    {
        return EXCEPTION_STATUS_ERROR;
    }

    if(true == offline_dump_region_query_seq_range(OFFLINE_REGION_MINI_DUMP, &start_num, &end_num))
    {
        if((end_num >= start_num) || (index <= (end_num - start_num + 1)))
        {
            if(true == offline_dump_region_query_by_seq(OFFLINE_REGION_MINI_DUMP, index + start_num - 1, address, length))
            {
                return EXCEPTION_STATUS_OK;
            }
        }
    }

    return EXCEPTION_STATUS_ERROR;
}

exception_status_t exception_minidump_region_query_count(uint32_t *count)
{
    uint32_t start_num, end_num = 0;

    if(count == NULL)
    {
        return EXCEPTION_STATUS_ERROR;
    }

    if(true == offline_dump_region_query_seq_range(OFFLINE_REGION_MINI_DUMP, &start_num, &end_num))
    {
        if(end_num >= start_num)
        {
            *count = end_num - start_num + 1;
            return EXCEPTION_STATUS_OK;
        }
    }

    return EXCEPTION_STATUS_ERROR;
}

exception_status_t exception_minidump_get_assert_info(uint32_t address, char **file, uint32_t *line)
{
    exception_minidump_header_t *header = NULL;

    if(address == 0)
    {
        return EXCEPTION_STATUS_ERROR;
    }

    header = (exception_minidump_header_t *)(address + sizeof(offline_dump_header_t));
    *file = (char *)((header->assert_info).file);
    *line = (header->assert_info).line;

    /* Check if the file is in Flash region */
    if(((uint32_t)(*file) > (ROM_BASE + ROM_LENGTH)) || ((uint32_t)(*file) < ROM_BASE))
    {
        return EXCEPTION_STATUS_ERROR;
    }

    return EXCEPTION_STATUS_OK;
}

exception_status_t exception_minidump_get_context_info(uint32_t address, uint8_t **context_address, uint32_t *size)
{
    if(address == 0)
    {
        return EXCEPTION_STATUS_ERROR;
    }

    *context_address = (uint8_t *)(address + sizeof(offline_dump_header_t) + MINIDUMP_HEADER_LENGTH);
    *size = sizeof(exception_context_t);

    return EXCEPTION_STATUS_OK;
}

exception_status_t exception_minidump_get_stack_info(uint32_t address, uint8_t **stack_address, uint32_t *size)
{
    exception_context_t *context = NULL;

    if(address == 0)
    {
        return EXCEPTION_STATUS_ERROR;
    }

    context = (exception_context_t *)(address + sizeof(offline_dump_header_t) + MINIDUMP_HEADER_LENGTH);
    if(context->sp == context->msp)
    {
        *stack_address = (uint8_t *)(address
                         + sizeof(offline_dump_header_t)
                         + MINIDUMP_HEADER_LENGTH
                         + MINIDUMP_CONTEXT_LENGTH
                         + MINIDUMP_PSPSTACK_LENGTH);
        *size = MINIDUMP_MSPSTACK_LENGTH;
    }
    else if(context->sp == context->psp)
    {
        *stack_address = (uint8_t *)(address
                         + sizeof(offline_dump_header_t)
                         + MINIDUMP_HEADER_LENGTH
                         + MINIDUMP_CONTEXT_LENGTH);
        *size = MINIDUMP_PSPSTACK_LENGTH;
    }
    else
    {
        return EXCEPTION_STATUS_ERROR;
    }

    return EXCEPTION_STATUS_OK;
}

#endif /* EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP */

/******************************************************************************/
/*            Exception's init Functions                                      */
/******************************************************************************/
ATTR_TEXT_IN_RAM void exception_init(void)
{
    uint32_t i = 0;

    SCB->CCR &= ~SCB_CCR_UNALIGN_TRP_Msk;

#if (configUSE_FLASH_SUSPEND == 1)
    Flash_ReturnReady();
#endif

#if EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP
    /* disable log service for safe */
    log_global_turn_off();
#endif /* EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP */

#ifdef MTK_SYSTEM_HANG_TRACER_ENABLE
    systemhang_exception_enter_trace();
#endif /* MTK_SYSTEM_HANG_TRACER_ENABLE */

#if EXCEPTION_MEMDUMP_MODE != EXCEPTION_MEMDUMP_MINIDUMP
#ifdef MTK_SWLA_ENABLE
    /* add a record for exception to mark the end of the last thread */
    const uint8_t ucExceptionRec[5] = "excp";
    const uint32_t xExceptionRec = (uint32_t)(ucExceptionRec[0] | (ucExceptionRec[1] << 8) | (ucExceptionRec[2] << 16) | (ucExceptionRec[3] << 24));
    SLA_RamLogging(xExceptionRec);
#endif /* MTK_SWLA_ENABLE */
#endif /* EXCEPTION_MEMDUMP_MODE != EXCEPTION_MEMDUMP_MINIDUMP */

#ifdef MTK_BOOTREASON_CHECK_ENABLE
    bootreason_set_flag_exception_reset();
#endif /* MTK_BOOTREASON_CHECK_ENABLE */

    /* Update Exception Number */
    exceptionInfo.count += 1;

#if (EXCEPTION_SLAVES_TOTAL > 0)
    /* Give exception hw semaphore for double exceptions */
    hal_hw_semaphore_give(EXCEPTION_HW_SEMAPHORE);
    /* Take Exception HW semaphore */
    while(HAL_HW_SEMAPHORE_STATUS_OK != hal_hw_semaphore_take(EXCEPTION_HW_SEMAPHORE));

    /* Trigger other Core's exception */
    for (i = 0; i < EXCEPTION_SLAVES_TOTAL; i++)
    {
        if ((exceptionSlavesDump[i].is_dump != 0) &&
            (exceptionSlavesDump[i].slave_alert != NULL))
        {
            exceptionSlavesDump[i].slave_alert();
        }
    }
#endif /* EXCEPTION_SLAVES_TOTAL > 0 */

    /* Get current time stamp */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M , &(exceptionInfo.timestamp));

#if EXCEPTION_MEMDUMP_MODE != EXCEPTION_MEMDUMP_MINIDUMP
    for (i = 0; i < exception_config.items; i++)
    {
        if (exception_config.configs[i].init_cb)
        {
            exception_config.configs[i].init_cb();
        }
    }
#else
    exception_dump_config(DISABLE_WHILELOOP_MAGIC);

#ifdef MTK_FOTA_VIA_RACE_CMD
    /* clean FOTA status to prevent FOTA upgrade after reboot */
    fota_multi_info_sector_reset();
#endif /* MTK_FOTA_VIA_RACE_CMD */
#endif /* EXCEPTION_MEMDUMP_MODE != EXCEPTION_MEMDUMP_MINIDUMP */

#if (EXCEPTION_SLAVES_TOTAL > 0)
    /* Give exception hw semaphore */
    hal_hw_semaphore_give(EXCEPTION_HW_SEMAPHORE);
#endif /* EXCEPTION_SLAVES_TOTAL > 0 */
}

/******************************************************************************/
/*            Exception's context dump Functions                              */
/******************************************************************************/
static void exception_dump_context(uint32_t stack[])
{
    /* Context Dump */
#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_TEXT) || (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_BINARY)
    exceptionContext.r0   = stack[r0];
    exceptionContext.r1   = stack[r1];
    exceptionContext.r2   = stack[r2];
    exceptionContext.r3   = stack[r3];
    exceptionContext.r12  = stack[r12];
    exceptionContext.sp   = ((uint32_t)stack) + 0x20;
    exceptionContext.lr   = stack[lr];
    exceptionContext.pc   = stack[pc];
    exceptionContext.psr  = stack[psr];

    /* FPU context? */
    if ( (exceptionContext.exc_return & 0x10) == 0 )
    {
        exceptionContext.s0 = stack[s0];
        exceptionContext.s1 = stack[s1];
        exceptionContext.s2 = stack[s2];
        exceptionContext.s3 = stack[s3];
        exceptionContext.s4 = stack[s4];
        exceptionContext.s5 = stack[s5];
        exceptionContext.s6 = stack[s6];
        exceptionContext.s7 = stack[s7];
        exceptionContext.s8 = stack[s8];
        exceptionContext.s9 = stack[s9];
        exceptionContext.s10 = stack[s10];
        exceptionContext.s11 = stack[s11];
        exceptionContext.s12 = stack[s12];
        exceptionContext.s13 = stack[s13];
        exceptionContext.s14 = stack[s14];
        exceptionContext.s15 = stack[s15];
        exceptionContext.fpscr = stack[fpscr];
        exceptionContext.sp += 72; /* s0-s15, fpsr, reserved */
    }

    /* if CCR.STKALIGN=1, check PSR[9] to know if there is forced stack alignment */
    if ( (SCB->CCR & SCB_CCR_STKALIGN_Msk) && (exceptionContext.psr & 0x200))
    {
        exceptionContext.sp += 4;
    }

    exception_printf("\r\nCM4 Register dump begin:\r\n");
    exception_printf("r0  = 0x%08x\r\n", exceptionContext.r0);
    exception_printf("r1  = 0x%08x\r\n", exceptionContext.r1);
    exception_printf("r2  = 0x%08x\r\n", exceptionContext.r2);
    exception_printf("r3  = 0x%08x\r\n", exceptionContext.r3);
    exception_printf("r4  = 0x%08x\r\n", exceptionContext.r4);
    exception_printf("r5  = 0x%08x\r\n", exceptionContext.r5);
    exception_printf("r6  = 0x%08x\r\n", exceptionContext.r6);
    exception_printf("r7  = 0x%08x\r\n", exceptionContext.r7);
    exception_printf("r8  = 0x%08x\r\n", exceptionContext.r8);
    exception_printf("r9  = 0x%08x\r\n", exceptionContext.r9);
    exception_printf("r10 = 0x%08x\r\n", exceptionContext.r10);
    exception_printf("r11 = 0x%08x\r\n", exceptionContext.r11);
    exception_printf("r12 = 0x%08x\r\n", exceptionContext.r12);
    exception_printf("lr  = 0x%08x\r\n", exceptionContext.lr);
    exception_printf("pc  = 0x%08x\r\n", exceptionContext.pc);
    exception_printf("psr = 0x%08x\r\n", exceptionContext.psr);
    exception_printf("EXC_RET = 0x%08x\r\n", exceptionContext.exc_return);

    /* update CONTROL.SPSEL and psp if returning to thread mode */
    if (exceptionContext.exc_return & 0x4)
    {
        exceptionContext.control |= 0x2; /* CONTROL.SPSel */
        exceptionContext.psp = exceptionContext.sp;
    }
    else /* update msp if returning to handler mode */
    {
        exceptionContext.msp = exceptionContext.sp;
    }

    /* FPU context? */
    if ( (exceptionContext.exc_return & 0x10) == 0 )
    {
        exceptionContext.control |= 0x4; /* CONTROL.FPCA */
        exception_printf("s0  = 0x%08x\r\n", exceptionContext.s0);
        exception_printf("s1  = 0x%08x\r\n", exceptionContext.s1);
        exception_printf("s2  = 0x%08x\r\n", exceptionContext.s2);
        exception_printf("s3  = 0x%08x\r\n", exceptionContext.s3);
        exception_printf("s4  = 0x%08x\r\n", exceptionContext.s4);
        exception_printf("s5  = 0x%08x\r\n", exceptionContext.s5);
        exception_printf("s6  = 0x%08x\r\n", exceptionContext.s6);
        exception_printf("s7  = 0x%08x\r\n", exceptionContext.s7);
        exception_printf("s8  = 0x%08x\r\n", exceptionContext.s8);
        exception_printf("s9  = 0x%08x\r\n", exceptionContext.s9);
        exception_printf("s10 = 0x%08x\r\n", exceptionContext.s10);
        exception_printf("s11 = 0x%08x\r\n", exceptionContext.s11);
        exception_printf("s12 = 0x%08x\r\n", exceptionContext.s12);
        exception_printf("s13 = 0x%08x\r\n", exceptionContext.s13);
        exception_printf("s14 = 0x%08x\r\n", exceptionContext.s14);
        exception_printf("s15 = 0x%08x\r\n", exceptionContext.s15);
        exception_printf("s16 = 0x%08x\r\n", exceptionContext.s16);
        exception_printf("s17 = 0x%08x\r\n", exceptionContext.s17);
        exception_printf("s18 = 0x%08x\r\n", exceptionContext.s18);
        exception_printf("s19 = 0x%08x\r\n", exceptionContext.s19);
        exception_printf("s20 = 0x%08x\r\n", exceptionContext.s20);
        exception_printf("s21 = 0x%08x\r\n", exceptionContext.s21);
        exception_printf("s22 = 0x%08x\r\n", exceptionContext.s22);
        exception_printf("s23 = 0x%08x\r\n", exceptionContext.s23);
        exception_printf("s24 = 0x%08x\r\n", exceptionContext.s24);
        exception_printf("s25 = 0x%08x\r\n", exceptionContext.s25);
        exception_printf("s26 = 0x%08x\r\n", exceptionContext.s26);
        exception_printf("s27 = 0x%08x\r\n", exceptionContext.s27);
        exception_printf("s28 = 0x%08x\r\n", exceptionContext.s28);
        exception_printf("s29 = 0x%08x\r\n", exceptionContext.s29);
        exception_printf("s30 = 0x%08x\r\n", exceptionContext.s30);
        exception_printf("s31 = 0x%08x\r\n", exceptionContext.s31);
        exception_printf("fpscr = 0x%08x\r\n", exceptionContext.fpscr);
    }

    exception_printf("CONTROL = 0x%08x\r\n", exceptionContext.control);
    exception_printf("MSP     = 0x%08x\r\n", exceptionContext.msp);
    exception_printf("PSP     = 0x%08x\r\n", exceptionContext.psp);
    exception_printf("sp      = 0x%08x\r\n", exceptionContext.sp);
    exception_printf("\r\nCM4 Register dump end:\r\n");

#elif (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
    uint32_t current_addr = 0;
    bool ret = false;

    exceptionContext.r0   = stack[r0];
    exceptionContext.r1   = stack[r1];
    exceptionContext.r2   = stack[r2];
    exceptionContext.r3   = stack[r3];
    exceptionContext.r12  = stack[r12];
    exceptionContext.sp   = ((uint32_t)stack) + 0x20;
    exceptionContext.lr   = stack[lr];
    exceptionContext.pc   = stack[pc];
    exceptionContext.psr  = stack[psr];

    /* FPU context? */
    if ( (exceptionContext.exc_return & 0x10) == 0 )
    {
        exceptionContext.control |= 0x4; /* CONTROL.FPCA */
        exceptionContext.s0 = stack[s0];
        exceptionContext.s1 = stack[s1];
        exceptionContext.s2 = stack[s2];
        exceptionContext.s3 = stack[s3];
        exceptionContext.s4 = stack[s4];
        exceptionContext.s5 = stack[s5];
        exceptionContext.s6 = stack[s6];
        exceptionContext.s7 = stack[s7];
        exceptionContext.s8 = stack[s8];
        exceptionContext.s9 = stack[s9];
        exceptionContext.s10 = stack[s10];
        exceptionContext.s11 = stack[s11];
        exceptionContext.s12 = stack[s12];
        exceptionContext.s13 = stack[s13];
        exceptionContext.s14 = stack[s14];
        exceptionContext.s15 = stack[s15];
        exceptionContext.fpscr = stack[fpscr];
        exceptionContext.sp += 72; /* s0-s15, fpsr, reserved */
    }

    /* if CCR.STKALIGN=1, check PSR[9] to know if there is forced stack alignment */
    if ( (SCB->CCR & SCB_CCR_STKALIGN_Msk) && (exceptionContext.psr & 0x200))
    {
        exceptionContext.sp += 4;
    }

    /* update CONTROL.SPSEL and psp if returning to thread mode */
    if (exceptionContext.exc_return & 0x4)
    {
        exceptionContext.control |= 0x2; /* CONTROL.SPSel */
        exceptionContext.psp = exceptionContext.sp;
    }
    else /* update msp if returning to handler mode */
    {
        exceptionContext.msp = exceptionContext.sp;
    }

    if(minidump_base_address != 0)
    {
        /* dump core registers */
        current_addr = minidump_base_address + MINIDUMP_HEADER_LENGTH;
        ret = offline_dump_region_write(OFFLINE_REGION_MINI_DUMP,
                                        current_addr,
                                        (uint8_t *)&exceptionContext,
                                        sizeof(exceptionContext));
        if(ret != true)
        {
            minidump_base_address = 0;
            return;
        }

        /* dump SCB registers */
        current_addr += sizeof(exceptionContext);
        ret = offline_dump_region_write(OFFLINE_REGION_MINI_DUMP,
                                        current_addr,
                                        (uint8_t *)0xe000e008,
                                        4);
        if(ret != true)
        {
            minidump_base_address = 0;
            return;
        }

        /* dump SCB registers */
        current_addr += 4;
        ret = offline_dump_region_write(OFFLINE_REGION_MINI_DUMP,
                                        current_addr,
                                        (uint8_t *)0xe000ed00,
                                        4 * 16);
        if(ret != true)
        {
            minidump_base_address = 0;
            return;
        }

        minidump_header.context_size = sizeof(exceptionContext) + 17 * 4;
    }

#endif /* EXCEPTION_MEMDUMP_MODE */
}

/******************************************************************************/
/*            Exception's memory dump Functions                               */
/******************************************************************************/
#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_TEXT) || (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_BINARY)
static void exception_dump_region_info(const memory_region_type *static_regions, exception_user_regions_t *user_regions)
{
    uint32_t i = 0;
    unsigned int *current, *end;

    /* static regions */
    for (i = 0; ; i++)
    {
        if ( !static_regions[i].region_name )
        {
           break;
        }
        if ( !static_regions[i].is_dumped )
        {
           continue;
        }
        current = (unsigned int *)((uint32_t)(static_regions[i].start_address) & 0xfffffffc);
        end     = (unsigned int *)(static_regions[i].end_address);
        if(current < end)
        {
            exception_printf("Region-%s: start_address = 0x%x, end_address = 0x%x\r\n", static_regions[i].region_name, (unsigned int)current, (unsigned int)end);
        }
    }

    /* dynamic regions */
    for (i = 0; ((i < user_regions->items) && (i < EXCEPTION_CONFIGURATIONS_MAX)); i++)
    {
        if((user_regions->regions)[i].is_dumped)
        {
            current = (unsigned int *)((uint32_t)((user_regions->regions)[i].start_address) & 0xfffffffc);
            end     = (unsigned int *)((user_regions->regions)[i].end_address);
            if(current < end)
            {
                exception_printf("Region-%s: start_address = 0x%x, end_address = 0x%x\r\n", (user_regions->regions)[i].region_name, (unsigned int)current, (unsigned int)end);
            }
        }
    }
}
#endif /* EXCEPTION_MEMDUMP_MODE */

#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_TEXT)
static void exception_dump_region_data_text(const memory_region_type *static_regions, exception_user_regions_t *user_regions)
{
    uint32_t i = 0;
    unsigned int *current, *end;

    /* static regions */
    for (i = 0; ; i++)
    {
        if ( !static_regions[i].region_name )
        {
           break;
        }
        if ( !static_regions[i].is_dumped )
        {
           continue;
        }
        current = (unsigned int *)((uint32_t)(static_regions[i].start_address) & 0xfffffffc);
        end     = (unsigned int *)(static_regions[i].end_address);
        for (; current < end; current += 4)
        {
            if (*(current + 0) == 0 &&
                *(current + 1) == 0 &&
                *(current + 2) == 0 &&
                *(current + 3) == 0 )
            {
                continue;
            }
            exception_printf("0x%08x: %08x %08x %08x %08x\r\n",
                            (unsigned int)current,
                            *(current + 0),
                            *(current + 1),
                            *(current + 2),
                            *(current + 3));
        }
    }

    /* dynamic regions */
    for (i = 0; ((i < user_regions->items) && (i < EXCEPTION_CONFIGURATIONS_MAX)); i++)
    {
        if((user_regions->regions)[i].is_dumped)
        {
            current = (unsigned int *)((uint32_t)((user_regions->regions)[i].start_address) & 0xfffffffc);
            end     = (unsigned int *)((user_regions->regions)[i].end_address);
            for (; current < end; current += 4)
            {
                if (*(current + 0) == 0 &&
                    *(current + 1) == 0 &&
                    *(current + 2) == 0 &&
                    *(current + 3) == 0 )
                {
                    continue;
                }
                exception_printf("0x%08x: %08x %08x %08x %08x\r\n",
                                (unsigned int)current,
                                *(current + 0),
                                *(current + 1),
                                *(current + 2),
                                *(current + 3));
            }
        }
    }
}

#elif (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_BINARY)
static void exception_dump_region_data_binary(const memory_region_type *static_regions, exception_user_regions_t *user_regions)
{
    uint32_t i = 0;
    unsigned int *current, *end;

    /* static regions */
    for (i = 0; ; i++) {
        if ( !static_regions[i].region_name )
        {
           break;
        }
        if ( !static_regions[i].is_dumped )
        {
           continue;
        }
        current = (unsigned int *)((uint32_t)(static_regions[i].start_address) & 0xfffffffc);
        end     = (unsigned int *)(static_regions[i].end_address);
        if(current < end)
        {
            log_dump_exception_data((const uint8_t *)current, (uint32_t)end - (uint32_t)current);
        }
    }

    /* dynamic regions */
    for (i = 0; ((i < user_regions->items) && (i < EXCEPTION_CONFIGURATIONS_MAX)); i++)
    {
        if((user_regions->regions)[i].is_dumped)
        {
            current = (unsigned int *)((uint32_t)((user_regions->regions)[i].start_address) & 0xfffffffc);
            end     = (unsigned int *)((user_regions->regions)[i].end_address);
            if(current < end)
            {
                log_dump_exception_data((const uint8_t *)current, (uint32_t)end - (uint32_t)current);
            }
        }
    }
}

#elif (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
static exception_status_t exception_minidump_check_address(uint32_t address)
{
    uint32_t i = 0;
    uint32_t start, end;

    for (i = 0; ; i++) {

        if ( !memory_regions[i].region_name ) {
           break;
        }

        start = (uint32_t)(memory_regions[i].start_address);
        end   = (uint32_t)(memory_regions[i].end_address);

        if((address >= start) && (address <= end))
        {
            return EXCEPTION_STATUS_OK;
        }

    }

    return EXCEPTION_STATUS_ERROR;
}

static void exception_dump_region_data_minidump(const memory_region_type *static_regions, exception_user_regions_t *user_regions)
{
    uint32_t current_addr = 0;
    bool ret = false;

    /* static regions */
    if(minidump_base_address != 0)
    {
        /* dump PSP stacks */
        current_addr = minidump_base_address + MINIDUMP_HEADER_LENGTH + MINIDUMP_CONTEXT_LENGTH;
        ret = exception_minidump_check_address(exceptionContext.psp - 100);
        if(ret == EXCEPTION_STATUS_OK)
        {
            ret = offline_dump_region_write(OFFLINE_REGION_MINI_DUMP,
                                            current_addr,
                                            (uint8_t *)(exceptionContext.psp - 100),
                                            MINIDUMP_PSPSTACK_LENGTH);
            if(ret != true)
            {
                minidump_base_address = 0;
                return;
            }
            minidump_header.regions[0].address = exceptionContext.psp - 100;
            minidump_header.regions[0].size = MINIDUMP_PSPSTACK_LENGTH;
        }
        /* PSP is not a valid value, so jump this region and set its begin address as 0xffffffff - MINIDUMP_PSPSTACK_LENGTH */
        else
        {
            minidump_header.regions[0].address = 0xffffffff - MINIDUMP_PSPSTACK_LENGTH;
            minidump_header.regions[0].size = MINIDUMP_PSPSTACK_LENGTH;
        }

        /* dump MSP stacks */
        current_addr += MINIDUMP_PSPSTACK_LENGTH;
        ret = exception_minidump_check_address(exceptionContext.msp - 100);
        if(ret == EXCEPTION_STATUS_OK)
        {
            ret = offline_dump_region_write(OFFLINE_REGION_MINI_DUMP,
                                            current_addr,
                                            (uint8_t *)(exceptionContext.msp - 100),
                                            MINIDUMP_MSPSTACK_LENGTH);
            if(ret != true)
            {
                minidump_base_address = 0;
                return;
            }
            minidump_header.regions[1].address = exceptionContext.msp - 100;
            minidump_header.regions[1].size = MINIDUMP_MSPSTACK_LENGTH;
        }
        /* MSP is not a valid value, so jump this region and set its begin address as 0xffffffff - MINIDUMP_MSPSTACK_LENGTH */
        else
        {
            minidump_header.regions[1].address = 0xffffffff - MINIDUMP_MSPSTACK_LENGTH;
            minidump_header.regions[1].size = MINIDUMP_MSPSTACK_LENGTH;
        }
    }
}

#endif /* EXCEPTION_MEMDUMP_MODE */

static void exception_dump_memory(void)
{
    /* Memory Dump */
#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_TEXT)
    /* Print Regions' information */
    exception_printf("CM4 Regions Information:\r\n");
    exception_dump_region_info(memory_regions, &exception_user_regions);

    /* Print Regions' data */
    exception_dump_region_data_text(memory_regions, &exception_user_regions);

    /* dump cm4 end log */
    exception_printf("\r\nmemory dump completed.\r\n");

#elif (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_BINARY)
    /* Print Regions' information */
    exception_printf("CM4 Regions Information:\r\n");
    exception_dump_region_info(memory_regions, &exception_user_regions);

    /* Print Memory one by one regions */
    exception_printf("CM4 Regions Data:\r\n");
    /* Print Regions' data */
    exception_dump_region_data_binary(memory_regions, &exception_user_regions);

    /* dump cm4 end log */
    exception_printf("\r\nmemory dump completed.\r\n");

#elif (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
    exception_dump_region_data_minidump(memory_regions, &exception_user_regions);

#endif /* EXCEPTION_MEMDUMP_MODE */
}

/******************************************************************************/
/*            Exception's slaves dump Functions                               */
/******************************************************************************/
#if (EXCEPTION_SLAVES_TOTAL > 0)
void exception_dump_slaves(void)
{
    uint32_t slave_current = 0;
    uint32_t slave_finish = 0;
    uint32_t gpt_start_count, gpt_current_count, gpt_duration_count;

    /* zero exceptionSlaveStatus */
    for(slave_current = 0; slave_current < EXCEPTION_SLAVES_TOTAL; slave_current++)
    {
        exceptionSlaveStatus[slave_current] = EXCEPTION_SLAVE_STATUS_IDLE;
    }
    /* check which core is ready */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_start_count);
    while(slave_finish <  EXCEPTION_SLAVES_TOTAL)
    {
        for(slave_current = 0; slave_current < EXCEPTION_SLAVES_TOTAL; slave_current++)
        {
            if(exceptionSlaveStatus[slave_current] == EXCEPTION_SLAVE_STATUS_IDLE)
            {
                if ((exceptionSlavesDump[slave_current].is_dump != 0) &&
                    (exceptionSlavesDump[slave_current].slave_checkstatus != NULL) &&
                    (exceptionSlavesDump[slave_current].slave_dump != NULL))
                {
                    if (exceptionSlavesDump[slave_current].slave_checkstatus() == EXCEPTION_SLAVE_STATUS_READY)
                    {
                        /* Slave is ready */
                        exceptionSlavesDump[slave_current].slave_dump();
                        exceptionSlaveStatus[slave_current] = EXCEPTION_SLAVE_STATUS_FINISH;
                        slave_finish += 1;
                        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_start_count);
                    }
                    else
                    {
                        /* Slave is not ready */
                        exceptionSlaveStatus[slave_current] = EXCEPTION_SLAVE_STATUS_IDLE;
                    }
                }
                else
                {
                    /* Slave configuration is not ok */
                    exceptionSlaveStatus[slave_current] = EXCEPTION_SLAVE_STATUS_ERROR;
                    slave_finish += 1;
                    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_start_count);
                }
            }
        }
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_current_count);
        hal_gpt_get_duration_count(gpt_start_count, gpt_current_count, &gpt_duration_count);
        /* Check if timeout */
        if(EXCEPTION_SLAVES_TIMEOUT < gpt_duration_count)
        {
            for(slave_current = 0; slave_current < EXCEPTION_SLAVES_TOTAL; slave_current++)
            {
                /* Print Slaves' status */
                if ((exceptionSlavesDump[slave_current].is_dump != 0) &&
                    (exceptionSlavesDump[slave_current].slave_checkstatus != NULL) &&
                    (exceptionSlavesDump[slave_current].slave_dump != NULL))
                {
#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_TEXT) || (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_BINARY)
                    exception_printf("[Warning]%s:%d\r\n", exceptionSlavesDump[slave_current].slave_name,
                                                           exceptionSlaveStatus[slave_current]);
#elif (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
#endif /* EXCEPTION_MEMDUMP_MODE */
                }

                /* Do Forced-dump */
                if ((exceptionSlavesDump[slave_current].is_dump != 0) &&
                    (exceptionSlavesDump[slave_current].slave_checkstatus != NULL) &&
                    (exceptionSlavesDump[slave_current].slave_forceddump != NULL) &&
                    (exceptionSlaveStatus[slave_current] != EXCEPTION_SLAVE_STATUS_FINISH))
                {
                    /* Slave needs to be forced to do dump */
                    exceptionSlavesDump[slave_current].slave_forceddump();
                }
            }
            slave_finish = EXCEPTION_SLAVES_TOTAL;
        }
    }
}
#endif /* EXCEPTION_SLAVES_TOTAL */

/******************************************************************************/
/*            Exception's print fault type Functions                          */
/******************************************************************************/
#if (EXCEPTION_MEMDUMP_MODE != EXCEPTION_MEMDUMP_MINIDUMP)
static void printUsageErrorMsg(uint32_t CFSRValue)
{
    exception_printf("Usage fault: ");
    CFSRValue >>= 16; /* right shift to lsb */
    if ((CFSRValue & (1 << 9)) != 0) {
        exception_printf("Divide by zero\r\n");
    }
    if ((CFSRValue & (1 << 8)) != 0) {
        exception_printf("Unaligned access\r\n");
    }
    if ((CFSRValue & (1 << 3)) != 0) {
        exception_printf("Coprocessor error\r\n");
    }
    if ((CFSRValue & (1 << 2)) != 0) {
        exception_printf("Invalid EXC_RETURN\r\n");
    }
    if ((CFSRValue & (1 << 1)) != 0) {
        exception_printf("Invalid state\r\n");
    }
    if ((CFSRValue & (1 << 0)) != 0) {
        exception_printf("Undefined instruction\r\n");
    }
}

static void printMemoryManagementErrorMsg(uint32_t CFSRValue)
{
    exception_printf("Memory Management fault: ");
    CFSRValue &= 0x000000FF; /* mask mem faults */
    if ((CFSRValue & (1 << 5)) != 0) {
        exception_printf("A MemManage fault occurred during FP lazy state preservation\r\n");
    }
    if ((CFSRValue & (1 << 4)) != 0) {
        exception_printf("A derived MemManage fault occurred on exception entry\r\n");
    }
    if ((CFSRValue & (1 << 3)) != 0) {
        exception_printf("A derived MemManage fault occurred on exception return\r\n");
    }
    if ((CFSRValue & (1 << 1)) != 0) { /* Need to check valid bit (bit 7 of CFSR)? */
        exception_printf("Data access violation @0x%08x\r\n", (unsigned int)SCB->MMFAR);
    }
    if ((CFSRValue & (1 << 0)) != 0) {
        exception_printf("MPU or Execute Never (XN) default memory map access violation\r\n");
    }
    if ((CFSRValue & (1 << 7)) != 0) { /* To review: remove this if redundant */
        exception_printf("SCB->MMFAR = 0x%08x\r\n", (unsigned int)SCB->MMFAR );
    }
}

static void printBusFaultErrorMsg(uint32_t CFSRValue)
{
    exception_printf("Bus fault: ");
    CFSRValue &= 0x0000FF00; /* mask bus faults */
    CFSRValue >>= 8;
    if ((CFSRValue & (1 << 5)) != 0) {
        exception_printf("A bus fault occurred during FP lazy state preservation\r\n");
    }
    if ((CFSRValue & (1 << 4)) != 0) {
        exception_printf("A derived bus fault has occurred on exception entry\r\n");
    }
    if ((CFSRValue & (1 << 3)) != 0) {
        exception_printf("A derived bus fault has occurred on exception return\r\n");
    }
    if ((CFSRValue & (1 << 2)) != 0) {
        exception_printf("Imprecise data access error has occurred\r\n");
    }
    if ((CFSRValue & (1 << 1)) != 0) { /* Need to check valid bit (bit 7 of CFSR)? */
        exception_printf("A precise data access error has occurred @x%08x\r\n", (unsigned int)SCB->BFAR);
    }
    if ((CFSRValue & (1 << 0)) != 0) {
        exception_printf("A bus fault on an instruction prefetch has occurred\r\n");
    }
    if ((CFSRValue & (1 << 7)) != 0) { /* To review: remove this if redundant */
        exception_printf("SCB->BFAR = 0x%08x\r\n", (unsigned int)SCB->BFAR );
    }
}
#endif /* EXCEPTION_MEMDUMP_MODE */

/******************************************************************************/
/*            Exception's dump processor Functions                            */
/******************************************************************************/
void exception_dump_preprocess(uint32_t fault_type)
{
#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_TEXT) || (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_BINARY)
    exceptionInfo.reason = fault_type;

    /* Genie start message */
    exception_printf("<<<<<<<< LOG START LOG START LOG START LOG START LOG START <<<<<<<<\r\n");

    exception_printf("\r\nCM4 Fault Dump:\r\n");
    exception_print_assert_info();
    exception_printf("Exception Count = 0x%08x\r\n", (unsigned int)exceptionInfo.count);
    exception_printf("Exception Time = 0x%08x\r\n", (unsigned int)exceptionInfo.timestamp);

    switch(fault_type)
    {
        case EXCEPTION_NMI:
            exception_printf("\r\nIn NMI Handler\r\n");
            break;
        case EXCEPTION_HARD_FAULT:
            exception_printf("\r\nIn Hard Fault Handler\r\n");
            break;
        case EXCEPTION_MEMMANAGE_FAULT:
            exception_printf("\r\nIn MemManage Fault Handler\r\n");
            break;
        case EXCEPTION_BUS_FAULT:
            exception_printf("\r\nIn Bus Fault Handler\r\n");
            break;
        case EXCEPTION_USAGE_FAULT:
            exception_printf("\r\nIn Usage Fault Handler\r\n");
            break;
        case EXCEPTION_DEBUGMON_FAULT:
            exception_printf("\r\nIn Debug Monitor Fault Handler\r\n");
#ifdef HAL_DWT_MODULE_ENABLED
            uint32_t offset, stack_end, is_match;
            /* is task stack overflow? */
            {
                offset = (0x10 * HAL_DWT_3) / 4;
                is_match = ((*(&DWT->FUNCTION0 + offset))& DWT_FUNCTION_MATCHED_Msk)>>DWT_FUNCTION_MATCHED_Pos;
                stack_end = *(&DWT->COMP0 + offset);
                exception_printf("Task stack overflow:%c, stack end:0x%x \r\n",((is_match)? 'Y':'N'),(unsigned int)stack_end);
            }
#endif /* HAL_DWT_MODULE_ENABLED */
            break;
        default:
            exception_printf("\r\nIn Unknow Fault Handler\r\n");
            break;
    }

    exception_printf("SCB->HFSR = 0x%08x\r\n", (unsigned int)SCB->HFSR);
    exception_printf("SCB->CFSR = 0x%08x\r\n", (unsigned int)SCB->CFSR );
    if ((SCB->HFSR & (1 << 30)) != 0)
    {
        exception_printf("Forced Hard Fault\r\n");
    }
    if ((SCB->CFSR & 0xFFFF0000) != 0)
    {
        printUsageErrorMsg(SCB->CFSR);
    }
    if ((SCB->CFSR & 0x0000FF00) != 0 )
    {
        printBusFaultErrorMsg(SCB->CFSR);
    }
    if ((SCB->CFSR & 0x000000FF) != 0 )
    {
        printMemoryManagementErrorMsg(SCB->CFSR);
    }

#elif (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
    exceptionInfo.reason = fault_type;
    minidump_header.reason = fault_type;
    if (assert_expr.is_valid)
    {
        minidump_header.assert_info.is_valid = assert_expr.is_valid;
        minidump_header.assert_info.expr = assert_expr.expr;
        minidump_header.assert_info.file = assert_expr.file;
        minidump_header.assert_info.line = assert_expr.line;
    }
    else
    {
        minidump_header.assert_info.is_valid = false;
    }

#ifdef HAL_DWT_MODULE_ENABLED
    uint32_t offset, stack_end, is_match;

    /* is task stack overflow? */
    offset = (0x10 * HAL_DWT_3) / 4;
    is_match = ((*(&DWT->FUNCTION0 + offset))& DWT_FUNCTION_MATCHED_Msk)>>DWT_FUNCTION_MATCHED_Pos;
    stack_end = *(&DWT->COMP0 + offset);
    if(is_match)
    {
        minidump_header.overflow_info.is_match = true;
        minidump_header.overflow_info.overflow_address = stack_end;
    }
    else
    {
        minidump_header.overflow_info.is_match = false;
    }
#else
    minidump_header.overflow_info.is_match = false;
#endif /* HAL_DWT_MODULE_ENABLED */

    /* minidump init and get the region address */
    // offline_dump_region_init();
    if(offline_dump_region_alloc(OFFLINE_REGION_MINI_DUMP, &minidump_base_address) != true)
    {
        minidump_base_address = 0;
    }

#endif /* EXCEPTION_MEMDUMP_MODE */
}

void exception_dump_postprocess(void)
{
#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_TEXT) || (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_BINARY)
    uint32_t i = 0;

    for (i = 0; i < exception_config.items; i++)
    {
        if (exception_config.configs[i].dump_cb)
        {
            exception_config.configs[i].dump_cb();
        }
    }

    /* Genie complete message */
    exception_printf("<<<<<<<< LOG END LOG END LOG END LOG END LOG END <<<<<<<<\r\n");

#elif (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
    /* write mini dump header */
    if(minidump_base_address != 0)
    {
        minidump_header.name[0]  = 'C';
        minidump_header.name[1]  = 'M';
        minidump_header.name[2]  = '4';
        minidump_header.name[3]  = ' ';
        minidump_header.name[4]  = 'M';
        minidump_header.name[5]  = 'I';
        minidump_header.name[6]  = 'N';
        minidump_header.name[7]  = 'I';
        minidump_header.name[8]  = 'D';
        minidump_header.name[9]  = 'U';
        minidump_header.name[10] = 'M';
        minidump_header.name[11] = 'P';
        minidump_header.name[12] = 'S';
        minidump_header.name[13] = 'U';
        minidump_header.name[14] = 'C';
        minidump_header.name[15] = 'C';
        minidump_header.name[16] = 'E';
        minidump_header.name[17] = 'E';
        minidump_header.name[18] = 'D';
        minidump_header.name[19] = 0;
        minidump_header.length = MINIDUMP_DATA_SIZE_CM4;
        minidump_header.core_num = 1 + EXCEPTION_SLAVES_TOTAL;
        minidump_header.core_size[0] = MINIDUMP_DATA_SIZE_CM4;
#if (PRODUCT_VERSION == 1552)
        minidump_header.core_size[1] = MINIDUMP_DATA_SIZE_DSP0;
        minidump_header.core_size[2] = MINIDUMP_DATA_SIZE_DSP1;
        minidump_header.core_size[3] = MINIDUMP_DATA_SIZE_N9;
#endif /* PRODUCT_VERSION */

        offline_dump_region_write(OFFLINE_REGION_MINI_DUMP,
                                  minidump_base_address,
                                  (uint8_t *)&minidump_header,
                                  sizeof(minidump_header));

        /* update status to minidump control block */
        offline_dump_region_write_end(OFFLINE_REGION_MINI_DUMP, MINIDUMP_TOTAL_SIZE);
    }

#endif /* EXCEPTION_MEMDUMP_MODE */

    if (reboot_check() == DISABLE_WHILELOOP_MAGIC)
    {
        exception_reboot();
    }
}

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/
void exception_cm4_fault_handler(uint32_t stack[], uint32_t fault_type)
{
    /* dump exception time, fault type, etc */
    exception_dump_preprocess(fault_type);

    /* dump the context when the exception happens */
    exception_dump_context(stack);

    /* dump the memory */
    exception_dump_memory();

#if (EXCEPTION_SLAVES_TOTAL > 0)
    /* do the slave dump */
    exception_dump_slaves();
#endif /* EXCEPTION_SLAVES_TOTAL */

    /* finish the dump */
    exception_dump_postprocess();

#if DEBUGGER_ON
    __ASM volatile("BKPT #01");
#else
    while (1);
#endif
}

/******************************************************************************/
/*                   Toolchain Dependent Part                                 */
/******************************************************************************/
#if defined(__GNUC__)

#define __EXHDLR_ATTR__ __attribute__((naked)) ATTR_TEXT_IN_RAM

/**
  * @brief  This function is the common part of exception handlers.
  * @param  r3 holds EXC_RETURN value
  * @retval None
  */
__EXHDLR_ATTR__ void CommonFault_Handler(void)
{
    __asm volatile
    (
        "cpsid i                       \n"     /* disable irq                 */
        "ldr r3, =pxExceptionStack     \n"
        "ldr r3, [r3]                  \n"     /* r3 := pxExceptionStack      */
        "ldr r0, =pExceptionContext    \n"
        "ldr r0, [r0]                  \n"     /* r0 := pExceptionContext          */
        "add r0, r0, #16               \n"     /* point to context.r4         */
        "stmia r0!, {r4-r11}           \n"     /* store r4-r11                */
        "mov r5, r12                   \n"     /* r5 := EXC_RETURN            */
        "add r0, r0, #20               \n"     /* point to context.control    */
        "mrs r1, control               \n"     /* move CONTROL to r1          */
        "str r1, [r0], #4              \n"     /* store CONTROL               */
        "str r5, [r0], #4              \n"     /* store EXC_RETURN            */
        "mrs r4, msp                   \n"     /* r4 := MSP                   */
        "str r4, [r0], #4              \n"     /* store MSP                   */
        "mrs r1, psp                   \n"     /* move PSP to r1              */
        "str r1, [r0]                  \n"     /* store PSP                   */
        "tst r5, #0x10                 \n"     /* FPU context?                */
        "itt eq                        \n"
        "addeq r0, r0, #68             \n"     /* point to contex.s16         */
        "vstmeq r0, {s16-s31}          \n"     /* store r16-r31               */
        "cmp r3, #0                    \n"     /* if (!pxExceptionStack)      */
        "it ne                         \n"
        "movne sp, r3                  \n"     /* update msp                  */
        "push {lr}                     \n"
        "bl exception_init             \n"
        "pop {lr}                      \n"
        "tst r5, #4                    \n"     /* thread or handler mode?     */
        "ite eq                        \n"
        "moveq r0, r4                  \n"
        "mrsne r0, psp                 \n"
        "bx lr                         \n"
    );
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
__EXHDLR_ATTR__ void NMI_Handler(void)
{
    __asm volatile
    (
// #if defined(HAL_WDT_MODULE_ENABLED)
//         "ldr r0, =s_hal_wdt_callback   \n"
//         "ldr r0, [r0]                  \n"
//         "cbnz r0, enter_wdt_isr        \n"
// #endif
        "mov r12, lr                   \n"
        "bl CommonFault_Handler        \n"
        "mov r1, #2                    \n"
        "bl exception_cm4_fault_handler\n"
// #if defined(HAL_WDT_MODULE_ENABLED)
//         "b  .                          \n"
//         "enter_wdt_isr:                \n"
//         "push {lr}                     \n"
//         "bl hal_wdt_isr                \n"
//         "pop {lr}                      \n"
//         "bx r14                        \n"
// #endif
    );
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
__EXHDLR_ATTR__ void HardFault_Handler(void)
{
    __asm volatile
    (
        "mov r12, lr                   \n"
        "bl CommonFault_Handler        \n"
        "mov r1, #3                    \n"
        "bl exception_cm4_fault_handler\n"
    );
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
__EXHDLR_ATTR__ void MemManage_Handler(void)
{
    __asm volatile
    (
        "mov r12, lr                   \n"
        "bl CommonFault_Handler        \n"
        "mov r1, #4                    \n"
        "bl exception_cm4_fault_handler\n"
    );
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
__EXHDLR_ATTR__ void BusFault_Handler(void)
{
    __asm volatile
    (
        "mov r12, lr                   \n"
        "bl CommonFault_Handler        \n"
        "mov r1, #5                    \n"
        "bl exception_cm4_fault_handler\n"
    );
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
__EXHDLR_ATTR__ void UsageFault_Handler(void)
{
    __asm volatile
    (
        "mov r12, lr                   \n"
        "bl CommonFault_Handler        \n"
        "mov r1, #6                    \n"
        "bl exception_cm4_fault_handler\n"
    );
}

#ifdef HAL_DWT_MODULE_ENABLED
__EXHDLR_ATTR__  void DebugMon_Handler(void)
{
    __asm volatile
    (
        "mov r12, lr                   \n"
        "bl CommonFault_Handler        \n"
        "mov r1, #12                   \n"
        "bl exception_cm4_fault_handler\n"
    );
}
#endif /* HAL_DWT_MODULE_ENABLED */

#endif /* __GNUC__ */

#if defined (__CC_ARM)

#define __EXHDLR_ATTR__ __asm ATTR_TEXT_IN_RAM

/**
  * @brief  This function is the common part of exception handlers.
  * @param  r3 holds EXC_RETURN value
  * @retval None
  */
__EXHDLR_ATTR__ void CommonFault_Handler(void)
{
    extern pExceptionContext
    extern pxExceptionStack

    PRESERVE8

    cpsid i                       /* disable irq                  */
    ldr r3, =pxExceptionStack
    ldr r3, [r3]                  /* r3 := pxExceptionStack       */
    ldr r0, =pExceptionContext
    ldr r0, [r0]                  /* r0 := pExceptionContext           */
    add r0, r0, #16               /* point to context.r4          */
    stmia r0!, {r4-r11}           /* store r4-r11                 */
    mov r5, r12                   /* r5 := EXC_RETURN             */
    add r0, r0, #20               /* point to context.control     */
    mrs r1, control               /* move CONTROL to r1           */
    str r1, [r0], #4              /* store CONTROL                */
    str r5, [r0], #4              /* store EXC_RETURN             */
    mrs r4, msp                   /* r4 := MSP                    */
    str r4, [r0], #4              /* store MSP                    */
    mrs r1, psp                   /* move PSP to r1               */
    str r1, [r0]                  /* store PSP                    */
    tst r5, #0x10                 /* FPU context?                 */
    itt eq
    addeq r0, r0, #68             /* point to contex.s16          */
    vstmeq r0, {s16-s31}          /* store r16-r31                */
    cmp r3, #0                    /* if (!pxExceptionStack)       */
    it ne
    movne sp, r3                  /* update msp                   */
    push {lr}
    bl __cpp(exception_init)
    pop  {lr}
    tst r5, #4                    /* thread or handler mode?      */
    ite eq
    moveq r0, r4
    mrsne r0, psp
    bx lr
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
__EXHDLR_ATTR__ void HardFault_Handler(void)
{
    PRESERVE8

    mov r12, lr
    bl __cpp(CommonFault_Handler)
    mov r1, #3
    bl __cpp(exception_cm4_fault_handler)
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
__EXHDLR_ATTR__ void MemManage_Handler(void)
{
    PRESERVE8

    mov r12, lr
    bl __cpp(CommonFault_Handler)
    mov r1, #4
    bl __cpp(exception_cm4_fault_handler)
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
__EXHDLR_ATTR__ void BusFault_Handler(void)
{
    PRESERVE8

    mov r12, lr
    bl __cpp(CommonFault_Handler)
    mov r1, #5
    bl __cpp(exception_cm4_fault_handler)
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
__EXHDLR_ATTR__ void UsageFault_Handler(void)
{
    PRESERVE8

    mov r12, lr
    bl __cpp(CommonFault_Handler)
    mov r1, #6
    bl __cpp(exception_cm4_fault_handler)
}

#ifdef HAL_DWT_MODULE_ENABLED
__EXHDLR_ATTR__  void DebugMon_Handler(void)
{
    PRESERVE8

    mov r12, lr
    bl __cpp(CommonFault_Handler)
    mov r1, #12
    bl __cpp(exception_cm4_fault_handler)
}
#endif /* HAL_DWT_MODULE_ENABLED */

#endif /* __CC_ARM */

#if defined (__ICCARM__)

#define __EXHDLR_ATTR__  __stackless

ATTR_TEXT_IN_RAM void CommonFault_Handler(void);
ATTR_TEXT_IN_RAM void HardFault_Handler(void);
ATTR_TEXT_IN_RAM void MemManage_Handler(void);
ATTR_TEXT_IN_RAM void BusFault_Handler(void);
ATTR_TEXT_IN_RAM void UsageFault_Handler(void);
ATTR_TEXT_IN_RAM void DebugMon_Handler(void);

/**
  * @brief  This function is the common part of exception handlers.
  * @param  r3 holds EXC_RETURN value
  * @retval None
  */
__EXHDLR_ATTR__ void CommonFault_Handler(void)
{
    __asm volatile
    (
        "cpsid i                       \n"     /* disable irq                 */
        "mov r3, %0                    \n"     /* r3 := pxExceptionStack      */
        "mov r0, %1                    \n"     /* r0 := pExceptionContext          */
        "add r0, r0, #16               \n"     /* point to context.r4         */
        "stmia r0!, {r4-r11}           \n"     /* store r4-r11                */
        "mov r5, r12                   \n"     /* r5 := EXC_RETURN            */
        "add r0, r0, #20               \n"     /* point to context.control    */
        "mrs r1, control               \n"     /* move CONTROL to r1          */
        "str r1, [r0], #4              \n"     /* store CONTROL               */
        "str r5, [r0], #4              \n"     /* store EXC_RETURN            */
        "mrs r4, msp                   \n"     /* r4 := MSP                   */
        "str r4, [r0], #4              \n"     /* store MSP                   */
        "mrs r1, psp                   \n"     /* move PSP to r1              */
        "str r1, [r0]                  \n"     /* store PSP                   */
        "tst r5, #0x10                 \n"     /* FPU context?                */
        "itt eq                        \n"
        "addeq r0, r0, #68             \n"     /* point to contex.s16         */
        "vstmeq r0, {s16-s31}          \n"     /* store r16-r31               */
        "cmp r3, #0                    \n"     /* if (!pxExceptionStack)      */
        "it ne                         \n"
        "movne sp, r3                  \n"     /* update msp                  */
        "push {lr}                     \n"
        "bl exception_init             \n"
        "pop {lr}                      \n"
        "tst r5, #4                    \n"     /* thread or handler mode?     */
        "ite eq                        \n"
        "moveq r0, r4                  \n"
        "mrsne r0, psp                 \n"
        "bx lr                         \n"
        ::"r"(pxExceptionStack), "r"(pExceptionContext)
    );
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
__EXHDLR_ATTR__ void HardFault_Handler(void)
{
    __asm volatile
    (
        "mov r12, lr                   \n"
        "bl CommonFault_Handler        \n"
        "mov r1, #3                    \n"
        "bl exception_cm4_fault_handler\n"
    );
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
__EXHDLR_ATTR__ void MemManage_Handler(void)
{
    __asm volatile
    (
        "mov r12, lr                   \n"
        "bl CommonFault_Handler        \n"
        "mov r1, #4                    \n"
        "bl exception_cm4_fault_handler\n"
    );
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
__EXHDLR_ATTR__ void BusFault_Handler(void)
{
    __asm volatile
    (
        "mov r12, lr                   \n"
        "bl CommonFault_Handler        \n"
        "mov r1, #5                    \n"
        "bl exception_cm4_fault_handler\n"
    );
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
__EXHDLR_ATTR__ void UsageFault_Handler(void)
{
    __asm volatile
    (
        "mov r12, lr                   \n"
        "bl CommonFault_Handler        \n"
        "mov r1, #6                    \n"
        "bl exception_cm4_fault_handler\n"
    );
}

#ifdef HAL_DWT_MODULE_ENABLED
__EXHDLR_ATTR__  void DebugMon_Handler(void)
{
    __asm volatile
    (
        "mov r12, lr                   \n"
        "bl CommonFault_Handler        \n"
        "mov r1, #12                   \n"
        "bl exception_cm4_fault_handler\n"
    );
}
#endif /* HAL_DWT_MODULE_ENABLED */

#endif /* __ICCARM__ */

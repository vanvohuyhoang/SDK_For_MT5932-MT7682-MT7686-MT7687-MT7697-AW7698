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

#ifndef __EXCEPTION_HANDLER__
#define __EXCEPTION_HANDLER__

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "stdio.h"
#include "stdarg.h"
#include <stdint.h>
#include <string.h>
#include "syslog.h"

#if (PRODUCT_VERSION == 2523 || PRODUCT_VERSION == 2533)
#include "mt2523.h"
#endif /* PRODUCT_VERSION */

#if (PRODUCT_VERSION == 7687) || (PRODUCT_VERSION == 7697)
#include "mt7687.h"
#endif /* PRODUCT_VERSION */

#if (PRODUCT_VERSION == 7686) || (PRODUCT_VERSION == 7682)
#include "mt7686.h"
#endif /* PRODUCT_VERSION */

#if (PRODUCT_VERSION == 5932)
#include "mt7686.h"
#endif /* PRODUCT_VERSION */

#if (PRODUCT_VERSION == 1552)
#include "ab155x.h"
#endif /* PRODUCT_VERSION */


/* Public define -------------------------------------------------------------*/
#define DISABLE_MEMDUMP_MAGIC                           0xdeadbeef
#define DISABLE_WHILELOOP_MAGIC                         0xdeadaaaa

#define EXCEPTION_MEMDUMP_TEXT                          0
#define EXCEPTION_MEMDUMP_BINARY                        1
#define EXCEPTION_MEMDUMP_MINIDUMP                      2

#ifdef MTK_MINIDUMP_ENABLE
#define EXCEPTION_MEMDUMP_MODE                          EXCEPTION_MEMDUMP_MINIDUMP
extern void printf_dummy(const char *message, ...);
#define platform_printf                                 printf_dummy
#define exception_printf                                printf_dummy
#else
#if (PRODUCT_VERSION == 1552)
#define EXCEPTION_MEMDUMP_MODE                          EXCEPTION_MEMDUMP_BINARY
extern int log_print_exception_log(const char *message, ...);
extern void log_dump_exception_data(const uint8_t *data, uint32_t size);
#define platform_printf                                 printf
#define exception_printf                                log_print_exception_log
#else
#define EXCEPTION_MEMDUMP_MODE                          EXCEPTION_MEMDUMP_TEXT
#define platform_printf                                 printf
#define exception_printf                                printf
#endif /* PRODUCT_VERSION */
#endif /* MTK_MINIDUMP_ENABLE */

#if (PRODUCT_VERSION == 5932)
#define configUSE_FLASH_SUSPEND                         0
#else
#define configUSE_FLASH_SUSPEND                         1
#endif /* PRODUCT_VERSION */

#if (PRODUCT_VERSION == 1552)
#define EXCEPTION_HW_SEMAPHORE                          HW_SEMAPHORE_EXCEPTION
#define EXCEPTION_SLAVES_TOTAL                          3
#define EXCEPTION_SLAVES_TIMEOUT                        10000000
#else
#define EXCEPTION_SLAVES_TOTAL                          0
#define EXCEPTION_SLAVES_TIMEOUT                        10000000
#endif /* PRODUCT_VERSION */


/* Public typedef ------------------------------------------------------------*/
typedef enum
{
    EXCEPTION_STATUS_ERROR = 0,
    EXCEPTION_STATUS_OK = 1
} exception_status_t;

typedef struct
{
  uint32_t is_valid;
  const char *expr;
  const char *file;
  uint32_t line;
} assert_expr_t;

typedef struct
{
  char *region_name;
  unsigned int *start_address;
  unsigned int *end_address;
  unsigned int is_dumped;
} memory_region_type;

typedef void (*f_exception_callback_t)(void);

typedef struct
{
    f_exception_callback_t init_cb;
    f_exception_callback_t dump_cb;
} exception_config_type;

#if (EXCEPTION_SLAVES_TOTAL > 0)
typedef enum
{
    EXCEPTION_SLAVE_STATUS_ERROR = -1,
    EXCEPTION_SLAVE_STATUS_IDLE = 0,
    EXCEPTION_SLAVE_STATUS_READY = 1,
    EXCEPTION_SLAVE_STATUS_FINISH = 2
} exception_slave_status_t;
#endif /* EXCEPTION_SLAVES_TOTAL > 0 */


/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
extern uint32_t minidump_base_address;
#endif /* EXCEPTION_MEMDUMP_MODE */


/* Public functions ----------------------------------------------------------*/
void exception_get_assert_expr(const char **expr, const char **file, int *line);
void platform_assert(const char *expr, const char *file, int line);
void exception_dump_config(int flag);
void exception_reboot(void);
exception_status_t exception_register_callbacks(exception_config_type *cb);
exception_status_t exception_register_regions(memory_region_type *region);
#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
exception_status_t exception_minidump_region_query_info(uint32_t index, uint32_t *address, uint32_t *length);
exception_status_t exception_minidump_region_query_count(uint32_t *count);
exception_status_t exception_minidump_get_assert_info(uint32_t address, char **file, uint32_t *line);
exception_status_t exception_minidump_get_context_info(uint32_t address, uint8_t **context_address, uint32_t *size);
exception_status_t exception_minidump_get_stack_info(uint32_t address, uint8_t **stack_address, uint32_t *size);
#endif /* EXCEPTION_MEMDUMP_MODE */

#endif /* #ifndef __EXCEPTION_HANDLER__ */

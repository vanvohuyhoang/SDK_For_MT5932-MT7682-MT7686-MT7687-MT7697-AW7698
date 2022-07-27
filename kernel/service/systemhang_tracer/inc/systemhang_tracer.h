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

#ifndef __SYSTEMHANG_TRACER_H__
#define __SYSTEMHANG_TRACER_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MTK_SYSTEM_HANG_TRACER_ENABLE

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <assert.h>


/* define ---------------------------------------------------------------------*/
#define SYSTEMHANG_PENDSV_COUNT_TOTAL                   (10)
#define SYSTEMHANG_ISR_COUNT_TOTAL                      (10)

#define SYSTEMHANG_RETENTION_INFO_TOTAL                 (5)

#if (PRODUCT_VERSION == 1552)
#define SYSTEMHANG_OS_INT_NUMBER                        (0)
#else
#define SYSTEMHANG_OS_INT_NUMBER                        (0xff)
#endif /* PRODUCT_VERSION */


/* typedef --------------------------------------------------------------------*/
typedef struct
{
  char retentionstatus_backup[SYSTEMHANG_RETENTION_INFO_TOTAL - 1];
} systemhang_tracer_t;


/* macro ----------------------------------------------------------------------*/
/* variables ------------------------------------------------------------------*/
extern uint32_t systemhang_wdt_count;


/* functions ------------------------------------------------------------------*/
extern void systemhang_wdt_config(uint32_t timeout_sec);
extern void systemhang_exception_enter_trace(void);
extern void systemhang_interrupt_enter_trace(uint32_t irq_num, systemhang_tracer_t *tracer);
extern void systemhang_interrupt_exit_trace(uint32_t irq_num, systemhang_tracer_t *tracer);
extern void systemhang_task_enter_trace(void);
extern void systemhang_task_exit_trace(uint32_t *task_psp);
extern void systemhang_tracer_init(uint32_t timeout_sec);
#ifdef MTK_SYSTEM_HANG_TRACER_ENABLE_O2
extern void systemhang_retention_status_set_tag(char tag);
#endif /* MTK_SYSTEM_HANG_TRACER_ENABLE_O2 */


#ifdef __cplusplus
}
#endif

#endif /* MTK_SYSTEM_HANG_TRACER_ENABLE */

#endif /* #ifndef __SYSTEMHANG_TRACER_H__ */

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

#ifndef __BOOTREASON_CHECK_H__
#define __BOOTREASON_CHECK_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MTK_BOOTREASON_CHECK_ENABLE

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <assert.h>


/* define ---------------------------------------------------------------------*/
#define BOOTREASON_EXCEPTION_RESET_OFFSET       0x1
#define BOOTREASON_EXCEPTION_RESET_FLAG         0x65

#define BOOTREASON_ASSERT_RESET_OFFSET          0x2
#define BOOTREASON_ASSERT_RESET_FLAG            0x61

#define BOOTREASON_WDT_SW_RESET_OFFSET          0x3
#define BOOTREASON_WDT_SW_RESET_FLAG            0x77

#define BOOTREASON_FIRST_POWER_ON_FLAG          0x0

#define BOOTREASON_POWER_ON_FLAG                0x1

#define BOOTREASON_WDT_TIMEOUT_RESET_FLAG       0x8

#define BOOTREASON_RESETPIN_RESET_FLAG          0xd


/* typedef --------------------------------------------------------------------*/
typedef enum {
    BOOTREASON_STATUS_ERROR,
    BOOTREASON_STATUS_OK
} bootreason_status_t;

typedef enum {
    BOOTREASON_UNKNOWN,
    BOOTREASON_NORMAL,
    BOOTREASON_ASSERT,
    BOOTREASON_EXCEPTION,
    BOOTREASON_WATCHDOG
} bootreason_reason_t;

typedef struct {
  uint8_t *data;
  uint32_t len;
} bootreason_element_t;

typedef struct {
    bootreason_reason_t  reason;
    bootreason_element_t panic_file;
    bootreason_element_t registers;
    bootreason_element_t stack;
    bootreason_element_t custom;
} bootreason_info_t;


/* macro ----------------------------------------------------------------------*/
/* variables ------------------------------------------------------------------*/
/* functions ------------------------------------------------------------------*/
void bootreason_set_flag_exception_reset(void);
void bootreason_set_flag_assert_reset(void);
void bootreason_set_flag_wdt_sw_reset(void);
void bootreason_init(void);
bootreason_status_t bootreason_get_reason(bootreason_reason_t *reason);
bootreason_status_t bootreason_get_info(bootreason_info_t *info);


#ifdef __cplusplus
}
#endif

#endif /* MTK_BOOTREASON_CHECK_ENABLE */

#endif /* #ifndef __BOOTREASON_CHECK_H__ */

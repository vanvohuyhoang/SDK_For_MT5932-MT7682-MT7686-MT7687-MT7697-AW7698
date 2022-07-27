/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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
 
#ifndef __TRNG_SW_H__
#define __TRNG_SW_H__


#include "hal_platform.h"
#include "hal_nvic.h"
#include "hal_nvic_internal.h"

#ifdef HAL_TRNG_MODULE_ENABLED

/*define the trng time configuration */
#define TRNG_TIME       ((uint32_t)0x030F0801)

#define TIMEOUT_VALUE    4096

#define TRNG_POLLING_TIMEOUT_VALUE 1000000

typedef enum {

    TRNG_GEN_TIMEOUT = -1,
    TRNG_GEN_FAIL    = -2,
    TRNG_GEN_SUCCESS = 0

} Trng_Result;

#define TRNG_INIT  1

#define TRNG_DEINIT  0

#define TRNG_CHECK_AND_SET_BUSY(busy_status)  \
do{ \
    uint32_t saved_mask; \
    hal_nvic_save_and_set_interrupt_mask(&saved_mask); \
    if(trng_init_status == TRNG_INIT){ \
        busy_status = HAL_TRNG_STATUS_ERROR; \
    } else { \
        trng_init_status = TRNG_INIT;  \
        busy_status = HAL_TRNG_STATUS_OK; \
    } \
    hal_nvic_restore_interrupt_mask(saved_mask); \
}while(0)

#define TRNG_SET_IDLE()   \
do{  \
    uint32_t saved_mask; \
    hal_nvic_save_and_set_interrupt_mask(&saved_mask); \
    trng_init_status= TRNG_DEINIT;  \
    hal_nvic_restore_interrupt_mask(saved_mask); \
}while(0)

/* function delcartion */
void  trng_init(void);
Trng_Result trng_config_timeout_limit(uint32_t timeout_value);
Trng_Result trng_enable_mode(bool H_GARO, bool H_RO, bool H_FIRO);
void  trng_start(void);
void  trng_deinit(void);
void trng_test(void);
Trng_Result  trng_get(uint32_t *random_data);
uint32_t  trng_get_random_data(void);
void  trng_stop(void);


#endif /*HAL_TRNG_MODULE_ENABLED*/
#endif


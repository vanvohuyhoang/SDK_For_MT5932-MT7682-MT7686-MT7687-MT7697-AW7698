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
 
#ifndef __HAL_MPU_INTERNAL_H__
#define __HAL_MPU_INTERNAL_H__
#include "hal_mpu.h"

#ifdef HAL_MPU_MODULE_ENABLED


#ifdef __cplusplus
extern "C" {
#endif

typedef union {
    struct {
        uint32_t ENABLE: 1;                     /**< Enables the MPU */
        uint32_t HFNMIENA: 1;                   /**< Enables the operation of MPU during hard fault, NMI, and FAULTMASK handlers */
        uint32_t PRIVDEFENA: 1;                 /**< Enables privileged software access to the default memory map */
        uint32_t _reserved0: 29;                /**< Reserved */
    } b;
    uint32_t w;
} MPU_CTRL_Type;

typedef uint32_t MPU_REGION_EN_Type;

typedef union {
    struct {
        uint32_t REGION: 4;                     /**< MPU region field */
        uint32_t VALID: 1;                      /**< MPU Region Number valid bit */
        uint32_t ADDR: 27;                      /**< Region base address field */
    } b;
    uint32_t w;
} MPU_RBAR_Type;

typedef union {
    struct {
        uint32_t ENABLE: 1;                     /**< Region enable bit */
        uint32_t SIZE: 5;                       /**< Specifies the size of the MPU protection region */
        uint32_t _reserved0: 2;                 /**< Reserved */
        uint32_t SRD: 8;                        /**< Subregion disable bits */
        uint32_t B: 1;                          /**< Memory access attributes */
        uint32_t C: 1;                          /**< Memory access attributes */
        uint32_t S: 1;                          /**< Shareable bit */
        uint32_t TEX: 3;                        /**< Memory access attributes */
        uint32_t _reserved1: 2;                 /**< Reserved */
        uint32_t AP: 3;                         /**< Access permission field */
        uint32_t _reserved2: 1;                 /**< Reserved */
        uint32_t XN: 1;                         /**< Instruction access disable bit */
        uint32_t _reserved3: 3;                 /**< Reserved */
    } b;
    uint32_t w;
} MPU_RASR_Type;

typedef struct {
    MPU_RBAR_Type mpu_rbar;
    MPU_RASR_Type mpu_rasr;
} MPU_ENTRY_Type;


extern MPU_CTRL_Type g_mpu_ctrl;
extern MPU_REGION_EN_Type g_mpu_region_en;
extern MPU_ENTRY_Type g_mpu_entry[HAL_MPU_REGION_MAX];

/* Save MPU status before entering deepsleep */
void mpu_status_save(void);

/* Restore MPU status after leaving deepsleep */
void mpu_status_restore(void);


#ifdef __cplusplus
}
#endif

#endif /* HAL_MPU_MODULE_ENABLED */
#endif /* __HAL_MPU_INTERNAL_H__ */


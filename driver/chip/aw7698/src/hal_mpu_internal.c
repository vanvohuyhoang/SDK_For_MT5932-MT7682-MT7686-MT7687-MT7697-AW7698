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
 
#include "hal_mpu.h"

#ifdef HAL_MPU_MODULE_ENABLED

#include "hal_mpu_internal.h"
#include "memory_attribute.h"

#ifdef __cplusplus
extern "C" {
#endif


ATTR_RWDATA_IN_TCM MPU_CTRL_Type g_mpu_ctrl;
ATTR_RWDATA_IN_TCM MPU_REGION_EN_Type g_mpu_region_en;
ATTR_RWDATA_IN_TCM MPU_ENTRY_Type g_mpu_entry[HAL_MPU_REGION_MAX];

ATTR_TEXT_IN_TCM void mpu_status_save(void)
{

}

/* restores only regions that are enabled before entering into deepsleep */
ATTR_TEXT_IN_TCM void mpu_status_restore(void)
{
    MPU_REGION_EN_Type mpu_region_en;
    hal_mpu_region_t region;

    mpu_region_en = g_mpu_region_en;
    for (region = HAL_MPU_REGION_0; region < HAL_MPU_REGION_MAX; region ++) {
        if (mpu_region_en & 1) {
            /* If the region is enabled, restore the previous setting of the corresponding region*/
            MPU->RBAR = g_mpu_entry[region].mpu_rbar.w;
            MPU->RASR = g_mpu_entry[region].mpu_rasr.w;
        }
        mpu_region_en = mpu_region_en >> 1;
    }
    MPU->CTRL = g_mpu_ctrl.w;
}


#ifdef __cplusplus
}
#endif

#endif /* HAL_MPU_MODULE_ENABLED */


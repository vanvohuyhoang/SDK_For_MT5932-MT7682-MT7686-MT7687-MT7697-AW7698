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
 
#ifndef HAL_EMI_INTERNAL_H
#define HAL_EMI_INTERNAL_H

#include <memory_attribute.h>

typedef enum emi_clock_enum_t
{
    EMI_CLK_LOW_TO_HIGH = 0,
    EMI_CLK_HIGH_TO_LOW  = 1,
} emi_clock;

typedef struct {
    uint32_t EMI_RDCT_VAL;
    uint32_t EMI_DSRAM_VAL;
    uint32_t EMI_MSRAM0_VAL;
	uint32_t EMI_MSRAM1_VAL;
    uint32_t EMI_IDL_C_VAL;
    uint32_t EMI_IDL_D_VAL;
    uint32_t EMI_IDL_E_VAL;
    uint32_t EMI_ODL_C_VAL;
    uint32_t EMI_ODL_D_VAL;
    uint32_t EMI_ODL_E_VAL;
    uint32_t EMI_ODL_F_VAL;
    uint32_t EMI_IO_A_VAL;
    uint32_t EMI_IO_B_VAL;

} EMI_SETTINGS;


typedef enum{
    FILTER_LENGTH_521T = 0,
	FILTER_LENGTH_1024T = 1,
	FILTER_LENGTH_2048T = 2,
	FILTER_LENGTH_4096T = 3,
}bandwidth_filter_length_t;



ATTR_TEXT_IN_TCM int32_t EMI_DynamicClockSwitch(emi_clock clock);
ATTR_TEXT_IN_TCM void mtk_psram_half_sleep_enter(void);
ATTR_TEXT_IN_TCM void mtk_psram_half_sleep_exit(void);
ATTR_TEXT_IN_TCM void EMI_Setting_restore(void);
ATTR_TEXT_IN_TCM void EMI_Setting_Save(void);
ATTR_TEXT_IN_TCM int8_t custom_setEMI(void);
ATTR_TEXT_IN_TCM int8_t custom_setAdvEMI(void);
ATTR_TEXT_IN_TCM void emi_mask_master(void);
ATTR_TEXT_IN_TCM void emi_unmask_master(void);


ATTR_TEXT_IN_TCM uint32_t __EMI_GetSR(uint32_t bank_no);
ATTR_TEXT_IN_TCM void __EMI_SetSR(uint32_t bank_no, uint32_t value);


#endif

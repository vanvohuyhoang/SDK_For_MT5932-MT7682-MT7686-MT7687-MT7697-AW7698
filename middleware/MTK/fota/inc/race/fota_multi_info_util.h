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
 
#ifndef __FOTA_MULTI_INFO_UTIL_H__
#define __FOTA_MULTI_INFO_UTIL_H__

#include "fota_util.h"


#define FOTA_MULTI_INFO_STATE_RECORD_LEN               (2)
#define FOTA_MULTI_INFO_UPGRADE_FLAG_RECORD_LEN        (2)
#define FOTA_MULTI_INFO_DL_INTEGRITY_RES_RECORD_LEN    (2)
#define FOTA_MULTI_INFO_VERSION_RECORD_LEN             (FOTA_VERSION_MAX_SIZE + FOTA_VERSION_INTEGRITY_CHECK_VALUE_SIZE)
#define FOTA_MULTI_INFO_RECORD_MAX_LEN    (FOTA_MULTI_INFO_VERSION_RECORD_LEN)
/* Least common multiple of all record_len */
#define FOTA_MULTI_INFO_RECORD_LEN_LCM    (FOTA_MULTI_INFO_VERSION_RECORD_LEN)


typedef enum
{
    FOTA_MULTI_INFO_TYPE_NONE = 0x00,

    FOTA_MULTI_INFO_TYPE_STATE = 0x01,
    FOTA_MULTI_INFO_TYPE_UPGRADE_FLAG = 0x02,
    FOTA_MULTI_INFO_TYPE_DL_INTEGRITY_RES = 0x04,
    FOTA_MULTI_INFO_TYPE_VERSION = 0x08,

    /* Add new info type before this line and update below line. */
    FOTA_MULTI_INFO_TYPE_NULL = 0x10,

    FOTA_MULTI_INFO_TYPE_ALL = 0xFFFF
}fota_multi_info_type_enum;


void fota_multi_info_sector_init(void);

FOTA_ERRCODE fota_multi_info_sector_reset(void);

FOTA_ERRCODE fota_multi_info_sector_clean(uint16_t not_wb_info_types, bool *erase_done);

FOTA_ERRCODE fota_multi_info_read(fota_multi_info_type_enum info_type,
                                      uint8_t *record_data,
                                      uint8_t record_len);


FOTA_ERRCODE fota_multi_info_write(fota_multi_info_type_enum info_type,
                                        uint8_t *record_data,
                                        uint8_t record_len);

#endif // __FOTA_MULTI_INFO_UTIL_H__


/* Copyright Statement:
 *
 * (C) 2018  Airoha Technology Corp. All rights reserved.
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

#ifndef __NVKEY_H__
#define __NVKEY_H__

#if (PRODUCT_VERSION == 1552)
#include "nvdm.h"

/** @brief This enum defines return type of NVKEY APIs. */
typedef enum {
    NVKEY_STATUS_INVALID_PARAMETER = -5,  /**< The user parameter is invalid. */
    NVKEY_STATUS_ITEM_NOT_FOUND = -4,     /**< The data item wasn't found by the NVKEY. */
    NVKEY_STATUS_INSUFFICIENT_SPACE = -3, /**< No space is available in the flash. */
    NVKEY_STATUS_INCORRECT_CHECKSUM = -2, /**< The NVKEY found a checksum error when reading the data item. */
    NVKEY_STATUS_ERROR = -1,              /**< An unknown error occurred. */
    NVKEY_STATUS_OK = 0,                  /**< The operation was successful. */
} nvkey_status_t;

nvkey_status_t nvkey_read_data(uint16_t id, uint8_t *buffer, uint32_t *size);

nvkey_status_t nvkey_write_data(uint16_t id, const uint8_t *buffer, uint32_t size);

#ifdef SYSTEM_DAEMON_TASK_ENABLE
nvkey_status_t nvkey_write_data_non_blocking(uint16_t id,
        const uint8_t *buffer,
        uint32_t size,
        const nvdm_user_callback_t callback,
        const void *user_data);
#endif

nvkey_status_t nvkey_data_item_length(uint16_t id, uint32_t *size);
#endif

#endif /* __NVKEY_H__ */

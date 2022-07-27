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
 
#include "hal_md5.h"

#ifdef HAL_MD5_MODULE_ENABLED

#include "stddef.h"
#include "crypt_md5.h"
#include "hal_log.h"
#include "hal_crypt_internal.h"

hal_md5_status_t hal_md5_init(hal_md5_context_t *context)
{
    if (NULL == context) {
        log_hal_error("context is NULL.");
        return HAL_MD5_STATUS_ERROR;
    }

    if (hal_crypt_lock_take() == -100)
        return HAL_MD5_STATUS_BUSY;

    crypt_md5_init((MD5_CTX_STRUC *)context);

    hal_crypt_lock_give();
    return HAL_MD5_STATUS_OK;
}

hal_md5_status_t hal_md5_append(hal_md5_context_t *context, uint8_t *message, uint32_t length)
{
    if (NULL == context) {
        log_hal_error("context is NULL.");
        return HAL_MD5_STATUS_ERROR;
    }

    if (hal_crypt_lock_take() == -100)
        return HAL_MD5_STATUS_BUSY;

    while (1) {
        if (length >= MAX_APPEND_LENGTH) {
            crypt_md5_append((MD5_CTX_STRUC *)context, message, MAX_APPEND_LENGTH);
            message += MAX_APPEND_LENGTH;
            length -= MAX_APPEND_LENGTH;
        } else {
            crypt_md5_append((MD5_CTX_STRUC *)context, message, length);
            break;
        }
    }

    hal_crypt_lock_give();
    return HAL_MD5_STATUS_OK;
}

hal_md5_status_t hal_md5_end(hal_md5_context_t *context, uint8_t digest_message[HAL_MD5_DIGEST_SIZE])
{
    if (NULL == context) {
        log_hal_error("context is NULL.");
        return HAL_MD5_STATUS_ERROR;
    }
    if (NULL == digest_message) {
        log_hal_error("digest_message is NULL.");
        return HAL_MD5_STATUS_ERROR;
    }

    if (hal_crypt_lock_take() == -100)
        return HAL_MD5_STATUS_BUSY;

    crypt_md5_end((MD5_CTX_STRUC *)context, digest_message);

    hal_crypt_lock_give();
    return HAL_MD5_STATUS_OK;
}

#endif /* HAL_MD5_MODULE_ENABLED */


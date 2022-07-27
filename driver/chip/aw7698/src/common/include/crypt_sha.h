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
 
/**
 * @file crypt_sha.h
 *
 * The "secure hash algorithm" family support.
 *
 * Currently, SHA1, SHA224, SHA256, SHA384, SHA512 are supported.
 */

#ifndef __CRYPT_SHA_H__
#define __CRYPT_SHA_H__

#include "stdint.h"
#include "stdbool.h"
#include "hal_sha.h"
#include "hal_crypt_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HAL_SHA_INTERNAL_SHA1 = 0,
    HAL_SHA_INTERNAL_SHA224 = 1,
    HAL_SHA_INTERNAL_SHA256 = 2,
    HAL_SHA_INTERNAL_SHA384 = 3,
    HAL_SHA_INTERNAL_SHA512 = 4
} hal_sha_internal_type_t;


extern void delay_time(uint32_t count);

#define SHA1_BLOCK_SIZE      64 /**<  512 bits =  64 bytes */
#define SHA224_BLOCK_SIZE    64 /**<  512 bits =  64 bytes */
#define SHA256_BLOCK_SIZE    64 /**<  512 bits =  64 bytes */
#define SHA384_BLOCK_SIZE   128 /**< 1024 bits = 128 bytes */
#define SHA512_BLOCK_SIZE   128 /**< 1024 bits = 128 bytes */


#define SHA1_DIGEST_SIZE     20 /**<  160 bits = 20 bytes */
#define SHA224_DIGEST_SIZE   28 /**<  224 bits = 28 bytes */
#define SHA256_DIGEST_SIZE   32 /**<  256 bits = 32 bytes */
#define SHA384_DIGEST_SIZE   48 /**<  384 bits = 48 bytes */
#define SHA512_DIGEST_SIZE   64  /**< 512 bits = 64 bytes */


#define MAX_APPEND_LENGTH (256)

extern void delay_time(uint32_t count);

/**
 * SHA support structure - common structure.
 */
typedef struct _SHA_CTX_STRUC {
    uint32_t    HashValue[16];
    uint32_t    MessageLen;             /**< Total size */
    uint8_t  Buff[SHA512_BLOCK_SIZE];
    uint8_t     *Block;
    uint32_t    BlockLen;
} SHA_CTX_STRUC, *PSHA_CTX_STRUC;

#define SHA1_CTX_STRUC SHA_CTX_STRUC
#define PSHA1_CTX_STRUC PSHA_CTX_STRUC
#define SHA224_CTX_STRUC SHA_CTX_STRUC
#define PSHA224_CTX_STRUC PSHA_CTX_STRUC
#define SHA256_CTX_STRUC SHA_CTX_STRUC
#define PSHA256_CTX_STRUC PSHA_CTX_STRUC
#define SHA384_CTX_STRUC SHA_CTX_STRUC
#define PSHA384_CTX_STRUC PSHA_CTX_STRUC
#define SHA512_CTX_STRUC SHA_CTX_STRUC
#define PSHA512_CTX_STRUC PSHA_CTX_STRUC

/**
 * @brief  Switch SHA to polling mode during secure boot.
 *
 */
void crypt_sha_enable_polling_mode();


/**
 * @brief  Switch SHA to interrupt mode.
 *
 */
void crypt_sha_disable_polling_mode();

/**
 * @brief  Initial SHA software structure and hardware setting.
 *
 * @param  [OUT] pSHA_CTX Initial table of SHA1 software architecture
 * @param  [IN] SHA type for operation
 */
hal_sha_status_t crypt_sha_init(SHA_CTX_STRUC *pSHA_CTX, hal_sha_internal_type_t sha_type);


/**
 * @brief  SHA Engine.
 *
 * @param  [IN] pSHA1_CTX SHA1 software architecture
 * @param  [IN] SHA type for operation
 */
void crypt_sha_hash(SHA_CTX_STRUC *pSHA_CTX, hal_sha_internal_type_t sha_type);


/**
 * @brief  The message is appended to block. If block size > 64 bytes, the SHA_Hash will be called.
 *
 * @param  [IN] pSHA1_CTX software architecture for Hash information.
 * @param  [IN] Message Message context
 * @param  [IN] MessageLen The length of message in bytes
 * @param  [IN] SHA type for operation
 */
hal_sha_status_t crypt_sha_append(SHA1_CTX_STRUC   *pSHA_CTX,
                                  uint8_t          Message[],
                                  uint32_t         MessageLen,
                                  hal_sha_internal_type_t sha_type);


/**
 * @brief  1. Append bit 1 to end of the message.
 *         2. Append the length of message in rightmost 64 bits.
 *         3. Transform the Hash Value to digest message
 *
 * @param  [IN]  pSHA_CTX      SHA1 architecture for Hash information.
 * @param  [OUT] DigestMessage Digest message
 * @param  [IN] SHA type for operation
 */
hal_sha_status_t crypt_sha_end(SHA1_CTX_STRUC  *pSHA_CTX,
                               uint8_t         DigestMessage[],
                               hal_sha_internal_type_t sha_type);


#ifdef __cplusplus
}
#endif

#endif /* __CRYPT_SHA_H__ */


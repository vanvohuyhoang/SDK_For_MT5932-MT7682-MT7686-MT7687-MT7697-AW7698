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
 
#ifndef __HAL_LZMA_DECODE_INTERFACE_H__
#define __HAL_LZMA_DECODE_INTERFACE_H__

#include <stddef.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


#define LZMA_OK                   0
#define LZMA_ERROR_DATA           1
#define LZMA_ERROR_MEM            2
#define LZMA_ERROR_CRC            3
#define LZMA_ERROR_UNSUPPORTED    4
#define LZMA_ERROR_PARAM          5
#define LZMA_ERROR_INPUT_EOF      6
#define LZMA_ERROR_OUTPUT_EOF     7
#define LZMA_ERROR_READ           8
#define LZMA_ERROR_WRITE          9
#define LZMA_ERROR_PROGRESS       10
#define LZMA_ERROR_FAIL           11
#define LZMA_ERROR_THREAD         12
#define LZMA_ERROR_ARCHIVE        16
#define LZMA_ERROR_NO_ARCHIVE     17

typedef int LZMA_ret;


typedef struct {
    void *(*Alloc)(void *p, size_t size);
    void (*Free)(void *p, void *address);   /* address can be 0 */
} lzma_alloc_t;

/*
    lzma_decode
    --------------
    In:
        destination                 - the destination for output data
        destination_length          - the maximum limit of output data size
        source                      - compressed input data
        source_length               - compressed input data length
        compression_property        - compression proerty
        compression_property_size   - compression property Size
    Out:
        destination                 - the decompressed output data
        destination_length          - the decompressed output size
        source_length               - decompressed input size
    Returns:
        SZ_OK                       - OK
        SZ_ERROR_DATA               - Data error
        SZ_ERROR_MEM                - Memory allocation arror
        SZ_ERROR_UNSUPPORTED        - Unsupported properties
        SZ_ERROR_INPUT_EOF          - it needs more bytes in input buffer (src)
*/

LZMA_ret lzma_decode (
    uint8_t *destination, 
    uint32_t *destination_length, 
    const uint8_t *source, 
    uint32_t *source_length,
    const uint8_t *compression_property, 
    uint32_t compression_property_size, 
    lzma_alloc_t *lzma_alloc);

/*
    lzma_decode to flash, use for decode from flash to flash
    --------------
    In:

        destination                 - the destination address on flash for output data
        reserved_size               - the reserved size for decompressed data
        source                      - the source address on flash of compressed input data
        lzma_alloc                  - the allocator for memory allocate and free

    Returns:
        SZ_OK                       - OK
        SZ_ERROR_DATA               - Data error
        SZ_ERROR_MEM                - Memory allocation arror
        SZ_ERROR_UNSUPPORTED        - Unsupported properties
        SZ_ERROR_INPUT_EOF          - it needs more bytes in input buffer (src)
*/
LZMA_ret lzma_decode2flash(
    uint8_t *destination,
    uint32_t reserved_size,
    const uint8_t *source,
    lzma_alloc_t *lzma_alloc);

#ifdef __cplusplus
}
#endif

#endif  /* __LZMA_DECODE_INTERFACE_H__ */

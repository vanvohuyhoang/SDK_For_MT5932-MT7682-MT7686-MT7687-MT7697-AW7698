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
 
#ifndef __HAL_EFUSE_H__
#define __HAL_EFUSE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @file hal_efuse.h
 *
 * @brief A common efuse driver.
 */

typedef enum {
    HAL_EFUSE_OK,
    HAL_EFUSE_INVALID_PARAMETER,
    HAL_EFUSE_INVALID_MAGIC,
    HAL_EFUSE_INVALID_ACCESS
} hal_efuse_status_t;


/**
 * Write data into eFuse.
 *
 * Write <i>len</i> bytes of data in buffer <i>buf</i> into eFuse at the
 * specified address <i>addr</i>.
 *
 * @param addr  address to write to.
 * @param buf   the data sequence to be write into eFuse.
 * @param len   the length of the date.
 *
 * @retval HAL_EFUSE_OK if write succeeded.
 * @retval HAL_EFUSE_INVALID_PARAMETER buf is incorrect, address is not supported,
 *         or length is not supported.
 * @retval HAL_EFUSE_INVALID_MAGIC the magic is wrong.
 */
hal_efuse_status_t hal_efuse_write(uint32_t magic, uint16_t addr, const uint8_t *buf, uint8_t len);


/**
 * Read data from eFuse.
 *
 * Read <i>len</i> bytes of data from specified address <i>addr</i> in eFuse
 * into buffer <i>buf</i>.
 *
 * @param addr  address to read from.
 * @param buf   keep the read data sequence into this buffer.
 * @param len   the length of the date.
 *
 * @retval HAL_EFUSE_OK if read succeeded.
 * @retval HAL_EFUSE_INVALID_PARAMETER buf is incorrect, address is not supported,
 *         or length is not supported.
 */
hal_efuse_status_t hal_efuse_read(uint16_t addr, uint8_t *buf, uint8_t len);


#ifdef __cplusplus
}
#endif

#endif /* __HAL_EFUSE_H__ */


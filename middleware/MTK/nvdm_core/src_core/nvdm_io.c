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

#ifdef MTK_NVDM_ENABLE

#include "nvdm.h"
#include "nvdm_port.h"
#include "nvdm_internal.h"
#include "nvdm_msgid_log.h"

#ifdef NVDM_INTERNAL_DEBUG

static uint8_t g_io_buffer[NVDM_BUFFER_SIZE];

extern uint32_t g_nvdm_peb_size;

static int32_t check_pattern(const void *buf, uint8_t patt, int32_t len)
{
    int32_t i;

    for (i = 0; i < len; i++) {
        if (((const uint8_t *)buf)[i] != patt) {
            return -1;
        }
    }

    return 0;
}

static int32_t compare_buffer(const void *buf1, const void *buf2, int32_t len)
{
    int32_t i;

    for (i = 0; i < len; i++) {
        if (((const uint8_t *)buf1)[i] != ((const uint8_t *)buf2)[i]) {
            return -1;
        }
    }

    return 0;
}

#endif

void peb_io_read(int32_t pnum, int32_t offset, uint8_t *buf, int32_t len)
{
    uint32_t addr;

    addr = nvdm_port_get_peb_address(pnum, offset);

    nvdm_port_flash_read(addr, buf, len);
}

void peb_io_write(int32_t pnum, int32_t offset, const uint8_t *buf, int32_t len)
{
    uint32_t addr;

    addr = nvdm_port_get_peb_address(pnum, offset);

    nvdm_port_flash_write(addr, buf, len);

#ifdef NVDM_INTERNAL_DEBUG
    {
        int32_t ret, i, fragment;
        void *io_buffer;

        io_buffer = g_io_buffer;

        /* check whether data has been written correctlly */
        fragment = len / NVDM_BUFFER_SIZE;
        for (i = 0; i < fragment; i++) {
            memset(io_buffer, 0, NVDM_BUFFER_SIZE);
            nvdm_port_flash_read(addr, io_buffer, NVDM_BUFFER_SIZE);
            ret = compare_buffer(io_buffer, &buf[NVDM_BUFFER_SIZE * i], NVDM_BUFFER_SIZE);
            if (ret) {
                nvdm_log_msgid_error(nvdm_046, 3, pnum, offset, len);
                return;
            }
            addr += NVDM_BUFFER_SIZE;
        }
        if (len % NVDM_BUFFER_SIZE) {
            memset(io_buffer, 0, NVDM_BUFFER_SIZE);
            nvdm_port_flash_read(addr, io_buffer, len % NVDM_BUFFER_SIZE);
            ret = compare_buffer(io_buffer, &buf[NVDM_BUFFER_SIZE * fragment], len % NVDM_BUFFER_SIZE);
            if (ret) {
                nvdm_log_msgid_error(nvdm_047, 4, addr, pnum, offset, len);
            }
        }
    }
#endif

}

void peb_erase(int32_t pnum)
{
    uint32_t addr;

    addr = nvdm_port_get_peb_address(pnum, 0);

    nvdm_port_flash_erase(addr);

#ifdef NVDM_INTERNAL_DEBUG
    {
        int32_t ret, i;
        void *io_buffer;

        io_buffer = g_io_buffer;

        /* peb size must be aligned to NVDM_BUFFER_SIZE boundary */
        for (i = 0; i < g_nvdm_peb_size / NVDM_BUFFER_SIZE; i++) {
            nvdm_port_flash_read(addr, io_buffer, NVDM_BUFFER_SIZE);
            ret = check_pattern(io_buffer, 0xFF, NVDM_BUFFER_SIZE);
            if (ret) {
                nvdm_log_msgid_error(nvdm_048, 1, pnum);
                return;
            }
            addr += NVDM_BUFFER_SIZE;
        }
    }
#endif

}

#endif


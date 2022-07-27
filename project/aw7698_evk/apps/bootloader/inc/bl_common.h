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
 
#ifndef __BL_COMMON_H__
#define __BL_COMMON_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BL_SLIM_UDIV_R(N, D, R) (((R)=(N)%(D)), ((N)/(D)))
#define BL_SLIM_UDIV(N, D) ((N)/(D))
#define BL_SLIM_UMOD(N, D) ((N)%(D))

typedef enum {
    BL_BOOTUP_FAIL_REASON_NONE = 0,
    BL_BOOTUP_FAIL_IN_HW_INIT,
    BL_BOOTUP_FAIL_IN_FOTA_UA
} bl_bootup_status_t;

#define BL_PRINT bl_print

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_CRIT,
    LOG_NONE,
} bl_log_level_t;

#ifndef ASSERT
#define ASSERT(x) { if (!(x))  {bl_print(LOG_DEBUG,"assert\n\r"); while (1); } }
#endif

void bl_set_debug_level(bl_log_level_t level);
void bl_print(bl_log_level_t level, char *fmt, ...);
void bl_dbg_print(char *fmt, ...);
uint32_t bl_custom_ram_baseaddr(void);
uint32_t bl_custom_rom_baseaddr(void);
uint32_t bl_custom_cm4_start_address(void);
uint32_t bl_custom_fota_start_address(void);
uint32_t bl_custom_fota_size(void);

#ifdef MTK_SECURE_BOOT_ENABLE
uint32_t bl_custom_header_start_address(void);
uint32_t bl_custom_header_size(void);
#endif

extern void JumpCmd(int);/* from startup_bootloader.s */
extern int custom_setSFIExt(void);

#ifdef __cplusplus
}
#endif

#endif /* __BL_COMMON_H__ */

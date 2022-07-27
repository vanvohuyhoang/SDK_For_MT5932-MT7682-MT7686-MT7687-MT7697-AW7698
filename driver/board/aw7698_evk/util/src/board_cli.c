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
 
#include <stdio.h>
#include <stdint.h>

#include "os.h"

#include "toi.h"
#include "cli.h"
#include "hal_sys.h"
#include "hal_cache.h"
#include "connsys_util.h"
#include "verno.h"


/****************************************************************************
 * Public Functions
 ****************************************************************************/


uint8_t board_cli_ver(uint8_t len, char *param[])
{
    cli_puts("CM4 Image Ver: ");
    cli_puts(MTK_FW_VERSION);
    cli_putln();


#ifdef MTK_CM4_WIFI_TASK_ENABLE
    char fw_ver[32];

    os_memset(fw_ver, 0, 32);

    connsys_util_get_n9_fw_ver(fw_ver);
    cli_puts("N9 Image  Ver: ");
    cli_puts(fw_ver);
    cli_putln();
#endif

#if (PRODUCT_VERSION == 7687 || PRODUCT_VERSION == 7697)
    char fw_ver[32];
    char patch_ver[32];

    os_memset(fw_ver, 0, 32);
    os_memset(patch_ver, 0, 32);

    connsys_util_get_n9_fw_ver(fw_ver);
    cli_puts("N9 Image  Ver: ");
    cli_puts(fw_ver);
    cli_putln();

    connsys_util_get_ncp_patch_ver(patch_ver);
    cli_puts("HW Patch  Ver: ");
    cli_puts(patch_ver);
    cli_putln();
#endif
    return 0;
}


uint8_t board_cli_reboot(uint8_t len, char *param[])
{
    cli_puts("Reboot Bye Bye Bye!!!!\n");

    hal_cache_disable();
    hal_cache_deinit();

    hal_sys_reboot(HAL_SYS_REBOOT_MAGIC, WHOLE_SYSTEM_REBOOT_COMMAND);

    return 0;
}


uint8_t board_cli_reg_read(uint8_t len, char *param[])
{
    uint32_t reg;
    uint32_t val;
    uint8_t  type;

    if (len != 1) {
        printf("reg#\n");
        return 0;
    }

    reg = toi(param[0], &type);

    if (type == TOI_ERR) {
        printf("reg#\n");
    } else {
        val = *((volatile uint32_t *)reg);
        printf("read register 0x%08x (%u) got 0x%08x\n", (unsigned int)reg, (unsigned int)reg, (unsigned int)val);
    }

    return 0;
}


uint8_t board_cli_reg_write(uint8_t len, char *param[])
{
    uint32_t reg;
    uint32_t val;
    uint8_t  type;

    if (len == 2) {
        reg = toi(param[0], &type);
        if (type == TOI_ERR) {
            printf("reg#\n");
            return 0;
        }
        val = toi(param[1], &type);
        if (type == TOI_ERR) {
            printf("val#\n");
            return 0;
        }

        *((volatile uint32_t *)reg) = val;
        printf("written register 0x%08x (%u) as 0x%08x\n", (unsigned int)reg, (unsigned int)reg, (unsigned int)val);
    }

    return 0;
}



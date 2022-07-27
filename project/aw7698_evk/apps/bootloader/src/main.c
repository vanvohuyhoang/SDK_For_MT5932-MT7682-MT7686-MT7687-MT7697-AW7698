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
 
#include "bl_common.h"
#include "bl_fota.h"
#include "hal_uart.h"
#include "hal_flash.h"
#include "core_cm4.h"
#include "hal_emi.h"
#include "hal_clock_internal.h"
#include "hal_dcxo.h"
#include "hal_wdt.h"
#include "hal_cache.h"

#ifdef MTK_SECURE_BOOT_ENABLE
#include "secure_boot.h"
#endif

#ifdef MTK_BOOTLOADER_XIP
#define BL_CACHE_ENABLE
#endif

#ifdef BL_CACHE_ENABLE
/**
* @brief       This function is to initialize cache controller.
* @param[in]   None.
* @return      None.
*/
static void cache_init(void)
{
    hal_cache_region_t region, region_number;
    /* Max region number is 16 */
    hal_cache_region_config_t region_cfg_tbl[] = {
        /* cacheable address, cacheable size(both MUST be 4k bytes aligned) */
        /* bootloader execution at SYSRAM */
        {BL_BASE, BL_LENGTH},
    };
    region_number = (hal_cache_region_t)(sizeof(region_cfg_tbl) / sizeof(region_cfg_tbl[0]));
    hal_cache_init();
    hal_cache_set_size(HAL_CACHE_SIZE_16KB);
    for (region = HAL_CACHE_REGION_0; region < region_number; region++) {
        hal_cache_region_config(region, &region_cfg_tbl[region]);
        hal_cache_region_enable(region);
    }
    for (; region < HAL_CACHE_REGION_MAX; region++) {
        hal_cache_region_disable(region);
    }
    hal_cache_enable();
}
/**
* @brief       This function is to de-initialize cache controller.
* @param[in]   None.
* @return      None.
*/
static void cache_deinit(void)
{
    hal_cache_disable();
    hal_cache_deinit();
}
#endif

/* Placement at TCM for SFC/EMI initialization.
   In XIP case, bl_print should NOT be called between
   hal_clock_set_pll_dcm_init and hal_emi_configure_advanced/custom_setSFIExt*/
ATTR_TEXT_IN_TCM void bl_hardware_init()
{
/* assembly code in startup.s
    cache_init();
 */
    /* To set DCXO frequency for hal_clock_fxo_is_26m */
    hal_clock_init();

    hal_uart_config_t uart_config;

    /* UART init */
    uart_config.baudrate = HAL_UART_BAUDRATE_115200;
    uart_config.parity = HAL_UART_PARITY_NONE;
    uart_config.stop_bit = HAL_UART_STOP_BIT_1;
    uart_config.word_length = HAL_UART_WORD_LENGTH_8;
    hal_uart_init(HAL_UART_0, &uart_config);

    /* LOG level setting */
#ifdef BL_LOG_LEVEL
    bl_set_debug_level(BL_LOG_LEVEL);
#else
    bl_set_debug_level(LOG_DEBUG);
#endif

    /* Enable FPU. Set CP10 and CP11 Full Access.  bl_print_internal in keil uses FPU.*/
    SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2));
    bl_print(LOG_DEBUG,"set CP10 and CP11 Full Access\r\n");

    bl_print(LOG_DEBUG, "bl_uart_init\n\r");

    bl_print(LOG_DEBUG, "hal_emi_configure \n\r");
    hal_emi_configure();

    /* PLL init */
    bl_print(LOG_DEBUG, "hal_clock_set_pll_dcm_init\n\r");
    hal_clock_set_pll_dcm_init();

    hal_emi_configure_advanced();

    /* SFI init required for SFC clock change. */
    custom_setSFIExt();

#if defined(BL_FOTA_ENABLE) || defined(__SERIAL_FLASH_STT_EN__)
    bl_print(LOG_DEBUG, "NOR_init \n\r");
    hal_flash_init();
#endif

    bl_print(LOG_DEBUG, "hal_flash_init \n\r");
    hal_flash_init();

#ifdef BL_DEBUG
    bl_print(LOG_DEBUG, "hf_fsys_ck freq=%d\n\r", hal_clock_get_freq_meter(19, 0));
    bl_print(LOG_DEBUG, "hf_fsfc_ck freq=%d\n\r", hal_clock_get_freq_meter(17, 0));
#endif

    /* enable 30 seconds watchdog timeout. */
    hal_wdt_config_t wdt_config;
    wdt_config.mode = HAL_WDT_MODE_RESET;
    wdt_config.seconds = 30;
    hal_wdt_status_t wdt_ret;

    wdt_ret = hal_wdt_init(&wdt_config);
    if (wdt_ret < 0) {
        bl_print(LOG_ERROR, "hal_wdt_init (%d) \n\r", wdt_ret);
    }
    hal_wdt_enable(HAL_WDT_ENABLE_MAGIC);

    /* SF STT and Disturbance Test*/
#ifdef __SERIAL_FLASH_STT_EN__
    extern void stt_main(void);
    stt_main();
#endif
}

void bl_start_user_code()
{
    uint32_t targetAddr = bl_custom_cm4_start_address();

#if defined(MTK_SECURE_BOOT_ENABLE)
    uint32_t hdrAddr = bl_custom_header_start_address();
    sboot_status_t ret = SBOOT_STATUS_OK;
    bl_print(LOG_DEBUG, "bl_custom_header_start_address = %x\r\n", hdrAddr);

    ret = sboot_secure_boot_check((uint8_t *)hdrAddr, NULL, SBOOT_IOTHDR_V1, 0);
    
    if (ret == SBOOT_STATUS_FAIL) {
        bl_print(LOG_DEBUG, "secure boot check failed. system halt\r\n");
        while(1);
    } else if (ret == SBOOT_STATUS_NOT_ENABLE) {
        bl_print(LOG_DEBUG, "secure boot disabled\r\n");
    } else if(ret == SBOOT_STATUS_OK) {
        bl_print(LOG_DEBUG, "secure boot check pass\r\n");
    }

#endif
    bl_print(LOG_INFO, "Jump to addr %x\n\r", targetAddr);
    JumpCmd(targetAddr);
}

int main()
{
    bl_hardware_init();

#ifdef BL_FOTA_ENABLE
    bl_fota_process();
#endif

    bl_start_user_code();

    return 0;
}

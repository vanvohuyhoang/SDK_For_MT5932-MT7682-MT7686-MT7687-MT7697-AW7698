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

#if defined(MTK_MINICLI_ENABLE)
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "hal_sleep_manager.h"
#include "hal_sleep_manager_internal.h"
#include "hal_gpt.h"
#include "hal_rtc.h"
#include "sleep_manager_cli.h"

#ifdef MTK_CM4_WIFI_TASK_ENABLE
extern uint32_t cli_dtim_sleep_mode;
extern uint32_t MACLPSState;
extern uint32_t MACLPNum;
TimerHandle_t xTimer_dtim_sleep;
uint32_t cli_dtim_mode_tmp;
#endif

static uint8_t _cli_lp_deep_sleep(uint8_t len, char *param[])
{
    printf("_cli_enter_deep_sleep\r\n");

    hal_gpt_delay_ms(100);

    spm_mask_wakeup_source(HAL_SLEEP_MANAGER_WAKEUP_SOURCE_ALL);
    __asm volatile("cpsid i");
    hal_sleep_manager_set_sleep_time(3000);
    hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_SLEEP);
    __asm volatile("cpsie i");

    printf("_cli_leave_deep_sleep\r\n");
    hal_gpt_delay_ms(100);

    return 0;
}

static uint8_t _cli_lp_deep_sleep_sysram_off(uint8_t len, char *param[])
{
    printf("_cli_enter_deep_sleep and SYSRAM OFF\r\n");
    hal_gpt_delay_ms(100);
    /*************************************************************************/
    //*SPM_INFRA_SYSRAM_CONTROL_0 = 0xFFFF;     //INFRA_SYSRAM PD-7682 Need first enter seep sleep!
    //*SPM_SEQUENCER_32K_REG_0 |= 0x10;           //Force on PMU
    spm_control_mtcmos(SPM_MTCMOS_CONN      ,SPM_MTCMOS_PWR_DISABLE);
    spm_control_mtcmos(SPM_MTCMOS_SDIO_SLV  ,SPM_MTCMOS_PWR_DISABLE);
    *SPM_SDIO_SLV_SRAM_PD = 1;
    /*************************************************************************/
    *SPM_INFRA_SYSRAM_CONTROL_0 = 0xFFFF;     //INFRA_SYSRAM PD-7682 Need first enter seep sleep
    spm_mask_wakeup_source(HAL_SLEEP_MANAGER_WAKEUP_SOURCE_ALL);
    __asm volatile("cpsid i");
    hal_sleep_manager_set_sleep_time(3000);
    hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_SLEEP);
    __asm volatile("cpsie i");

    printf("_cli_leave_deep_sleep\r\n");
    hal_gpt_delay_ms(100);

    return 0;
}

static uint8_t _cli_lp_sleep_status(uint8_t len, char *param[])
{
    hal_gpt_delay_ms(200);
    if(hal_sleep_manager_is_sleep_locked()) {
        sleep_management_get_lock_sleep_handle_list();
    } else {
        printf("\r\nAny Sleep Handle not locked\r\n");
    }

    return 0;
}

#ifdef MTK_CM4_WIFI_TASK_ENABLE
static void _cli_lp_dtim_enter_sleep_callback(TimerHandle_t pxTimer)
{
    if ((MACLPSState == 1) && (MACLPNum >= 3)) {
        printf("DTIM enter deep sleep:%d\r\n", (unsigned int)cli_dtim_sleep_mode);
        if (cli_dtim_mode_tmp == 1) {
            cli_dtim_sleep_mode = 2;
        } else {
            /* Enable FW_OWN_BACK_INT interrupt */
            hal_lp_connsys_get_own_enable_int();
            /* Give connsys ownership to N9 */
            hal_lp_connsys_give_n9_own();

            printf("enter cli deep sleep\r\n");

            printf("disable all wakeup source\r\n");
            spm_mask_wakeup_source(HAL_SLEEP_MANAGER_WAKEUP_SOURCE_ALL);
            hal_gpt_delay_ms(25);

            __asm volatile("cpsid i");
            hal_sleep_manager_set_sleep_time(10000);
            hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_SLEEP);
            __asm volatile("cpsie i");

        }
    } else  {
        xTimerStart(xTimer_dtim_sleep, 0);
    }
}

static uint8_t _cli_lp_dtim_sleep(uint8_t len, char *param[])
{
    uint32_t mode = atoi(param[0]);

    xTimer_dtim_sleep = xTimerCreate("DTIM enter sleep mode",       /* Just a text name, not used by the kernel. */
                                     (1 * 50 / portTICK_PERIOD_MS),    /* The timer period in ticks. */
                                     pdFALSE,        /* The timers will auto-reload themselves when they expire. */
                                     NULL,   /* Assign each timer a unique id equal to its array index. */
                                     _cli_lp_dtim_enter_sleep_callback /* Each timer calls the same callback when it expires. */
                                    );

    cli_dtim_mode_tmp = mode;

    if (cli_dtim_mode_tmp == 0) {
        cli_dtim_mode_tmp = 2;
    }

    printf("\r\nstart DTIM enter sleep timer:%d\r\n", (int)cli_dtim_mode_tmp);
    xTimerStart(xTimer_dtim_sleep, 0);

    return 0;
}
#endif

static uint8_t _cli_lp_rtc_mode(uint8_t len, char *param[])
{
    printf("cli enter rtc mode\r\n");
    hal_gpt_delay_ms(100);

    hal_rtc_enter_retention_mode();

    return 0;
}

static uint8_t _cli_lp_set_sleep_lock_handle(uint8_t len, char *param[])
{
    uint32_t handle_index = atoi(param[0]);

    printf("cli force lock sleep handle : %d\r\n", (int)handle_index);
    hal_sleep_manager_lock_sleep(handle_index);

    return 0;
}

static uint8_t _cli_lp_set_sleep_unlock_handle(uint8_t len, char *param[])
{
    uint32_t handle_index = atoi(param[0]);

    printf("cli force unlock sleep handle : %d\r\n", (int)handle_index);
    hal_sleep_manager_unlock_sleep(handle_index);

    return 0;
}

static uint8_t _cli_lp_dump_debug_log(uint8_t len, char *param[])
{
#ifdef  SLEEP_MANAGEMENT_DEBUG_ENABLE
    uint32_t mode = atoi(param[0]);
    if (mode == 0) {
        sleep_management_debug_dump_lock_sleep_time();
    } else if (mode == 1) {
        sleep_management_debug_dump_backup_restore_time();
    } else {
        printf("unknown lp debug command\r\n");
    }
#else
    printf("need enable 'SLEEP_MANAGEMENT_DEBUG_ENABLE' feature option\r\n");
#endif
    return 0;
}

#if 0
static uint8_t _cli_lp_reset_debug_log(uint8_t len, char *param[])
{
#ifdef  SLEEP_MANAGEMENT_DEBUG_ENABLE
    uint32_t mode = atoi(param[0]);
    if(mode == 0) {
        sleep_management_debug_reset_lock_sleep_time();
        printf("Reset Sleep Lock Time and Count History\r\n");
    }
#else
    printf("need enable 'SLEEP_MANAGEMENT_DEBUG_ENABLE' feature option\r\n");
#endif
    return 0;
}
#endif

#ifdef __MTK_BLE_ONLY_ENABLE__
extern void bt_driver_set_power_constrainpower(void);
extern void bt_driver_deepsleep(void);
extern void bt_driver_write_sfr(uint32_t Address, uint32_t Val);
#endif
static uint8_t _cli_lp_bt_sleep(uint8_t len, char *param[])
{
    printf("cli BT enter sleep mode\r\n");
#ifdef __MTK_BLE_ONLY_ENABLE__

    bt_driver_set_power_constrainpower();
#endif
    return 0;
}

static uint8_t _cli_lp_bt_deep_sleep(uint8_t len, char *param[])
{
    printf("cli BT enter deep sleep mode\r\n");
#ifdef __MTK_BLE_ONLY_ENABLE__
    bt_driver_deepsleep();
#endif
    return 0;
}

static uint8_t _cli_lp_bt_off(uint8_t len, char *param[])
{
    printf("cli BT enter off mode\r\n");

#ifdef __MTK_BLE_ONLY_ENABLE__
    bt_driver_write_sfr(0x805000,0x11);
#endif
    return 0;
}

cmd_t lp_cli[] = {
    { "ds",             "deep sleep",                       _cli_lp_deep_sleep              },
    { "status",         "sleep status",                     _cli_lp_sleep_status            },
#ifdef MTK_CM4_WIFI_TASK_ENABLE
    { "dtim",           "DTIM sleep",                       _cli_lp_dtim_sleep              },
#endif
    { "rtc",            "rtc mode",                         _cli_lp_rtc_mode                },
    { "set_slplock",    "set sleep lock",                   _cli_lp_set_sleep_lock_handle   },
    { "set_slpunlock",  "set sleep unlock",                 _cli_lp_set_sleep_unlock_handle },
    { "debug",          "debug log",                        _cli_lp_dump_debug_log          },
    { "bt_sleep",       "bt sleep",                         _cli_lp_bt_sleep                },
    { "bt_deepsleep",   "bt deep sleep",                    _cli_lp_bt_deep_sleep           },
    { "bt_off",         "bt off ",                          _cli_lp_bt_off                  },
    { "ds_sysramoff",   "deep sleep and sysram off",        _cli_lp_deep_sleep_sysram_off   },
    { NULL }
};
#endif

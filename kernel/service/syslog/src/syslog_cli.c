/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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

#include "syslog.h"
#include "syslog_cli.h"

/****************************************************************************
 *
 * Actual CLI API
 *
 ****************************************************************************/

uint8_t syslog_cli_set_filter(uint8_t len, char *param[])
{
#if !defined (MTK_DEBUG_LEVEL_NONE)

    int i = 0;

    if ( len < 3 || len % 3 ) {
        printf("required parameters: <module_name> <log_switch> <print_level>\n");
        printf("<log_switch>   := on | off\n");
        printf("<print_level>  := debug | info | warning | error\n");
        return 1;
    }

    while (i < len) {
        if (syslog_set_filter(param[i], param[i + 1], param[i + 2], (i == len - 3))) {
            printf("invalid %s %s %s\n", param[i], param[i + 1], param[i + 2]);
            return 2;
        }
        i += 3;
    }

    return 0;

#else

    printf("syslog not supported. check MTK_DEBUG_LEVEL in project's feature.mk\n");
    return 0;

#endif

}

uint8_t syslog_cli_show_config(uint8_t len, char *param[])
{
#if !defined (MTK_DEBUG_LEVEL_NONE)

    syslog_config_t   config;
    int               i   = 0;

    syslog_get_config(&config);

    printf("%s\t%s\t%s\n", "module", "on/off", "level");
    printf("%s\t%s\t%s\n", "------", "------", "-----");

    while (config.filters && config.filters[i] != NULL) {
        printf("%s\t%s\t%s\n",
               config.filters[i]->module_name,
               log_switch_to_str(config.filters[i]->log_switch),
               print_level_to_str(config.filters[i]->print_level));
        i++;
    }

    return 0;

#else

    printf("syslog not supported. check MTK_DEBUG_LEVEL in project's feature.mk\n");
    return 0;

#endif

}

#if defined(MTK_SAVE_LOG_TO_FLASH_ENABLE)
#include <string.h>

static void help_syslog_switch_cmd(void)
{
    cli_puts("required parameters: <path> <save_to_nvdm> <always_save>\n");
    cli_puts("<path>             := flash | console\n");
    cli_puts("<save_to_nvdm>     := y | n\n");
    cli_puts("<always_save>      := y | n\n");
    cli_puts("For instance:\n");
    cli_puts("    Let the syslog show over console\n");
    cli_puts("    Let the setting save to nvdm\n");
    cli_puts("  Use the command like below:\n");
    cli_puts("    log switch console y\n");
    cli_puts("For instance:\n");
    cli_puts("    Let the syslog save to flash\n");
    cli_puts("    Let the setting not save to nvdm\n");
    cli_puts("    Trigger the log to be saved to flash only when user call log save API\n");
    cli_puts("  Use the command like below:\n");
    cli_puts("    log switch flash n n\n");
}

uint8_t syslog_switch_cmd(uint8_t len, char *param[])
{
    bool save_to_flash, save_to_nvdm, always_dump;

    if ( len < 2 || len > 3 ) {
        help_syslog_switch_cmd();
        return 1;
    }

    if (!strcmp(param[0], "flash")) {
        save_to_flash = true;
    } else if (!strcmp(param[0], "console")) {
        save_to_flash = false;
    } else {
        return 1;
    }

    if (!strcmp(param[1], "y")) {
        save_to_nvdm = true;
    } else if (!strcmp(param[1], "n")) {
        save_to_nvdm = false;
    } else {
        return 1;
    }

    if (save_to_flash == true) {
        if (len != 3) {
            help_syslog_switch_cmd();
            return 1;
        }
        if (!strcmp(param[2], "y")) {
            always_dump = true;
        } else if (!strcmp(param[2], "n")) {
            always_dump = false;
        } else {
            return 1;
        }
    } else {
        always_dump = true;
    }

    log_path_switch(save_to_flash, save_to_nvdm, always_dump);

    return 0;
}

#include "hal_trng.h"
#include "FreeRTOS.h"
#include "task.h"
#include "task_def.h"
#include <stdlib.h>
#include "hal_gpt.h"
#include "toi.h"
#include "exception_handler.h"

static uint32_t g_assert_during_time, g_log_during_time;

uint32_t random_get_value(uint32_t min, uint32_t max)
{
    uint32_t random_count;

    while (hal_trng_init() != HAL_TRNG_STATUS_OK) {
        vTaskDelay(50);
    }

    do {
        hal_trng_get_generated_random_number(&random_count);
        random_count %= max;
    } while (random_count < min);

    hal_trng_deinit();

    return random_count;
}

static void save_log_test_task1(void *pvParameters)
{
    while (1) {
        LOG_I(common, "hello world at tick %d", xTaskGetTickCount());
        vTaskDelay(random_get_value(50, 100));
    }
}

static void save_log_test_task2(void *pvParameters)
{
    uint32_t gpt_start_count, gpt_curr_count;

    srand(100);
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_start_count);

    while (1) {
        vTaskDelay(random_get_value(g_log_during_time, g_log_during_time + 100));
        log_trigger_write_to_flash();
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_curr_count);
        if (((gpt_curr_count + random_get_value(5 * 1000000, 10 * 1000000)) - gpt_start_count) > (g_assert_during_time * 1000000)) {
            assert(0);
        }
    }
}

static uint8_t save_log_test(uint8_t len, char *param[])
{
    uint8_t type;

    if (len != 2) {
        cli_puts("parameter error");
        cli_putln();
        return 0;
    }

    exception_dump_config(DISABLE_WHILELOOP_MAGIC);

    g_log_during_time = toi(param[0], &type);
    g_assert_during_time = toi(param[1], &type);

    xTaskCreate(save_log_test_task1, "dump_test1", 2048 / sizeof(StackType_t), NULL, TASK_PRIORITY_HIGH, NULL);
    xTaskCreate(save_log_test_task2, "dump_test2", 2048 / sizeof(StackType_t), NULL, TASK_PRIORITY_HIGH, NULL);

    cli_puts("save_log_test_task2 creat done");
    cli_putln();

    return 0;
}

#endif

#if defined(MTK_MINICLI_ENABLE)
cmd_t syslog_cli[] = {
    { "set",    "setup filter",  syslog_cli_set_filter,   NULL },
#if defined(MTK_SAVE_LOG_TO_FLASH_ENABLE)
    { "switch",  "log over flash or console",  syslog_switch_cmd,  NULL },
    { "save", "test for saving log to flash, Ex: log save <log save time(ms)> <assert time(s)>", save_log_test, NULL },
#endif
    { NULL,     NULL,            NULL,                    NULL }
};
#endif


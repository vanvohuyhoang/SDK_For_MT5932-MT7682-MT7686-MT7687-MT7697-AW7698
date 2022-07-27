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
 
#include <stdint.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "nvdm.h"
#include "bt_system.h"
#include "hal_trng.h"
#include "project_config.h"
#include <string.h>
#include <stdlib.h>
#include "task_def.h"
#include "bt_callback_manager.h"


#define BT_TIMER_BUF_SIZE (BT_TIMER_NUM * BT_CONTROL_BLOCK_SIZE_OF_TIMER)
#define BT_CONNECTION_BUF_SIZE (BT_CONNECTION_MAX* BT_CONTROL_BLOCK_SIZE_OF_LE_CONNECTION)

BT_ALIGNMENT4(
    static char timer_cb_buf[BT_TIMER_BUF_SIZE]//one timer control block is 20 bytes
);
BT_ALIGNMENT4(
    static char connection_cb_buf[BT_CONNECTION_BUF_SIZE]
);

BT_ALIGNMENT4(
    static char tx_buf[BT_TX_BUF_SIZE]
);

BT_ALIGNMENT4(
    static char rx_buf[BT_RX_BUF_SIZE]
);
extern void bt_task(void *arg);

bt_bd_addr_t local_public_addr;

static void bt_preread_local_address(bt_bd_addr_t addr)
{
    nvdm_status_t status;
    int8_t i;
    uint32_t random_seed;
    uint32_t size = 12;
    uint8_t buffer[18] = {0};
    uint8_t tmp_buf[3] = {0};
    bt_bd_addr_t tempaddr = {0};
    hal_trng_status_t ret = HAL_TRNG_STATUS_ERROR;
    if (memcmp(addr, &tempaddr, sizeof(bt_bd_addr_t)) == 0) {
        LOG_I(common, "[BT]Empty bt address after bt_gap_le_get_local_address()\n");
        LOG_I(common, "[BT]Try to read from NVDM.\n");
        status = nvdm_read_data_item("BT", "address", buffer, &size);
        if (NVDM_STATUS_OK == status) {
            LOG_I(common, "[BT]Read from NVDM:%s\n", buffer);
            for (i = 0; i < 6; ++i) {
                tmp_buf[0] = buffer[2 * i];
                tmp_buf[1] = buffer[2 * i + 1];
                addr[i] = (uint8_t)strtoul((char *)tmp_buf, NULL, 16);
            }

            LOG_I(common, "[BT]Read address from NVDM 0 ~ 5[%02X %02X %02X %02X %02X %02X]\n", addr[0],
                  addr[1], addr[2], addr[3], addr[4], addr[5]);
            return;
        } else {
            LOG_I(common, "[BT]Failed to Read from NVDM:%d.\n", status);
            ret = hal_trng_init();
            if (HAL_TRNG_STATUS_OK != ret) {
                LOG_I(common, "[BT]generate_random_address--error 1");
            }
            for (i = 0; i < 30; ++i) {
                ret = hal_trng_get_generated_random_number(&random_seed);
                if (HAL_TRNG_STATUS_OK != ret) {
                    LOG_I(common, "[BT]generate_random_address--error 2");
                }
                LOG_I(common, "[BT]generate_random_address--trn: 0x%x", random_seed);
            }
            /* randomly generate address */
            ret = hal_trng_get_generated_random_number(&random_seed);
            if (HAL_TRNG_STATUS_OK != ret) {
                LOG_I(common, "[BT]generate_random_address--error 3");
            }
            LOG_I(common, "[BT]generate_random_address--trn: 0x%x", random_seed);
            addr[0] = random_seed & 0xFF;
            addr[1] = (random_seed >> 8) & 0xFF;
            addr[2] = (random_seed >> 16) & 0xFF;
            addr[3] = (random_seed >> 24) & 0xFF;
            ret = hal_trng_get_generated_random_number(&random_seed);
            if (HAL_TRNG_STATUS_OK != ret) {
                LOG_I(common, "[BT]generate_random_address--error 3");
            }
            LOG_I(common, "[BT]generate_random_address--trn: 0x%x", random_seed);
            addr[4] = random_seed & 0xFF;
            addr[5] = (random_seed >> 8) & 0xCF;
            hal_trng_deinit();
        }
    }
    /* save address to NVDM */
    for (i = 0; i < 6; ++i) {
        snprintf((char *)buffer + 2 * i, 3, "%02X", addr[i]);
    }
    LOG_I(common, "[BT]address to write:%s len:%d\n", buffer, strlen((char *)buffer));
    status = nvdm_write_data_item("BT", "address", NVDM_DATA_ITEM_TYPE_STRING, buffer, strlen((char *)buffer));
    if (NVDM_STATUS_OK != status) {
        LOG_I(common, "[BT]Failed to store address.\n");
    } else {
        LOG_I(common, "[BT]Successfully store address to NVDM [%02X:%02X:%02X:%02X:%02X:%02X]\n", addr[0],
              addr[1], addr[2], addr[3], addr[4], addr[5]);
    }
}

void bt_mm_init()
{
    bt_memory_init_packet(BT_MEMORY_TX_BUFFER, tx_buf, BT_TX_BUF_SIZE);
    bt_memory_init_packet(BT_MEMORY_RX_BUFFER, rx_buf, BT_RX_BUF_SIZE);
    bt_memory_init_control_block(BT_MEMORY_CONTROL_BLOCK_TIMER, timer_cb_buf, BT_TIMER_BUF_SIZE);
    bt_memory_init_control_block(BT_MEMORY_CONTROL_BLOCK_LE_CONNECTION, connection_cb_buf,
                                 BT_CONNECTION_BUF_SIZE);
}

#include "bt_gap_le.h"
#include "bt_gatts.h"

#ifdef __MTK_BT_MESH_ENABLE__
extern bt_status_t bt_app_event_callback_demo(bt_msg_type_t msg, bt_status_t status, void *buff);
extern void mesh_app_util_log_off();
#endif
extern void clear_bonded_info();
extern void bt_app_callback_handler_init();

void bt_create_task(void)
{
    bt_mm_init();
    bt_app_callback_handler_init();
#ifdef __MTK_BT_MESH_ENABLE__
    mesh_app_util_log_off();
    bt_callback_manager_register_callback(bt_callback_type_app_event,
                                          (uint32_t)(MODULE_MASK_GAP | MODULE_MASK_GATT | MODULE_MASK_SYSTEM | MODULE_MASK_MM),
                                          (void *)bt_app_event_callback_demo);
#endif
    clear_bonded_info();
    bt_preread_local_address(local_public_addr);
    printf("[BT]local_public_addr [%02X:%02X:%02X:%02X:%02X:%02X]\n", local_public_addr[5],
           local_public_addr[4], local_public_addr[3], local_public_addr[2], local_public_addr[1], local_public_addr[0]);
    log_config_print_switch(BT, DEBUG_LOG_OFF);
    log_config_print_switch(BTMM, DEBUG_LOG_OFF);
    log_config_print_switch(BTHCI, DEBUG_LOG_OFF);
    log_config_print_switch(BTL2CAP, DEBUG_LOG_OFF);
    log_config_print_switch(BTA2DP, DEBUG_LOG_OFF);
    log_config_print_switch(BTAVM, DEBUG_LOG_OFF);
    log_config_print_switch(BTAVRCP, DEBUG_LOG_OFF);
    log_config_print_switch(BTAWS, DEBUG_LOG_OFF);
    log_config_print_switch(BTCOMMON, DEBUG_LOG_OFF);
    log_config_print_switch(BTGAP, DEBUG_LOG_OFF);
    log_config_print_switch(BTGATT, DEBUG_LOG_OFF);
    log_config_print_switch(BTGOEP, DEBUG_LOG_OFF);
    log_config_print_switch(BTHID, DEBUG_LOG_OFF);
    log_config_print_switch(BTAIRUPDATE, DEBUG_LOG_OFF);
    log_config_print_switch(BTHFP, DEBUG_LOG_OFF);
    log_config_print_switch(BTHSP, DEBUG_LOG_OFF);
    log_config_print_switch(BTATT, DEBUG_LOG_OFF);
    log_config_print_switch(BTPBAPC, DEBUG_LOG_OFF);
    log_config_print_switch(BTSDP, DEBUG_LOG_OFF);
    log_config_print_switch(BTSM, DEBUG_LOG_OFF);
    log_config_print_switch(BTTIMER, DEBUG_LOG_OFF);
    if (pdPASS != xTaskCreate(bt_task, BLUETOOTH_TASK_NAME, BLUETOOTH_TASK_STACKSIZE / sizeof(StackType_t), (void *)local_public_addr, BLUETOOTH_TASK_PRIO, NULL)) {
        LOG_E(common, "cannot create bt_task.");
    }
}

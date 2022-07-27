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
 
#include "hal.h"
#include "hal_sdio_slave.h"
#include "hal_sdio_slave_internal.h"

#ifdef HAL_SDIO_SLAVE_MODULE_ENABLED
#include "hal_log.h"
#include "assert.h"
#include "hal_platform.h"
#include <string.h>
#ifdef HAL_SLEEP_MANAGER_ENABLED
#include "hal_sleep_manager.h"
#include "hal_sleep_manager_internal.h"
#endif

extern sdio_slave_private_t sdio_private;

hal_sdio_slave_status_t hal_sdio_slave_register_callback(hal_sdio_slave_port_t sdio_slave_port, hal_sdio_slave_callback_t sdio_slave_callback, void *user_data)
{
    sdio_private.sdio_property.sdio_slave_callback = sdio_slave_callback;
    sdio_private.sdio_property.sdio_slave_callback_user_data = user_data;

    return HAL_SDIO_SLAVE_STATUS_OK;
}


hal_sdio_slave_status_t hal_sdio_slave_init(hal_sdio_slave_port_t sdio_slave_port)
{
    sdio_slave_init();
	
#ifdef HAL_SLEEP_MANAGER_ENABLED
    sleep_management_register_suspend_callback(SLEEP_BACKUP_RESTORE_SDIO_SLV, (sleep_management_suspend_callback_t)sdio_slave_backup_all, NULL);
    sleep_management_register_resume_callback(SLEEP_BACKUP_RESTORE_SDIO_SLV, (sleep_management_resume_callback_t)sdio_slave_restore_all, NULL);
#endif

    return HAL_SDIO_SLAVE_STATUS_OK;
}



hal_sdio_slave_status_t hal_sdio_slave_read_mailbox(hal_sdio_slave_port_t sdio_slave_port, uint32_t mailbox_number, uint32_t *mailbox_value)
{
    while (true == sdio_slave_check_fw_own());

    sdio_slave_read_mailbox(mailbox_number, mailbox_value);

    return HAL_SDIO_SLAVE_STATUS_OK;
}


hal_sdio_slave_status_t hal_sdio_slave_write_mailbox(hal_sdio_slave_port_t sdio_slave_port, uint32_t mailbox_number, uint32_t mailbox_value)
{
    while (true == sdio_slave_check_fw_own());

    sdio_slave_write_mailbox(mailbox_number, mailbox_value);

    return HAL_SDIO_SLAVE_STATUS_OK;
}


hal_sdio_slave_status_t hal_sdio_slave_trigger_d2h_interrupt(hal_sdio_slave_port_t sdio_slave_port, uint32_t interrupt_number)
{
    while (true == sdio_slave_check_fw_own());

    sdio_slave_set_device_to_host_interrupt(interrupt_number);

    return HAL_SDIO_SLAVE_STATUS_OK;
}



hal_sdio_slave_status_t hal_sdio_slave_send_dma(hal_sdio_slave_port_t sdio_slave_port, hal_sdio_slave_rx_queue_id_t queue_id, const uint32_t *buffer, uint32_t size)
{
    sdio_slave_status_t status;

    while (true == sdio_slave_check_fw_own());

    status = sdio_slave_send((sdio_slave_rx_queue_id_t)queue_id, (uint32_t)buffer, size);
    if (SDIO_SLAVE_STATUS_OK != status) {
        return HAL_SDIO_SLAVE_STATUS_ERROR;
    }

    return HAL_SDIO_SLAVE_STATUS_OK;
}

hal_sdio_slave_status_t hal_sdio_slave_receive_dma(hal_sdio_slave_port_t sdio_slave_port, hal_sdio_slave_tx_queue_id_t queue_id, uint32_t *buffer, uint32_t size)
{
    sdio_slave_status_t status;

    while (true == sdio_slave_check_fw_own());

    status = sdio_slave_receive((sdio_slave_tx_queue_id_t)queue_id, (uint32_t)buffer, size);
    if (SDIO_SLAVE_STATUS_OK != status) {
        return HAL_SDIO_SLAVE_STATUS_ERROR;
    }

    return HAL_SDIO_SLAVE_STATUS_OK;
}


#ifdef HAL_SLEEP_MANAGER_ENABLED
/*
bit16 bit8 bit0
  1     1    1          SDIO_SLV sram power donw
  1     0    0          SDIO_SLV sram sleep
  0     1    0          SDIO_SLV sram normal
*/
void sdio_slave_backup_all(void *data)
{
    *SPM_SDIO_SLV_SRAM_PD = 0x010101;
}

void sdio_slave_restore_all(void *data)
{
    *SPM_SDIO_SLV_SRAM_PD = 0x0100;
}
#endif
#endif


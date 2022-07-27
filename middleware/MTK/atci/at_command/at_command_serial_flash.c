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
 
#ifdef MTK_SPI_EXTERNAL_SERIAL_FLASH_ENABLED

/* Includes ------------------------------------------------------------------*/
#include "at_command.h"
#include "bsp_external_flash.h"
#include "toi.h"
#include "syslog.h"
#include "string.h"

#if  defined ( __GNUC__ )
  #ifndef __weak
    #define __weak   __attribute__((weak))
  #endif /* __weak */
#endif /* __GNUC__ */

/* Private macro -------------------------------------------------------------*/
#define AT_COMMAND_SERIAL_FLASH_HELP \
"AT+EFLASH=0\r\n"

#define is_0_to_f(_c)   ((_c >= '0' && _c <= '9') || (_c >= 'a' && _c <= 'f'))
#define ascii_to_number(_c)   ((_c >= '0' && _c <= '9') ? (_c - '0') : (_c - 'a' + 10))
#define number_to_ascii(_c)   ((_c <= 9) ? (_c + '0') : (_c - 10 + 'a'))

static uint8_t g_serial_flash_temp_buffer[24];

static uint32_t g_serial_flash_id_len[] = {
    3,
    3,
    3,
};

static uint8_t g_serial_flash_id[][4] = {
    {0xEF, 0x60, 0x18},
    {0xEF, 0x80, 0x18},
	{0xEF, 0x60, 0x17},
};

extern	bsp_flash_status_t bsp_external_flash_get_rdid(uint8_t *buffer);

/**
* @brief      AT command handler function for port service.
* @param[in]  parse_cmd: command parameters imported by ATCI module.
* @return     Execution result of port service AT command.
*/
atci_status_t atci_cmd_hdlr_serial_flash(atci_parse_cmd_param_t *parse_cmd)
{
    uint32_t i, j;
    bool result, id_is_match;
    uint8_t flash_id[6];
    atci_response_t response = {{0}};

    response.response_flag = 0; // Command Execute Finish.
    #ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    response.cmd_id = parse_cmd->cmd_id;
    #endif

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:
            strcpy((char *)(response.response_buf), AT_COMMAND_SERIAL_FLASH_HELP);
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            response.response_len = strlen((char *)(response.response_buf));
            break;
        case ATCI_CMD_MODE_EXECUTION:
            if (strstr((char *)parse_cmd->string_ptr, "AT+EFLASH=0") != NULL) {
                LOG_MSGID_I(common, "Begin to run flash write/read/verify test\r\n", 0);
                bsp_external_flash_init(HAL_SPI_MASTER_2, 1000000);
                memset(flash_id, 0, sizeof(flash_id));
                bsp_external_flash_get_rdid(flash_id);
                LOG_MSGID_I(common, "Flash id: ", 0);
                for (i = 0; i < sizeof(flash_id); i++) {
                    LOG_MSGID_I(common, "%02x",1, flash_id[i]);
                }
                id_is_match = false;
                for (i = 0; i < sizeof(g_serial_flash_id_len) / sizeof(uint32_t); i++) {
                    for (j = 0; j < g_serial_flash_id_len[i]; j++) {
                        if (g_serial_flash_id[i][j] != flash_id[1 + j]) {
                            break;
                        }
                    }
                    if (j >= g_serial_flash_id_len[i]) {
                        id_is_match = true;
                        break;
                    }
                }
                if (id_is_match == false) {
                    response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    LOG_MSGID_I(common, "Read flash ID fail", 0);
                    break;
                }
                for (i = 0; i < sizeof(g_serial_flash_temp_buffer); i++) {
                    g_serial_flash_temp_buffer[i] = i % 256;
                }
                bsp_external_flash_erase(0, FLASH_BLOCK_4K);
                bsp_external_flash_write(0x4F, g_serial_flash_temp_buffer, sizeof(g_serial_flash_temp_buffer));
                memset(g_serial_flash_temp_buffer, 0x00, sizeof(g_serial_flash_temp_buffer));
                bsp_external_flash_read(0x4F, g_serial_flash_temp_buffer, sizeof(g_serial_flash_temp_buffer));
                for (i = 0; i < sizeof(g_serial_flash_temp_buffer); i++) {
                    if (g_serial_flash_temp_buffer[i] != (i % 256)) {
                        response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                        LOG_MSGID_I(common, "Verify flash write/read fail", 0);
                        atci_send_response(&response);
                        return ATCI_STATUS_OK;
                    }
                }
                strcpy((char *)response.response_buf, "External flash test run pass\r\n");
                response.response_len = strlen((char *)response.response_buf);
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else {
                response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            }
            break;
        default:
            strcpy((char *)(response.response_buf), "ERROR\r\n");
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            response.response_len = strlen((char *)(response.response_buf));
            break;
    }

    atci_send_response(&response);

    return ATCI_STATUS_OK;
}

#endif


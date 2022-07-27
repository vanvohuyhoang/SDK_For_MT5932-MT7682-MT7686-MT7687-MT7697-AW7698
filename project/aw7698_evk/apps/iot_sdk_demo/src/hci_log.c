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
 
//#include "uart.h"
#include "hal_platform.h"
#include "hal_uart.h"
#include "hci_log.h"
#include <stdio.h>
#include <string.h>


#ifdef MTK_HCI_CONSOLE_MIX_ENABLE
//#include "mt7687.h"
#define HCI_MAGIC_HI    0xab
#define HCI_MAGIC_LO    0xcd
#define BT_HCILOG_PORT  HAL_UART_0
#else
#define BT_HCILOG_PORT  HAL_UART_1
#endif
extern int log_write(char *buf, int len);

#ifdef BLE_THROUGHPUT
// only output part hci log data to prevent it influencing throughput.
#define HCI_LOG_LEN (15)      // HEAD:16, payload: frame_num 1, frame header: 5, payload len:2
static unsigned char g_hci_log_buf[2048];
static unsigned char g_gatt_omit_buf[28];
static unsigned char* hci_log_gatt_part_data_omit(int32_t buf_len, unsigned char* buf, uint32_t *out_len)
{
    if (buf_len > HCI_LOG_LEN )
    {
        memcpy(g_gatt_omit_buf, buf, HCI_LOG_LEN);
        g_gatt_omit_buf[2] = HCI_LOG_LEN - 4;
        g_gatt_omit_buf[3] = 0;
        g_gatt_omit_buf[4] = HCI_LOG_LEN - 8;
        g_gatt_omit_buf[5] = 0;
        *out_len = HCI_LOG_LEN;
        return g_gatt_omit_buf;
    }
    else
    {
        memcpy(g_hci_log_buf, buf, buf_len);
        *out_len = buf_len;
        return g_hci_log_buf;
    }
}
#endif

static int32_t hci_log(unsigned char type, unsigned char *buf, int32_t length)
{
    unsigned char head[5] = {0xAB,0xCD, type, (uint8_t)((length & 0xff00) >> 8), (uint8_t)(length & 0xff)};

    log_write((char *)head, 5);
    log_write((char *)buf, length);

    return length;
}

int32_t hci_log_cmd(unsigned char *buf, int32_t length)
{
    return hci_log(HCI_COMMAND, buf, length);
}

int32_t hci_log_event(unsigned char *buf, int32_t length)
{
    return hci_log(HCI_EVENT, buf, length);
}

int32_t hci_log_acl_out(unsigned char *buf, int32_t length)
{
#ifdef BLE_THROUGHPUT
    uint32_t out_len;
    unsigned char* omit_buf = hci_log_gatt_part_data_omit(length, buf, &out_len);
    return hci_log(HCI_ACL_OUT, omit_buf, out_len);
#else
    return hci_log(HCI_ACL_OUT, buf, length);
#endif
}
int32_t hci_log_acl_in(unsigned char *buf, int32_t length)
{
    return hci_log(HCI_ACL_IN, buf, length);
}

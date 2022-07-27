/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#include "bt_type.h"
#include "bt_debug.h"
#include "syslog.h"

#ifdef __BT_DEBUG__
log_create_module(BT, PRINT_LEVEL_INFO);
log_create_module(BTMM, PRINT_LEVEL_INFO);
log_create_module(BTHCI, PRINT_LEVEL_INFO);
log_create_module(BTL2CAP, PRINT_LEVEL_INFO);
log_create_module(BTRFCOMM, PRINT_LEVEL_INFO);
log_create_module(BTSPP, PRINT_LEVEL_INFO);

log_create_module(BTA2DP, PRINT_LEVEL_INFO);
log_create_module(BTAVM, PRINT_LEVEL_INFO);
log_create_module(BTAVRCP, PRINT_LEVEL_INFO);
log_create_module(BTAWS, PRINT_LEVEL_INFO);
log_create_module(BTCOMMON, PRINT_LEVEL_INFO);
log_create_module(BTGAP, PRINT_LEVEL_INFO);
log_create_module(BTGATT, PRINT_LEVEL_INFO);
log_create_module(BTGOEP, PRINT_LEVEL_INFO);
log_create_module(BTHID, PRINT_LEVEL_INFO);
log_create_module(BTAIRUPDATE, PRINT_LEVEL_INFO);
log_create_module(BTHFP, PRINT_LEVEL_INFO);
log_create_module(BTHSP, PRINT_LEVEL_INFO);
log_create_module(BTATT, PRINT_LEVEL_INFO);
log_create_module(BTPBAPC, PRINT_LEVEL_INFO);
log_create_module(BTSDP, PRINT_LEVEL_INFO);
log_create_module(BTSM, PRINT_LEVEL_INFO);
log_create_module(BTTIMER, PRINT_LEVEL_INFO);

log_create_module_variant(BTIF, DEBUG_LOG_OFF, PRINT_LEVEL_INFO);

//#define BT_DEBUG_NO_BTIF
#define BT_DEBUG_BUFF_SIZE  150
static char bt_debug_buff[BT_DEBUG_BUFF_SIZE];

typedef struct {
    log_control_block_t* log_ctrl_blk;
    char * module;
} bt_log_ctrl_blk_t;

const static bt_log_ctrl_blk_t bt_log_filter_blk[] = {
       {&log_control_block_BT, "[BT]"},
       {&log_control_block_BTMM, "[MM]"},
       {&log_control_block_BTHCI, "[HCI]"},
       {&log_control_block_BTL2CAP, "[L2CAP]"},
       {&log_control_block_BTRFCOMM, "[RFCOMM]"},
       {&log_control_block_BTSPP, "[SPP]"},
       {&log_control_block_BTA2DP, "[A2DP]"},
       {&log_control_block_BTA2DP, "[AVDTP]"},
       {&log_control_block_BTAVM, "[AVM]"},
       {&log_control_block_BTAVRCP, "[AVRCP]"},
       {&log_control_block_BTAWS, "[AWSMCE]"},
       {&log_control_block_BTAWS, "[AWS]"},
       {&log_control_block_BTCOMMON, "[COMMON]"},
       {&log_control_block_BTGAP, "[GAP]"},
       {&log_control_block_BTGATT, "[GATT]"},
       {&log_control_block_BTGOEP, "[GOEP]"},
       {&log_control_block_BTHID, "[HID]"},
       {&log_control_block_BTAIRUPDATE, "[BT_AIRUPDATE]"},
       {&log_control_block_BTHFP, "[HFP]"},
       {&log_control_block_BTHSP, "[HSP]"},
       {&log_control_block_BTATT, "[ATT]"},
       {&log_control_block_BTPBAPC, "[PBAPC]"},
       {&log_control_block_BTSDP, "[SDP]"},
       {&log_control_block_BTSM, "[SM]"},
       {&log_control_block_BTTIMER, "[TIMER]"},
       {&log_control_block_BTIF, "[BTIF]"}
};

static log_control_block_t* bt_debug_find_ctrl_blk_by_module(const char *module)
{
    uint32_t idx = 0;
    const uint32_t bt_log_ctrl_blk_len = sizeof(bt_log_filter_blk) / sizeof(bt_log_ctrl_blk_t);
    while (idx < bt_log_ctrl_blk_len) {
        if (memcmp(bt_log_filter_blk[idx].module, module, strlen(module)) == 0) {
            break;
        }
        ++idx;
    }
    if (idx == bt_log_ctrl_blk_len) {
        idx = 0;
    }
    return bt_log_filter_blk[idx].log_ctrl_blk;
}

void bt_debug_log(const char *module, const char *format, ...)
{
    va_list arg;

    if ( 0
#ifdef BT_DEBUG_NO_MM
        || strstr(format, "[MM]")
#endif
#ifdef BT_DEBUG_NO_TIMER
        || strstr(format, "[TIMER]")
#endif
#ifdef BT_DEBUG_NO_HCI
        || strstr(format, "[HCI]")
#endif
#ifdef BT_DEBUG_NO_BTIF
        || strstr(format, "BTIF")
#endif
#ifdef BT_DEBUG_NO_GAP
        || strstr(format, "[GAP]")
#endif
#ifdef BT_DEBUG_NO_A2DP
        || strstr(format, "[A2DP]")
#endif
#ifdef BT_DEBUG_NO_AVDTP
        || strstr(format, "[AVDTP]")
#endif

#ifdef BT_DEBUG_NO_I
        || strstr(format, "[I]")
#endif
#ifdef BT_DEBUG_NO_D
        || strstr(format, "[D]")
#endif
        ) {
        return;
    }
    print_level_t level = PRINT_LEVEL_DEBUG;
    if (module[1] == 'I') {
        level = PRINT_LEVEL_INFO;
    } else if (module[1] == 'W') {
        level = PRINT_LEVEL_WARNING;
    } else if (module[1] == 'E') {
        level = PRINT_LEVEL_ERROR;
    }

    log_control_block_t *block_name = bt_debug_find_ctrl_blk_by_module(module+3);

    va_start(arg, format);
    vprint_module_log(block_name, __FUNCTION__, __LINE__, level, format, arg);
    va_end(arg);
}

const char *bt_debug_bd_addr2str(const bt_bd_addr_t addr)
{
    snprintf(bt_debug_buff, BT_DEBUG_BUFF_SIZE, "%02x-%02x-%02x-%02x-%02x-%02x",
            addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
    return bt_debug_buff;
}

const char *bt_debug_bd_addr2str2(const bt_bd_addr_t addr)
{
    snprintf(bt_debug_buff, BT_DEBUG_BUFF_SIZE, "LAP: %02x-%02x-%02x, UAP: %02x, NAP: %02x-%02x",
            addr[2], addr[1], addr[0], addr[3], addr[5], addr[4]);
    return bt_debug_buff;
}

const char *bt_debug_addr2str(const bt_addr_t *p)
{
    bt_bd_addr_ptr_t addr = p->addr;
    snprintf(bt_debug_buff, BT_DEBUG_BUFF_SIZE, "[%s%s] %02x-%02x-%02x-%02x-%02x-%02x",
            (p->type & 0x01) == BT_ADDR_PUBLIC ? "PUBLIC" : "RANDOM",
             p->type >=2 ?"_IDENTITY":"",
            addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
    return bt_debug_buff;
}

const char *bt_debug_addr2str2(const bt_addr_t *p)
{
    bt_bd_addr_ptr_t addr = p->addr;
    snprintf(bt_debug_buff, BT_DEBUG_BUFF_SIZE, "[%s%s] LAP: %02x-%02x-%02x, UAP: %02x, NAP: %02x-%02x",
            (p->type & 0x01) == BT_ADDR_PUBLIC ? "PUBLIC" : "RANDOM",
             p->type >=2 ?"_IDENTITY":"",
            addr[2], addr[1], addr[0], addr[3], addr[5], addr[4]);
    return bt_debug_buff;
}

void bt_debug_log_msgid_d(const char *module, const char *message, uint32_t arg_cnt, ...)
{
#if !defined (MTK_DEBUG_LEVEL_NONE)
    va_list ap;
    log_control_block_t *block_name = bt_debug_find_ctrl_blk_by_module(module);

    va_start(ap, arg_cnt);
    log_print_msgid(block_name, PRINT_LEVEL_DEBUG, message, arg_cnt, ap);
    va_end(ap);
#endif
}

void bt_debug_log_msgid_i(const char *module, const char *message, uint32_t arg_cnt, ...)
{
#if !defined (MTK_DEBUG_LEVEL_NONE)
    va_list ap;
    log_control_block_t *block_name = bt_debug_find_ctrl_blk_by_module(module);

    va_start(ap, arg_cnt);
    log_print_msgid(block_name, PRINT_LEVEL_INFO, message, arg_cnt, ap);
    va_end(ap);
#endif
}

void bt_debug_log_msgid_w(const char *module, const char *message, uint32_t arg_cnt, ...)
{
#if !defined (MTK_DEBUG_LEVEL_NONE)
    va_list ap;
    log_control_block_t *block_name = bt_debug_find_ctrl_blk_by_module(module);

    va_start(ap, arg_cnt);
    log_print_msgid(block_name, PRINT_LEVEL_WARNING, message, arg_cnt, ap);
    va_end(ap);
#endif
}

void bt_debug_log_msgid_e(const char *module, const char *message, uint32_t arg_cnt, ...)
{
#if !defined (MTK_DEBUG_LEVEL_NONE)
    va_list ap;
    log_control_block_t *block_name = bt_debug_find_ctrl_blk_by_module(module);

    va_start(ap, arg_cnt);
    log_print_msgid(block_name, PRINT_LEVEL_ERROR, message, arg_cnt, ap);
    va_end(ap);
#endif
}

#endif /* BT_DEBUG */


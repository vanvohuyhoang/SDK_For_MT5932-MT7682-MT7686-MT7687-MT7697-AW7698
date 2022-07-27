/* Copyright Statement:
 *
 * (C) 2018  Airoha Technology Corp. All rights reserved.
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

#ifdef MTK_NVDM_ENABLE

#include "hal_flash.h"
#include "syslog.h"
#include <assert.h>
#include "nvdm_port.h"
#include "memory_attribute.h"

#if !defined(MTK_DEBUG_LEVEL_NONE)
    ATTR_LOG_STRING_LIB nvdm_001[] = LOG_INFO_PREFIX(nvdm) "data item header info show below:";
    ATTR_LOG_STRING_LIB nvdm_002[] = LOG_INFO_PREFIX(nvdm) "status: 0x%02x";
    ATTR_LOG_STRING_LIB nvdm_003[] = LOG_INFO_PREFIX(nvdm) "pnum: %d";
    ATTR_LOG_STRING_LIB nvdm_004[] = LOG_INFO_PREFIX(nvdm) "offset: 0x%04x";
    ATTR_LOG_STRING_LIB nvdm_005[] = LOG_INFO_PREFIX(nvdm) "sequence_number: %d";
    ATTR_LOG_STRING_LIB nvdm_006[] = LOG_INFO_PREFIX(nvdm) "group_name_size: %d";
    ATTR_LOG_STRING_LIB nvdm_007[] = LOG_INFO_PREFIX(nvdm) "data_item_name_size: %d";
    ATTR_LOG_STRING_LIB nvdm_008[] = LOG_INFO_PREFIX(nvdm) "value_size: %d";
    ATTR_LOG_STRING_LIB nvdm_009[] = LOG_INFO_PREFIX(nvdm) "index: %d";
    ATTR_LOG_STRING_LIB nvdm_010[] = LOG_INFO_PREFIX(nvdm) "type: %d";
    ATTR_LOG_STRING_LIB nvdm_011[] = LOG_INFO_PREFIX(nvdm) "hash_name: 0x%08x";
    ATTR_LOG_STRING_LIB nvdm_012[] = LOG_INFO_PREFIX(nvdm) "hashname = 0x%08x";
    ATTR_LOG_STRING_LIB nvdm_013[] = LOG_INFO_PREFIX(nvdm) "nvdm_read_data_item: begin to read";
    ATTR_LOG_STRING_LIB nvdm_015[] = LOG_INFO_PREFIX(nvdm) "nvdm_write_data_item: begin to write";
    ATTR_LOG_STRING_LIB nvdm_016[] = LOG_INFO_PREFIX(nvdm) "find_data_item_by_hashname return %d";
    ATTR_LOG_STRING_LIB nvdm_017[] = LOG_INFO_PREFIX(nvdm) "peb free space is not enough\n";
    ATTR_LOG_STRING_LIB nvdm_018[] = LOG_WARNING_PREFIX(nvdm) "too many data items in nvdm region\n";
    ATTR_LOG_STRING_LIB nvdm_019[] = LOG_INFO_PREFIX(nvdm) "new data item append";
    ATTR_LOG_STRING_LIB nvdm_020[] = LOG_INFO_PREFIX(nvdm) "old data item overwrite";
    ATTR_LOG_STRING_LIB nvdm_022[] = LOG_INFO_PREFIX(nvdm) "nvdm_write_data_item_non_blocking: begin to write";
    ATTR_LOG_STRING_LIB nvdm_023[] = LOG_ERROR_PREFIX(nvdm) "Can't alloc memory!!";
    ATTR_LOG_STRING_LIB nvdm_024[] = LOG_WARNING_PREFIX(nvdm) "Can't send queue!!";
    ATTR_LOG_STRING_LIB nvdm_025[] = LOG_INFO_PREFIX(nvdm) "nvdm_delete_data_item: enter";
    ATTR_LOG_STRING_LIB nvdm_026[] = LOG_INFO_PREFIX(nvdm) "nvdm_delete_group: enter";
    ATTR_LOG_STRING_LIB nvdm_027[] = LOG_INFO_PREFIX(nvdm) "nvdm_delete_all: enter";
    ATTR_LOG_STRING_LIB nvdm_028[] = LOG_INFO_PREFIX(nvdm) "nvdm_query_begin: enter";
    ATTR_LOG_STRING_LIB nvdm_029[] = LOG_INFO_PREFIX(nvdm) "nvdm_query_end: enter";
    ATTR_LOG_STRING_LIB nvdm_030[] = LOG_INFO_PREFIX(nvdm) "nvdm_query_next_group_name: enter";
    ATTR_LOG_STRING_LIB nvdm_031[] = LOG_INFO_PREFIX(nvdm) "nvdm_query_next_data_item_name: enter";
    ATTR_LOG_STRING_LIB nvdm_032[] = LOG_INFO_PREFIX(nvdm) "nvdm_query_data_item_length: begin to query";
    ATTR_LOG_STRING_LIB nvdm_034[] = LOG_INFO_PREFIX(nvdm) "scanning pnum(%d) to analysis data item info";
    ATTR_LOG_STRING_LIB nvdm_035[] = LOG_ERROR_PREFIX(nvdm) "pnum=%d, offset=0x%x";
    ATTR_LOG_STRING_LIB nvdm_036[] = LOG_ERROR_PREFIX(nvdm) "Detect index of data item with out of range, max = %d, curr = %d";
    ATTR_LOG_STRING_LIB nvdm_037[] = LOG_INFO_PREFIX(nvdm) "detect checksum error\n";
    ATTR_LOG_STRING_LIB nvdm_038[] = LOG_ERROR_PREFIX(nvdm) "too many data items in nvdm region\n";
    ATTR_LOG_STRING_LIB nvdm_039[] = LOG_INFO_PREFIX(nvdm) "detect two valid copy of data item";
    ATTR_LOG_STRING_LIB nvdm_040[] = LOG_INFO_PREFIX(nvdm) "copy1(pnum=%d, offset=0x%04x), copy2(pnum=%d, offset=0x%04x)\n";
    ATTR_LOG_STRING_LIB nvdm_041[] = LOG_ERROR_PREFIX(nvdm) "abnormal_data_item = %d";
    ATTR_LOG_STRING_LIB nvdm_042[] = LOG_ERROR_PREFIX(nvdm) "Max size of data item must less than or equal to 2048 bytes";
    ATTR_LOG_STRING_LIB nvdm_043[] = LOG_ERROR_PREFIX(nvdm) "alloc data_item_headers fail";
    ATTR_LOG_STRING_LIB nvdm_044[] = LOG_ERROR_PREFIX(nvdm) "old_src_pnum=%d, old_pos=0x%x, new_src_pnum=%d, new_pos=0x%x";
    ATTR_LOG_STRING_LIB nvdm_045[] = LOG_ERROR_PREFIX(nvdm) "src_pnum=%d, pos=0x%x";
    ATTR_LOG_STRING_LIB nvdm_046[] = LOG_ERROR_PREFIX(nvdm) "pnum=%d, offset=0x%x, len=%d";
    ATTR_LOG_STRING_LIB nvdm_047[] = LOG_ERROR_PREFIX(nvdm) "addr=0x%x, pnum=%d, offset=0x%x, len=%d";
    ATTR_LOG_STRING_LIB nvdm_048[] = LOG_ERROR_PREFIX(nvdm) "pnum=%d";
    ATTR_LOG_STRING_LIB nvdm_049[] = LOG_INFO_PREFIX(nvdm) "region info show below:";
    ATTR_LOG_STRING_LIB nvdm_050[] = LOG_INFO_PREFIX(nvdm) "peb    free    dirty    erase_count    is_reserved";
    ATTR_LOG_STRING_LIB nvdm_051[] = LOG_INFO_PREFIX(nvdm) "%d     %d     %d     %d      %d";
    ATTR_LOG_STRING_LIB nvdm_052[] = LOG_WARNING_PREFIX(nvdm) "g_valid_data_size = %d";
    ATTR_LOG_STRING_LIB nvdm_053[] = LOG_INFO_PREFIX(nvdm) "peb header(%d) info show below:";
    ATTR_LOG_STRING_LIB nvdm_054[] = LOG_INFO_PREFIX(nvdm) "magic: %08x";
    ATTR_LOG_STRING_LIB nvdm_055[] = LOG_INFO_PREFIX(nvdm) "erase_count: %08x";
    ATTR_LOG_STRING_LIB nvdm_056[] = LOG_INFO_PREFIX(nvdm) "status: %02x";
    ATTR_LOG_STRING_LIB nvdm_057[] = LOG_INFO_PREFIX(nvdm) "peb_reserved: %02x";
    ATTR_LOG_STRING_LIB nvdm_058[] = LOG_INFO_PREFIX(nvdm) "version: %02x";
    ATTR_LOG_STRING_LIB nvdm_059[] = LOG_ERROR_PREFIX(nvdm) "pnum=%d";
    ATTR_LOG_STRING_LIB nvdm_060[] = LOG_ERROR_PREFIX(nvdm) "pnum=%d";
    ATTR_LOG_STRING_LIB nvdm_061[] = LOG_ERROR_PREFIX(nvdm) "offset=0x%x";
    ATTR_LOG_STRING_LIB nvdm_062[] = LOG_ERROR_PREFIX(nvdm) "len=%d";
    ATTR_LOG_STRING_LIB nvdm_063[] = LOG_ERROR_PREFIX(nvdm) "magic=0x%x, erase_count=0x%x, status=0x%x, peb_reserved=0x%x";
    ATTR_LOG_STRING_LIB nvdm_064[] = LOG_ERROR_PREFIX(nvdm) "pnum=%d";
    ATTR_LOG_STRING_LIB nvdm_065[] = LOG_ERROR_PREFIX(nvdm) "offset=0x%x";
    ATTR_LOG_STRING_LIB nvdm_066[] = LOG_ERROR_PREFIX(nvdm) "len=%d";
    ATTR_LOG_STRING_LIB nvdm_067[] = LOG_ERROR_PREFIX(nvdm) "pnum=%d";
    ATTR_LOG_STRING_LIB nvdm_068[] = LOG_ERROR_PREFIX(nvdm) "pnum=%d";
    ATTR_LOG_STRING_LIB nvdm_069[] = LOG_ERROR_PREFIX(nvdm) "pnum=%d";
    ATTR_LOG_STRING_LIB nvdm_070[] = LOG_ERROR_PREFIX(nvdm) "pnum=%d";
    ATTR_LOG_STRING_LIB nvdm_071[] = LOG_ERROR_PREFIX(nvdm) "pnum=%d";
    ATTR_LOG_STRING_LIB nvdm_072[] = LOG_INFO_PREFIX(nvdm) "found no valid data in reclaiming pebs when relocate_pebs()";
    ATTR_LOG_STRING_LIB nvdm_073[] = LOG_ERROR_PREFIX(nvdm) "target_peb=%d";
    ATTR_LOG_STRING_LIB nvdm_074[] = LOG_INFO_PREFIX(nvdm) "found a target peb(%d) for reclaiming";
    ATTR_LOG_STRING_LIB nvdm_075[] = LOG_INFO_PREFIX(nvdm) "merge peb %d";
    ATTR_LOG_STRING_LIB nvdm_076[] = LOG_WARNING_PREFIX(nvdm) "start garbage collection!!!";
    ATTR_LOG_STRING_LIB nvdm_077[] = LOG_ERROR_PREFIX(nvdm) "peb_list alloc fail";
    ATTR_LOG_STRING_LIB nvdm_078[] = LOG_INFO_PREFIX(nvdm) "non_reserved_pebs = %d";
    ATTR_LOG_STRING_LIB nvdm_079[] = LOG_INFO_PREFIX(nvdm) "mean_erase_count = %d";
    ATTR_LOG_STRING_LIB nvdm_080[] = LOG_INFO_PREFIX(nvdm) "reclaim blocks select by erase count = %d";
    ATTR_LOG_STRING_LIB nvdm_081[] = LOG_INFO_PREFIX(nvdm) "reclaim peb_list(no-sort): ";
    ATTR_LOG_STRING_LIB nvdm_082[] = LOG_INFO_PREFIX(nvdm) "%d";
    ATTR_LOG_STRING_LIB nvdm_083[] = LOG_INFO_PREFIX(nvdm) "reclaim peb_list(sort): ";
    ATTR_LOG_STRING_LIB nvdm_084[] = LOG_INFO_PREFIX(nvdm) "%d";
    ATTR_LOG_STRING_LIB nvdm_085[] = LOG_INFO_PREFIX(nvdm) "reclaim blocks select by valid size = %d";
    ATTR_LOG_STRING_LIB nvdm_086[] = LOG_INFO_PREFIX(nvdm) "reclaim peb_list(no-sort): ";
    ATTR_LOG_STRING_LIB nvdm_087[] = LOG_INFO_PREFIX(nvdm) "%d";
    ATTR_LOG_STRING_LIB nvdm_088[] = LOG_INFO_PREFIX(nvdm) "reclaim peb_list(sort): ";
    ATTR_LOG_STRING_LIB nvdm_089[] = LOG_INFO_PREFIX(nvdm) "%d";
    ATTR_LOG_STRING_LIB nvdm_090[] = LOG_INFO_PREFIX(nvdm) "find_free_peb: target_peb = %d, reserved_peb = %d, reserved_peb_cnt = %d\n";
    ATTR_LOG_STRING_LIB nvdm_091[] = LOG_WARNING_PREFIX(nvdm) "total avail space = %d\n";
    ATTR_LOG_STRING_LIB nvdm_092[] = LOG_INFO_PREFIX(nvdm) "space_is_enough: g_valid_data_size = %d, new add size = %d\n";
    ATTR_LOG_STRING_LIB nvdm_093[] = LOG_ERROR_PREFIX(nvdm) "detect g_valid_data_size abnormal";
    ATTR_LOG_STRING_LIB nvdm_094[] = LOG_ERROR_PREFIX(nvdm) "reclaiming_peb alloc fail";
    ATTR_LOG_STRING_LIB nvdm_095[] = LOG_INFO_PREFIX(nvdm) "scan and verify peb headers";
    ATTR_LOG_STRING_LIB nvdm_096[] = LOG_INFO_PREFIX(nvdm) "before verify peb header";
    ATTR_LOG_STRING_LIB nvdm_097[] = LOG_ERROR_PREFIX(nvdm) "peb_header validate fail, pnum=%d";
    ATTR_LOG_STRING_LIB nvdm_098[] = LOG_ERROR_PREFIX(nvdm) "find more than one transfering peb, frist=%d, second=%d";
    ATTR_LOG_STRING_LIB nvdm_099[] = LOG_ERROR_PREFIX(nvdm) "find more than one transfered peb, frist=%d, second=%d";
    ATTR_LOG_STRING_LIB nvdm_100[] = LOG_ERROR_PREFIX(nvdm) "peb_header validate fail, pnum=%d";
    ATTR_LOG_STRING_LIB nvdm_101[] = LOG_ERROR_PREFIX(nvdm) "peb_header validate fail, pnum=%d";
    ATTR_LOG_STRING_LIB nvdm_102[] = LOG_INFO_PREFIX(nvdm) "after verify peb header";
    ATTR_LOG_STRING_LIB nvdm_103[] = LOG_INFO_PREFIX(nvdm) "transfering_peb = %d";
    ATTR_LOG_STRING_LIB nvdm_104[] = LOG_INFO_PREFIX(nvdm) "transfered_peb = %d";
    ATTR_LOG_STRING_LIB nvdm_105[] = LOG_INFO_PREFIX(nvdm) "reclaiming_peb[%d] = %d";
    ATTR_LOG_STRING_LIB nvdm_106[] = LOG_INFO_PREFIX(nvdm) "update erase count for unknown pebs";
    ATTR_LOG_STRING_LIB nvdm_107[] = LOG_INFO_PREFIX(nvdm) "scan all non-reserved pebs including reclaiming pebs and transfering peb";
    ATTR_LOG_STRING_LIB nvdm_108[] = LOG_INFO_PREFIX(nvdm) "found a peb in transfering status";
    ATTR_LOG_STRING_LIB nvdm_109[] = LOG_INFO_PREFIX(nvdm) "found a peb in transfered status";
    ATTR_LOG_STRING_LIB nvdm_110[] = LOG_ERROR_PREFIX(nvdm) "reclaim_idx=%d, transfered_peb=%d, transfering_peb=%d";
    ATTR_LOG_STRING_LIB nvdm_111[] = LOG_INFO_PREFIX(nvdm) "calculate total valid data size";
    ATTR_LOG_STRING_LIB nvdm_112[] = LOG_ERROR_PREFIX(nvdm) "Count of PEB for NVDM region must greater than or equal to 2";
    ATTR_LOG_STRING_LIB nvdm_113[] = LOG_ERROR_PREFIX(nvdm) "alloc peb_info fail";
    ATTR_LOG_STRING_LIB nvdm_114[] = LOG_WARNING_PREFIX(nvdm) "nvdm init finished\n";
#else /* !defined(MTK_DEBUG_LEVEL_NONE) */
    ATTR_LOG_STRING_LIB nvdm_001[] = "";
    ATTR_LOG_STRING_LIB nvdm_002[] = "";
    ATTR_LOG_STRING_LIB nvdm_003[] = "";
    ATTR_LOG_STRING_LIB nvdm_004[] = "";
    ATTR_LOG_STRING_LIB nvdm_005[] = "";
    ATTR_LOG_STRING_LIB nvdm_006[] = "";
    ATTR_LOG_STRING_LIB nvdm_007[] = "";
    ATTR_LOG_STRING_LIB nvdm_008[] = "";
    ATTR_LOG_STRING_LIB nvdm_009[] = "";
    ATTR_LOG_STRING_LIB nvdm_010[] = "";
    ATTR_LOG_STRING_LIB nvdm_011[] = "";
    ATTR_LOG_STRING_LIB nvdm_012[] = "";
    ATTR_LOG_STRING_LIB nvdm_013[] = "";
    ATTR_LOG_STRING_LIB nvdm_015[] = "";
    ATTR_LOG_STRING_LIB nvdm_016[] = "";
    ATTR_LOG_STRING_LIB nvdm_017[] = "";
    ATTR_LOG_STRING_LIB nvdm_018[] = "";
    ATTR_LOG_STRING_LIB nvdm_019[] = "";
    ATTR_LOG_STRING_LIB nvdm_020[] = "";
    ATTR_LOG_STRING_LIB nvdm_022[] = "";
    ATTR_LOG_STRING_LIB nvdm_023[] = "";
    ATTR_LOG_STRING_LIB nvdm_024[] = "";
    ATTR_LOG_STRING_LIB nvdm_025[] = "";
    ATTR_LOG_STRING_LIB nvdm_026[] = "";
    ATTR_LOG_STRING_LIB nvdm_027[] = "";
    ATTR_LOG_STRING_LIB nvdm_028[] = "";
    ATTR_LOG_STRING_LIB nvdm_029[] = "";
    ATTR_LOG_STRING_LIB nvdm_030[] = "";
    ATTR_LOG_STRING_LIB nvdm_031[] = "";
    ATTR_LOG_STRING_LIB nvdm_032[] = "";
    ATTR_LOG_STRING_LIB nvdm_034[] = "";
    ATTR_LOG_STRING_LIB nvdm_035[] = "";
    ATTR_LOG_STRING_LIB nvdm_036[] = "";
    ATTR_LOG_STRING_LIB nvdm_037[] = "";
    ATTR_LOG_STRING_LIB nvdm_038[] = "";
    ATTR_LOG_STRING_LIB nvdm_039[] = "";
    ATTR_LOG_STRING_LIB nvdm_040[] = "";
    ATTR_LOG_STRING_LIB nvdm_041[] = "";
    ATTR_LOG_STRING_LIB nvdm_042[] = "";
    ATTR_LOG_STRING_LIB nvdm_043[] = "";
    ATTR_LOG_STRING_LIB nvdm_044[] = "";
    ATTR_LOG_STRING_LIB nvdm_045[] = "";
    ATTR_LOG_STRING_LIB nvdm_046[] = "";
    ATTR_LOG_STRING_LIB nvdm_047[] = "";
    ATTR_LOG_STRING_LIB nvdm_048[] = "";
    ATTR_LOG_STRING_LIB nvdm_049[] = "";
    ATTR_LOG_STRING_LIB nvdm_050[] = "";
    ATTR_LOG_STRING_LIB nvdm_051[] = "";
    ATTR_LOG_STRING_LIB nvdm_052[] = "";
    ATTR_LOG_STRING_LIB nvdm_053[] = "";
    ATTR_LOG_STRING_LIB nvdm_054[] = "";
    ATTR_LOG_STRING_LIB nvdm_055[] = "";
    ATTR_LOG_STRING_LIB nvdm_056[] = "";
    ATTR_LOG_STRING_LIB nvdm_057[] = "";
    ATTR_LOG_STRING_LIB nvdm_058[] = "";
    ATTR_LOG_STRING_LIB nvdm_059[] = "";
    ATTR_LOG_STRING_LIB nvdm_060[] = "";
    ATTR_LOG_STRING_LIB nvdm_061[] = "";
    ATTR_LOG_STRING_LIB nvdm_062[] = "";
    ATTR_LOG_STRING_LIB nvdm_063[] = "";
    ATTR_LOG_STRING_LIB nvdm_064[] = "";
    ATTR_LOG_STRING_LIB nvdm_065[] = "";
    ATTR_LOG_STRING_LIB nvdm_066[] = "";
    ATTR_LOG_STRING_LIB nvdm_067[] = "";
    ATTR_LOG_STRING_LIB nvdm_068[] = "";
    ATTR_LOG_STRING_LIB nvdm_069[] = "";
    ATTR_LOG_STRING_LIB nvdm_070[] = "";
    ATTR_LOG_STRING_LIB nvdm_071[] = "";
    ATTR_LOG_STRING_LIB nvdm_072[] = "";
    ATTR_LOG_STRING_LIB nvdm_073[] = "";
    ATTR_LOG_STRING_LIB nvdm_074[] = "";
    ATTR_LOG_STRING_LIB nvdm_075[] = "";
    ATTR_LOG_STRING_LIB nvdm_076[] = "";
    ATTR_LOG_STRING_LIB nvdm_077[] = "";
    ATTR_LOG_STRING_LIB nvdm_078[] = "";
    ATTR_LOG_STRING_LIB nvdm_079[] = "";
    ATTR_LOG_STRING_LIB nvdm_080[] = "";
    ATTR_LOG_STRING_LIB nvdm_081[] = "";
    ATTR_LOG_STRING_LIB nvdm_082[] = "";
    ATTR_LOG_STRING_LIB nvdm_083[] = "";
    ATTR_LOG_STRING_LIB nvdm_084[] = "";
    ATTR_LOG_STRING_LIB nvdm_085[] = "";
    ATTR_LOG_STRING_LIB nvdm_086[] = "";
    ATTR_LOG_STRING_LIB nvdm_087[] = "";
    ATTR_LOG_STRING_LIB nvdm_088[] = "";
    ATTR_LOG_STRING_LIB nvdm_089[] = "";
    ATTR_LOG_STRING_LIB nvdm_090[] = "";
    ATTR_LOG_STRING_LIB nvdm_091[] = "";
    ATTR_LOG_STRING_LIB nvdm_092[] = "";
    ATTR_LOG_STRING_LIB nvdm_093[] = "";
    ATTR_LOG_STRING_LIB nvdm_094[] = "";
    ATTR_LOG_STRING_LIB nvdm_095[] = "";
    ATTR_LOG_STRING_LIB nvdm_096[] = "";
    ATTR_LOG_STRING_LIB nvdm_097[] = "";
    ATTR_LOG_STRING_LIB nvdm_098[] = "";
    ATTR_LOG_STRING_LIB nvdm_099[] = "";
    ATTR_LOG_STRING_LIB nvdm_100[] = "";
    ATTR_LOG_STRING_LIB nvdm_101[] = "";
    ATTR_LOG_STRING_LIB nvdm_102[] = "";
    ATTR_LOG_STRING_LIB nvdm_103[] = "";
    ATTR_LOG_STRING_LIB nvdm_104[] = "";
    ATTR_LOG_STRING_LIB nvdm_105[] = "";
    ATTR_LOG_STRING_LIB nvdm_106[] = "";
    ATTR_LOG_STRING_LIB nvdm_107[] = "";
    ATTR_LOG_STRING_LIB nvdm_108[] = "";
    ATTR_LOG_STRING_LIB nvdm_109[] = "";
    ATTR_LOG_STRING_LIB nvdm_110[] = "";
    ATTR_LOG_STRING_LIB nvdm_111[] = "";
    ATTR_LOG_STRING_LIB nvdm_112[] = "";
    ATTR_LOG_STRING_LIB nvdm_113[] = "";
    ATTR_LOG_STRING_LIB nvdm_114[] = "";
#endif /* !defined(MTK_DEBUG_LEVEL_NONE) */

log_create_module(nvdm, PRINT_LEVEL_INFO);

#if !defined (MTK_DEBUG_LEVEL_NONE)

void nvdm_log_info(const char *message, ...)
{
    va_list ap;

    va_start(ap, message);
    //vprint_module_log(&LOG_CONTROL_BLOCK_SYMBOL(nvdm), __FUNCTION__, __LINE__, PRINT_LEVEL_INFO, message, ap);
    va_end(ap);
}

void nvdm_log_warning(const char *message, ...)
{
    va_list ap;

    va_start(ap, message);
    vprint_module_log(&LOG_CONTROL_BLOCK_SYMBOL(nvdm), __FUNCTION__, __LINE__, PRINT_LEVEL_WARNING, message, ap);
    va_end(ap);
}

void nvdm_log_error(const char *message, ...)
{
    va_list ap;

    va_start(ap, message);
    vprint_module_log(&LOG_CONTROL_BLOCK_SYMBOL(nvdm), __FUNCTION__, __LINE__, PRINT_LEVEL_ERROR, message, ap);
    va_end(ap);

    assert(0);
}

void nvdm_log_msgid_info(const char *message, uint32_t arg_cnt, ...)
{
    va_list ap;

    va_start(ap, arg_cnt);
    //log_print_msgid(&LOG_CONTROL_BLOCK_SYMBOL(nvdm), PRINT_LEVEL_INFO, message, arg_cnt, ap);
    va_end(ap);
}

void nvdm_log_msgid_warning(const char *message, uint32_t arg_cnt, ...)
{
    va_list ap;

    va_start(ap, arg_cnt);
    log_print_msgid(&LOG_CONTROL_BLOCK_SYMBOL(nvdm), PRINT_LEVEL_WARNING, message, arg_cnt, ap);
    va_end(ap);
}

void nvdm_log_msgid_error(const char *message, uint32_t arg_cnt, ...)
{
    va_list ap;

    va_start(ap, arg_cnt);
    log_print_msgid(&LOG_CONTROL_BLOCK_SYMBOL(nvdm), PRINT_LEVEL_ERROR, message, arg_cnt, ap);
    va_end(ap);

    assert(0);
}

#else

void nvdm_log_info(const char *message, ...)
{}

void nvdm_log_warning(const char *message, ...)
{}

void nvdm_log_error(const char *message, ...)
{
    assert(0);
}

void nvdm_log_msgid_info(const char *message, uint32_t arg_cnt, ...)
{}

void nvdm_log_msgid_warning(const char *message, uint32_t arg_cnt, ...)
{}

void nvdm_log_msgid_error(const char *message, uint32_t arg_cnt, ...)
{
    assert(0);
}

#endif

void nvdm_port_flash_read(uint32_t address, uint8_t *buffer, uint32_t length)
{
    hal_flash_status_t status;

    status = hal_flash_read(address, buffer, length);
    if (status != HAL_FLASH_STATUS_OK) {
        LOG_MSGID_E(nvdm, "hal_flash_read: address = 0x%08x, buffer = 0x%08x, length = %d, status = %d", 4, address, (uint32_t)buffer, length, status);
    }
}

void nvdm_port_flash_write(uint32_t address, const uint8_t *buffer, uint32_t length)
{
    hal_flash_status_t status;

    status = hal_flash_write(address, buffer, length);
    if (status != HAL_FLASH_STATUS_OK) {
        LOG_MSGID_E(nvdm, "hal_flash_write: address = 0x%08x, buffer = 0x%08x, length = %d, status = %d", 4, address, (uint32_t)buffer, length, status);
    }
}

/* erase unit is 4K large(which is size of PEB) */
void nvdm_port_flash_erase(uint32_t address)
{
    hal_flash_status_t status;

    status = hal_flash_erase(address, HAL_FLASH_BLOCK_4K);
    if (status != HAL_FLASH_STATUS_OK) {
        LOG_MSGID_E(nvdm, "hal_flash_erase: address = 0x%08x, status = %d", 2, address, status);
    }
}

#ifdef FREERTOS_ENABLE
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

typedef SemaphoreHandle_t nvdm_port_mutex_t;
static nvdm_port_mutex_t g_nvdm_mutex;
static TaskHandle_t g_task_handler;

void nvdm_port_mutex_creat(void)
{
    g_nvdm_mutex = xSemaphoreCreateMutex();

    if (g_nvdm_mutex == NULL) {
        LOG_MSGID_E(nvdm, "nvdm_port_mutex_creat error\r\n", 0);
        return;
    }

    LOG_MSGID_I(nvdm, "nvdm_port_mutex_creat successfully", 0);
}

void nvdm_port_mutex_take(void)
{
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        if (xSemaphoreTake(g_nvdm_mutex, portMAX_DELAY) == pdFALSE) {
            LOG_MSGID_E(nvdm, "nvdm_port_mutex_take error\r\n", 0);
            return;
        }
        LOG_MSGID_I(nvdm, "nvdm_port_mutex_take successfully", 0);
    }
}

void nvdm_port_mutex_give(void)
{
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        if (xSemaphoreGive(g_nvdm_mutex) == pdFALSE) {
            LOG_MSGID_E(nvdm, "nvdm_port_mutex_give error\r\n", 0);
            return;
        }
        LOG_MSGID_I(nvdm, "nvdm_port_mutex_give successfully", 0);
    }
}

void *nvdm_port_malloc(uint32_t size)
{
    return pvPortMalloc(size);
}

void nvdm_port_free(void *pdata)
{
    vPortFree(pdata);
}

void nvdm_port_get_task_handler(void)
{
    g_task_handler = xTaskGetCurrentTaskHandle();
}

bool nvdm_port_query_task_handler(void)
{
    if (xTaskGetCurrentTaskHandle() != g_task_handler) {
        return false;
    }

    return true;
}

void nvdm_port_task_delay(void)
{
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        vTaskDelay(50);
    }
}

#ifdef SYSTEM_DAEMON_TASK_ENABLE
#include "system_daemon.h"

bool nvdm_port_send_queue(const void *p_item)
{
    BaseType_t ret;

    ret = system_daemon_send_message(SYSTEM_DAEMON_ID_NVDM, p_item);
    if (ret != pdPASS) {
        return false;
    }

    return true;
}
#else
bool nvdm_port_send_queue(const void *p_item)
{
    return false;
}
#endif

#else

#include "malloc.h"
void nvdm_port_mutex_creat(void)
{}

void nvdm_port_mutex_take(void)
{}

void nvdm_port_mutex_give(void)
{}

void *nvdm_port_malloc(uint32_t size)
{
    return malloc(size);
}

void nvdm_port_free(void *pdata)
{
    free(pdata);
}

void nvdm_port_get_task_handler(void)
{}

bool nvdm_port_query_task_handler(void)
{
    return true;
}

void nvdm_port_task_delay(void)
{}

bool nvdm_port_send_queue(const void *p_item)
{
    return false;
}

#endif

#if (PRODUCT_VERSION == 7687) || (PRODUCT_VERSION == 7697) || (PRODUCT_VERSION == 7686) || (PRODUCT_VERSION == 7698) || (PRODUCT_VERSION == 7682) || (PRODUCT_VERSION == 5932) || (PRODUCT_VERSION == 2533) || (PRODUCT_VERSION == 2523)

/* This macro defines max count of data items */
#define NVDM_PORT_DAT_ITEM_COUNT (200)
/* This macro defines size of PEB, normally it is size of flash block */
#define NVDM_PORT_PEB_SIZE (4096)
/* This macro defines max size of data item during all user defined data items.
 * 1. Must not define it greater than 2048 bytes.
 * 2. Define it as smaller as possible to enhance the utilization rate of NVDM region.
 * 2. Try your best to store small data less than 256 bytes.
 */
#define NVDM_PORT_MAX_DATA_ITEM_SIZE (2048)
/* This macro defines start address and PEB count of the NVDM region */
#if (PRODUCT_VERSION == 7687) || (PRODUCT_VERSION == 7697)
#include "flash_map.h"
#define NVDM_PORT_GET_REGION_ADDRESS(pnum, offset) (NVDM_BASE + pnum * NVDM_PORT_PEB_SIZE + offset)
#define NVDM_PORT_GET_REGION_PEB_COUNT    (NVDM_LENGTH / NVDM_PORT_PEB_SIZE)
#else
#include "memory_map.h"
#define NVDM_PORT_GET_REGION_ADDRESS(pnum, offset) (ROM_NVDM_BASE - HAL_FLASH_BASE_ADDRESS + pnum * NVDM_PORT_PEB_SIZE + offset)
#define NVDM_PORT_GET_REGION_PEB_COUNT    (ROM_NVDM_LENGTH / NVDM_PORT_PEB_SIZE)
#endif
/* This macro defines max length of group name of data item */
#define NVDM_PORT_GROUP_NAME_MAX_LENGTH (16)
/* This macro defines max length of data item name of data item */
#define NVDM_PORT_DATA_ITEM_NAME_MAX_LENGTH (32)

#else

#include "nvdm_config.h"
#define NVDM_PORT_GET_REGION_ADDRESS(pnum, offset)  (NVDM_PORT_REGION_ADDRESS - HAL_FLASH_BASE_ADDRESS + pnum * NVDM_PORT_PEB_SIZE + offset)
#define NVDM_PORT_GET_REGION_PEB_COUNT              (NVDM_PORT_REGION_SIZE / NVDM_PORT_PEB_SIZE)

#endif

uint32_t nvdm_port_get_data_item_config(uint32_t *max_data_item_size,
                                        uint32_t *max_group_name_size,
                                        uint32_t *max_data_item_name_size)
{
    *max_data_item_size = NVDM_PORT_MAX_DATA_ITEM_SIZE;
    *max_group_name_size = NVDM_PORT_GROUP_NAME_MAX_LENGTH;
    *max_data_item_name_size = NVDM_PORT_DATA_ITEM_NAME_MAX_LENGTH;

    return NVDM_PORT_DAT_ITEM_COUNT;
}

uint32_t nvdm_port_get_peb_config(uint32_t *peb_count)
{
    *peb_count = NVDM_PORT_GET_REGION_PEB_COUNT;

    return NVDM_PORT_PEB_SIZE;
}

uint32_t nvdm_port_get_peb_address(int32_t pnum, int32_t offset)
{
    return NVDM_PORT_GET_REGION_ADDRESS(pnum, offset);
}

void nvdm_port_poweroff_time_set(void)
{}
void nvdm_port_poweroff(uint32_t poweroff_time)
{}

#endif


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

#ifndef __NVDM_MSGID_LOG_H__
#define __NVDM_MSGID_LOG_H__

#ifdef MTK_NVDM_ENABLE

#include <stdint.h>

/*  "data item header info show below:"  */
extern const char nvdm_001[];
/*  "status: 0x%02x"  */
extern const char nvdm_002[];
/*  "pnum: %d"  */
extern const char nvdm_003[];
/*  "offset: 0x%04x"  */
extern const char nvdm_004[];
/*  "sequence_number: %d"  */
extern const char nvdm_005[];
/*  "group_name_size: %d"  */
extern const char nvdm_006[];
/*  "data_item_name_size: %d"  */
extern const char nvdm_007[];
/*  "value_size: %d"  */
extern const char nvdm_008[];
/*  "index: %d"  */
extern const char nvdm_009[];
/*  "type: %d"  */
extern const char nvdm_010[];
/*  "hash_name: 0x%08x"  */
extern const char nvdm_011[];
/*  "hashname = 0x%08x"  */
extern const char nvdm_012[];
/*  "nvdm_read_data_item: begin to read"  */
extern const char nvdm_013[];
/*  "nvdm_write_data_item: begin to write"  */
extern const char nvdm_015[];
/*  "find_data_item_by_hashname return %d"  */
extern const char nvdm_016[];
/*  "peb free space is not enough\n"  */
extern const char nvdm_017[];
/*  "too many data items in nvdm region\n"  */
extern const char nvdm_018[];
/*  "new data item append"  */
extern const char nvdm_019[];
/*  "old data item overwrite"  */
extern const char nvdm_020[];
/*  "nvdm_write_data_item_non_blocking: begin to write"  */
extern const char nvdm_022[];
/*  "Can't alloc memory!!"  */
extern const char nvdm_023[];
/*  "Can't send queue!!"  */
extern const char nvdm_024[];
/*  "nvdm_delete_data_item: enter"  */
extern const char nvdm_025[];
/*  "nvdm_delete_group: enter"  */
extern const char nvdm_026[];
/*  "nvdm_delete_all: enter"  */
extern const char nvdm_027[];
/*  "nvdm_query_begin: enter"  */
extern const char nvdm_028[];
/*  "nvdm_query_end: enter"  */
extern const char nvdm_029[];
/*  "nvdm_query_next_group_name: enter"  */
extern const char nvdm_030[];
/*  "nvdm_query_next_data_item_name: enter"  */
extern const char nvdm_031[];
/*  "nvdm_query_data_item_length: begin to query"  */
extern const char nvdm_032[];
/*  "scanning pnum(%d) to analysis data item info"  */
extern const char nvdm_034[];
/*  "pnum=%d, offset=0x%x"  */
extern const char nvdm_035[];
/*  "Detect index of data item with out of range, max = %d, curr = %d"  */
extern const char nvdm_036[];
/*  "detect checksum error\n"  */
extern const char nvdm_037[];
/*  "too many data items in nvdm region\n"  */
extern const char nvdm_038[];
/*  "detect two valid copy of data item"  */
extern const char nvdm_039[];
/*  "copy1(pnum=%d, offset=0x%04x), copy2(pnum=%d, offset=0x%04x)\n"  */
extern const char nvdm_040[];
/*  "abnormal_data_item = %d"  */
extern const char nvdm_041[];
/*  "Max size of data item must less than or equal to 2048 bytes"  */
extern const char nvdm_042[];
/*  "alloc data_item_headers fail"  */
extern const char nvdm_043[];
/*  "old_src_pnum=%d, old_pos=0x%x, new_src_pnum=%d, new_pos=0x%x"  */
extern const char nvdm_044[];
/*  "src_pnum=%d, pos=0x%x"  */
extern const char nvdm_045[];
/*  "pnum=%d, offset=0x%x, len=%d"  */
extern const char nvdm_046[];
/*  "addr=0x%x, pnum=%d, offset=0x%x, len=%d"  */
extern const char nvdm_047[];
/*  "pnum=%d"  */
extern const char nvdm_048[];
/*  "region info show below:"  */
extern const char nvdm_049[];
/*  "peb    free    dirty    erase_count    is_reserved"  */
extern const char nvdm_050[];
/*  "%d     %d     %d     %d      %d"  */
extern const char nvdm_051[];
/*  "g_valid_data_size = %d"  */
extern const char nvdm_052[];
/*  "peb header(%d) info show below:"  */
extern const char nvdm_053[];
/*  "magic: %08x"  */
extern const char nvdm_054[];
/*  "erase_count: %08x"  */
extern const char nvdm_055[];
/*  "status: %02x"  */
extern const char nvdm_056[];
/*  "peb_reserved: %02x"  */
extern const char nvdm_057[];
/*  "version: %02x"  */
extern const char nvdm_058[];
/*  "pnum=%d"  */
extern const char nvdm_059[];
/*  "pnum=%d"  */
extern const char nvdm_060[];
/*  "offset=0x%x"  */
extern const char nvdm_061[];
/*  "len=%d"  */
extern const char nvdm_062[];
/*  "magic=0x%x, erase_count=0x%x, status=0x%x, peb_reserved=0x%x"  */
extern const char nvdm_063[];
/*  "pnum=%d"  */
extern const char nvdm_064[];
/*  "offset=0x%x"  */
extern const char nvdm_065[];
/*  "len=%d"  */
extern const char nvdm_066[];
/*  "pnum=%d"  */
extern const char nvdm_067[];
/*  "pnum=%d"  */
extern const char nvdm_068[];
/*  "pnum=%d"  */
extern const char nvdm_069[];
/*  "pnum=%d"  */
extern const char nvdm_070[];
/*  "pnum=%d"  */
extern const char nvdm_071[];
/*  "found no valid data in reclaiming pebs when relocate_pebs()"  */
extern const char nvdm_072[];
/*  "target_peb=%d"  */
extern const char nvdm_073[];
/*  "found a target peb(%d) for reclaiming"  */
extern const char nvdm_074[];
/*  "merge peb %d"  */
extern const char nvdm_075[];
/*  "start garbage collection!!!"  */
extern const char nvdm_076[];
/*  "peb_list alloc fail"  */
extern const char nvdm_077[];
/*  "non_reserved_pebs = %d"  */
extern const char nvdm_078[];
/*  "mean_erase_count = %d"  */
extern const char nvdm_079[];
/*  "reclaim blocks select by erase count = %d"  */
extern const char nvdm_080[];
/*  "reclaim peb_list(no-sort): "  */
extern const char nvdm_081[];
/*  "%d"  */
extern const char nvdm_082[];
/*  "reclaim peb_list(sort): "  */
extern const char nvdm_083[];
/*  "%d"  */
extern const char nvdm_084[];
/*  "reclaim blocks select by valid size = %d"  */
extern const char nvdm_085[];
/*  "reclaim peb_list(no-sort): "  */
extern const char nvdm_086[];
/*  "%d"  */
extern const char nvdm_087[];
/*  "reclaim peb_list(sort): "  */
extern const char nvdm_088[];
/*  "%d"  */
extern const char nvdm_089[];
/*  "find_free_peb: target_peb = %d, reserved_peb = %d, reserved_peb_cnt = %d\n"  */
extern const char nvdm_090[];
/*  "total avail space = %d\n"  */
extern const char nvdm_091[];
/*  "space_is_enough: g_valid_data_size = %d, new add size = %d\n"  */
extern const char nvdm_092[];
/*  "detect g_valid_data_size abnormal"  */
extern const char nvdm_093[];
/*  "reclaiming_peb alloc fail"  */
extern const char nvdm_094[];
/*  "scan and verify peb headers"  */
extern const char nvdm_095[];
/*  "before verify peb header"  */
extern const char nvdm_096[];
/*  "peb_header validate fail, pnum=%d"  */
extern const char nvdm_097[];
/*  "find more than one transfering peb, frist=%d, second=%d"  */
extern const char nvdm_098[];
/*  "find more than one transfered peb, frist=%d, second=%d"  */
extern const char nvdm_099[];
/*  "peb_header validate fail, pnum=%d"  */
extern const char nvdm_100[];
/*  "peb_header validate fail, pnum=%d"  */
extern const char nvdm_101[];
/*  "after verify peb header"  */
extern const char nvdm_102[];
/*  "transfering_peb = %d"  */
extern const char nvdm_103[];
/*  "transfered_peb = %d"  */
extern const char nvdm_104[];
/*  "reclaiming_peb[%d] = %d"  */
extern const char nvdm_105[];
/*  "update erase count for unknown pebs"  */
extern const char nvdm_106[];
/*  "scan all non-reserved pebs including reclaiming pebs and transfering peb"  */
extern const char nvdm_107[];
/*  "found a peb in transfering status"  */
extern const char nvdm_108[];
/*  "found a peb in transfered status"  */
extern const char nvdm_109[];
/*  "reclaim_idx=%d, transfered_peb=%d, transfering_peb=%d"  */
extern const char nvdm_110[];
/*  "calculate total valid data size"  */
extern const char nvdm_111[];
/*  "Count of PEB for NVDM region must greater than or equal to 2"  */
extern const char nvdm_112[];
/*  "alloc peb_info fail"  */
extern const char nvdm_113[];
/*  "nvdm init finished\n"  */
extern const char nvdm_114[];

#endif

#endif


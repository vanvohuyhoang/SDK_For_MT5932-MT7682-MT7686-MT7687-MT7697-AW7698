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

/*
 * [regions of PEBs]
 * we maintain NVDM_PEB_COUNT logic erase blocks, including main PEBs and reserved PEBs.
 * main PEBs are used to store data items header and actural data.
 * reserved PEBs are used in garbage collection to store data item's content merged from other main dirty PEBs.
 *
 * [composition of every PEB]
 * dirty data + valid data + free space
 *
 * [Flow for PEB allocation]
 * 1, find most best fit PEB with required free space;
 * 2, if found, return number and offset of PEB;
 * 3, if not found, begin to start garbage collection;
 * 4, scan PEBs to find victims, and it's criteria is:
 *      - find erase count of PEB less than average erase count to much;
 *      - try to merge PEBs to reserved PEBs;
 * 5, sort victims PEBs with valid data items size;
 * 6, try to merge PEBs to reserved PEBs;
 *      - at least two PEBs can be merge into one reserved PEB;
 * 7, if merge fail, directlly reclaim least valid size space PEB;
 *      - we should report this situation;
 */

#include "nvdm.h"
#include "nvdm_port.h"
#include "nvdm_internal.h"
#include "nvdm_msgid_log.h"

typedef struct {
    uint16_t free;
    uint16_t dirty;
    int32_t is_reserved;
    uint32_t erase_count;
} peb_info_t;

/* status of every peb */
static peb_info_t *g_pebs_info;
/* current valid data size in all main pebs  */
static int32_t g_valid_data_size = 0;
/* total avail space for storge data items */
static uint32_t g_total_avail_space = 0;
/* status of nvdm module initialization */
bool nvdm_init_status = false;
/* PEB size decided by flash device */
uint32_t g_nvdm_peb_size;
/* PEB count cunstom by user */
uint32_t g_nvdm_peb_count;

extern uint8_t g_working_buffer[];
extern uint32_t g_max_data_item_size;
extern uint32_t g_max_group_name_size;
extern uint32_t g_max_data_item_name_size;

static void peb_print_info(void)
{
    uint32_t i;

    nvdm_log_msgid_info(nvdm_049, 0);
    nvdm_log_msgid_info(nvdm_050, 0);
    for (i = 0; i < g_nvdm_peb_count; i++) {
        nvdm_log_msgid_info(nvdm_051, 5,
                      i,
                      g_pebs_info[i].free,
                      g_pebs_info[i].dirty,
                      g_pebs_info[i].erase_count,
                      g_pebs_info[i].is_reserved);
    }
    nvdm_log_msgid_warning(nvdm_052, 1, g_valid_data_size);
}
static void peb_header_print_info(uint32_t peb_index, peb_header_t *peb_hdr)
{
    nvdm_log_msgid_info(nvdm_053, 1, peb_index);
    nvdm_log_msgid_info(nvdm_054, 1, peb_hdr->magic);
    nvdm_log_msgid_info(nvdm_055, 1, peb_hdr->erase_count);
    nvdm_log_msgid_info(nvdm_056, 1, peb_hdr->status);
    nvdm_log_msgid_info(nvdm_057, 1, peb_hdr->peb_reserved);
    nvdm_log_msgid_info(nvdm_058, 1, peb_hdr->version);
}

static bool peb_header_is_validate(peb_header_t *peb_header, int32_t is_empty)
{
    if (peb_header->magic != PEB_HEADER_MAGIC) {
        return false;
    }

    if (peb_header->erase_count == ERASE_COUNT_MAX) {
        return false;
    }

    if (peb_header->version != NVDM_VERSION) {
        return false;
    }

    if (is_empty) {
        if ((peb_header->status != PEB_STATUS_EMPTY) ||
                (peb_header->peb_reserved != 0xFF)) {
            return false;
        }
    } else {
        if (peb_header->peb_reserved == 0xFF) {
            return false;
        }
    }

    return true;
}

void peb_read_header(int32_t pnum, peb_header_t *peb_header)
{
    uint8_t buf[PEB_HEADER_SIZE];

    if (pnum >= g_nvdm_peb_count) {
        nvdm_log_msgid_error(nvdm_059, 1, pnum);
        return;
    }

    peb_io_read(pnum, 0, buf, PEB_HEADER_SIZE);

    if (peb_header) {
        *peb_header = *(peb_header_t *)buf;
    }
}

void peb_write_data(int32_t pnum, int32_t offset, const uint8_t *buf, int32_t len)
{
    int32_t ret;
    peb_header_t peb_header;

    if (pnum >= g_nvdm_peb_count) {
        nvdm_log_msgid_error(nvdm_060, 1, pnum);
        return;
    }
    if (offset >= g_nvdm_peb_size - PEB_HEADER_SIZE) {
        nvdm_log_msgid_error(nvdm_061, 1, offset);
        return;
    }
    if (len > g_nvdm_peb_size - PEB_HEADER_SIZE) {
        nvdm_log_msgid_error(nvdm_062, 1, len);
        return;
    }

    /*
     * We write to the data area of the physical eraseblock. Make
     * sure it has valid EC headers.
     */
    peb_read_header(pnum, &peb_header);
    ret = peb_header_is_validate(&peb_header, 0);
    if (ret == false) {
        nvdm_log_msgid_error(nvdm_063, 4,
                       peb_header.magic, peb_header.erase_count,
                       peb_header.status, peb_header.peb_reserved);
        return;
    }

    peb_io_write(pnum, PEB_HEADER_SIZE + offset, buf, len);
}

void peb_read_data(int32_t pnum, int32_t offset, uint8_t *buf, int32_t len)
{
    if (pnum >= g_nvdm_peb_count) {
        nvdm_log_msgid_error(nvdm_064, 1, pnum);
        return;
    }
    if (offset >= g_nvdm_peb_size - PEB_HEADER_SIZE) {
        nvdm_log_msgid_error(nvdm_065, 1, offset);
        return;
    }
    if (len > g_nvdm_peb_size - PEB_HEADER_SIZE) {
        nvdm_log_msgid_error(nvdm_066, 1, len);
        return;
    }

    peb_io_read(pnum, PEB_HEADER_SIZE + offset, buf, len);
}

void peb_update_status(int32_t pnum, peb_status_t status)
{
    if (pnum >= g_nvdm_peb_count) {
        nvdm_log_msgid_error(nvdm_067, 1, pnum);
        return;
    }

    peb_io_write(pnum, PEB_STATUS_OFFSET, (uint8_t *)&status, 1);
}

static void peb_write_hdr_erase_count(int32_t pnum, uint32_t erase_count)
{
    if (pnum >= g_nvdm_peb_count) {
        nvdm_log_msgid_error(nvdm_068, 1, pnum);
        return;
    }

    peb_io_write(pnum, PEB_ERASE_COUNT_OFFSET, (uint8_t *)&erase_count, 4);
}

static void peb_write_hdr_peb_reserved(int32_t pnum)
{
    uint8_t unreserved_mark = PEB_UNRESERVED;

    if (pnum >= g_nvdm_peb_count) {
        nvdm_log_msgid_error(nvdm_069, 1, pnum);
        return;
    }

    peb_io_write(pnum, PEB_RESERVED_OFFSET, &unreserved_mark, 1);
}

static void peb_write_hdr_magic_number(int32_t pnum)
{
    uint32_t magic_number;

    if (pnum >= g_nvdm_peb_count) {
        nvdm_log_msgid_error(nvdm_070, 1, pnum);
        return;
    }

    magic_number = PEB_HEADER_MAGIC;
    peb_io_write(pnum, PEB_MAGIC_OFFSET, (uint8_t *)&magic_number, 4);
}

static void peb_write_hdr_version(int32_t pnum)
{
    uint8_t version = NVDM_VERSION;

    if (pnum >= g_nvdm_peb_count) {
        nvdm_log_msgid_error(nvdm_071, 1, pnum);
        return;
    }

    peb_io_write(pnum, PEB_VERSION_OFFSET, &version, 1);
}

int32_t peb_activing(int32_t pnum)
{
    if (g_pebs_info[pnum].free == (g_nvdm_peb_size - PEB_HEADER_SIZE)) {
        peb_update_status(pnum, PEB_STATUS_ACTIVING);
        nvdm_port_poweroff(9);
        peb_write_hdr_peb_reserved(pnum);
        g_pebs_info[pnum].is_reserved = 0;
        return 1;
    }

    return 0;
}

static void peb_reclaim(int32_t pnum)
{
    peb_update_status(pnum, PEB_STATUS_ERASING);
    nvdm_port_poweroff(10);
    peb_erase(pnum);
    peb_write_hdr_magic_number(pnum);
    peb_write_hdr_erase_count(pnum, ++(g_pebs_info[pnum].erase_count));
    peb_write_hdr_version(pnum);
    peb_update_status(pnum, PEB_STATUS_EMPTY);
    nvdm_port_poweroff(11);
    g_pebs_info[pnum].is_reserved = 1;
    g_pebs_info[pnum].dirty = 0;
    g_pebs_info[pnum].free = (g_nvdm_peb_size - PEB_HEADER_SIZE);
}

/* relocate one or more pebs with large of dirty data to one new empty peb */
static void relocate_pebs(int32_t *source_pebs, int32_t source_peb_count)
{
    int32_t i, offset;
    int32_t target_peb;
    uint32_t least_erase_count, total_valid_data;

    /* there is a special case that all source_pebs cantain no valid data at all.
     * So we just reclaim these pebs and erase them.
     */
    total_valid_data = 0;
    for (i = 0; i < source_peb_count; i++) {
        total_valid_data += (g_nvdm_peb_size - PEB_HEADER_SIZE) - g_pebs_info[source_pebs[i]].dirty - g_pebs_info[source_pebs[i]].free;
    }
    if (total_valid_data == 0) {
        nvdm_log_msgid_info(nvdm_072, 0);
        for (i = 0; i < source_peb_count; i++) {
            peb_reclaim(source_pebs[i]);
        }

        return;
    }

    /* mark source pebs which is needed relocation */
    for (i = 0; i < source_peb_count; i++) {
        peb_update_status(source_pebs[i], PEB_STATUS_RECLAIMING);
        nvdm_port_poweroff(12);
    }

    /* search a target peb and mark it */
    least_erase_count = ERASE_COUNT_MAX;
    target_peb = g_nvdm_peb_count;
    for (i = 0; i < g_nvdm_peb_count; i++) {
        if (!g_pebs_info[i].is_reserved) {
            continue;
        }
        if (least_erase_count > g_pebs_info[i].erase_count) {
            target_peb = i;
            least_erase_count = g_pebs_info[i].erase_count;
        }
    }
    if (target_peb >= g_nvdm_peb_count) {
        nvdm_log_msgid_error(nvdm_073, 1, target_peb);
        return;
    }
    nvdm_log_msgid_info(nvdm_074, 1, target_peb);
    peb_update_status(target_peb, PEB_STATUS_ACTIVING);
    nvdm_port_poweroff(13);
    peb_write_hdr_peb_reserved(target_peb);
    g_pebs_info[target_peb].is_reserved = 0;
    peb_update_status(target_peb, PEB_STATUS_TRANSFERING);
    nvdm_port_poweroff(14);

    /* begin transfer data from source pebs to target peb */
    offset = 0;
    for (i = 0; i < source_peb_count; i++) {
        offset = date_item_migration(source_pebs[i], target_peb, offset);
    }

    /* We can't dirrectly update status of discard pebs to PEB_STATUS_ACTIVED,
     * or we can't deal with those pebs in init flow if power-off happen below.
     */
    peb_update_status(target_peb, PEB_STATUS_TRANSFERED);
    nvdm_port_poweroff(15);
    g_pebs_info[target_peb].is_reserved = 0;

    /* put back all discard pebs to wear-leaving */
    for (i = 0; i < source_peb_count; i++) {
        peb_reclaim(source_pebs[i]);
    }

    /* Now we can update status of target peb to PEB_STATUS_ACTIVED safety */
    peb_update_status(target_peb, PEB_STATUS_ACTIVED);
    nvdm_port_poweroff(16);
}

/* Define max number of PEBs during each round of reclaim. */
#define MAX_MERGE_PEBS_PER_ROUND 3
/* Define max loop count during each garbage reclaim. */
#define MAX_MERGE_LOOP_PER_GARBAGE_COLLECTION 1
static void garbage_reclaim_pebs(int32_t found_blocks, int32_t *peb_list)
{
    int32_t i, sum_valid, merge_cnt, merge_start, hard_merge, merge_loop, curr_valid, empty_pebs;

    /* try to merge PEBs to reserved PEBs as many as possible.
       * The max number of merge PEBs and the max of merge loop need to be limited to prevent too many time stay here.
       * Also peb_list[] is list in order, so we need to handle merge of empty PEBs.
       */
    merge_start = 0;
    hard_merge = 0;
    merge_loop = MAX_MERGE_LOOP_PER_GARBAGE_COLLECTION;
    do {
        sum_valid = 0;
        merge_cnt = 0;
        empty_pebs = 0;
        for (i = merge_start; i < found_blocks; i++) {
            curr_valid = (g_nvdm_peb_size - PEB_HEADER_SIZE) - g_pebs_info[peb_list[i]].dirty - g_pebs_info[peb_list[i]].free;
            if (curr_valid == 0) {
                empty_pebs++;
            } else {
                if (empty_pebs > 0) {
                    break;
                } else {
                    sum_valid += curr_valid;
                    if (sum_valid > (g_nvdm_peb_size - PEB_HEADER_SIZE)) {
                        break;
                    }
                }
            }
            merge_cnt++;
            if (merge_cnt >= MAX_MERGE_PEBS_PER_ROUND) {
                break;
            }
        }
        /* no need to continue merge if merge_cnt less than 2 */
        if (merge_cnt < 2) {
            hard_merge = 1;
        }
        /* now let us start merging operation */
        for (i = merge_start; i < merge_start + merge_cnt; i++) {
            nvdm_log_msgid_info(nvdm_075, 1, peb_list[i]);
        }
        relocate_pebs(&peb_list[merge_start], merge_cnt);
        /* if merge loop reach limit or merge hard, stop merge attemption again. */
        if ((--merge_loop == 0) || (hard_merge == 1)) {
            break;
        }
        merge_start += merge_cnt;
    } while (merge_start < found_blocks);
}

/* NVDM will start garbage collection for PEBs
 * with PEB_ERASE_COUNT_CRITERION less than average erase count
 */
#define PEB_ERASE_COUNT_CRITERION 5
static void garbage_collection_peb(void)
{
    int32_t i, j, max;
    int32_t cur_valid, tmp_valid;
    uint32_t cur_erase_count, tmp_erase_count;
    int32_t found_blocks, non_reserved_pebs;
    int32_t *peb_list, tmp_peb;
    uint64_t mean_erase_count;

    nvdm_log_msgid_warning(nvdm_076, 0);

    peb_print_info();

    peb_list = nvdm_port_malloc(g_nvdm_peb_count * sizeof(int32_t));
    if (peb_list == NULL) {
        nvdm_log_msgid_error(nvdm_077, 0);
        return;
    }

    /* Method 1: Reclaim PEBs by erase count. */
    /* 1. scan all non-reserved pebs to calculate average erase counter. */
    mean_erase_count = 0;
    non_reserved_pebs = 0;
    for (i = 0; i < g_nvdm_peb_count; i++) {
        if (g_pebs_info[i].is_reserved) {
            continue;
        }
        mean_erase_count += g_pebs_info[i].erase_count;
        non_reserved_pebs++;
    }
    mean_erase_count /= non_reserved_pebs;
    nvdm_log_msgid_info(nvdm_078, 1, non_reserved_pebs);
    nvdm_log_msgid_info(nvdm_079, 1, mean_erase_count);
    /* 2. find all PEBs with erase counter below threshold. */
    found_blocks = 0;
    for (i = 0; i < g_nvdm_peb_count; i++) {
        if (g_pebs_info[i].is_reserved) {
            continue;
        }
        if (g_pebs_info[i].erase_count < (mean_erase_count * (100 - PEB_ERASE_COUNT_CRITERION)) / 100) {
            peb_list[found_blocks++] = i;
        }
    }
    /* 3. sort victims PEBs with erase count */
    if (found_blocks) {
        nvdm_log_msgid_info(nvdm_080, 1, found_blocks);
        nvdm_log_msgid_info(nvdm_081, 0);
        for (i = 0; i < found_blocks; i++) {
            nvdm_log_msgid_info(nvdm_082, 1, peb_list[i]);
        }
        for (i = 0; i < found_blocks; i++) {
            cur_erase_count = g_pebs_info[peb_list[i]].erase_count;
            max = i;
            for (j = i; j < found_blocks; j++) {
                tmp_erase_count = g_pebs_info[peb_list[j]].erase_count;
                if (cur_erase_count > tmp_erase_count) {
                    cur_erase_count = tmp_erase_count;
                    max = j;
                }
            }
            if (i != max) {
                tmp_peb = peb_list[max];
                peb_list[max] = peb_list[i];
                peb_list[i] = tmp_peb;
            }
        }
        nvdm_log_msgid_info(nvdm_083, 0);
        for (i = 0; i < found_blocks; i++) {
            nvdm_log_msgid_info(nvdm_084, 1, peb_list[i]);
        }
    }

    /* Method 2: Reclaim PEBs by valid size. */
    /* 1. find all non researved PEBs. */
    if (found_blocks == 0) {
        for (i = 0; i < g_nvdm_peb_count; i++) {
            if (g_pebs_info[i].is_reserved) {
                continue;
            }
            peb_list[found_blocks++] = i;
        }
        nvdm_log_msgid_info(nvdm_085, 1, found_blocks);
        nvdm_log_msgid_info(nvdm_086, 0);
        for (i = 0; i < found_blocks; i++) {
            nvdm_log_msgid_info(nvdm_087, 1, peb_list[i]);
        }
    }
    /* 2. sort victims PEBs with valid data items size. */
    for (i = 0; i < found_blocks; i++) {
        cur_valid = (g_nvdm_peb_size - PEB_HEADER_SIZE) - g_pebs_info[peb_list[i]].dirty - g_pebs_info[peb_list[i]].free;
        max = i;
        for (j = i; j < found_blocks; j++) {
            tmp_valid = (g_nvdm_peb_size - PEB_HEADER_SIZE) - g_pebs_info[peb_list[j]].dirty - g_pebs_info[peb_list[j]].free;
            if (cur_valid > tmp_valid) {
                cur_valid = tmp_valid;
                max = j;
            }
        }
        if (i != max) {
            tmp_peb = peb_list[max];
            peb_list[max] = peb_list[i];
            peb_list[i] = tmp_peb;
        }
    }
    nvdm_log_msgid_info(nvdm_088, 0);
    for (i = 0; i < found_blocks; i++) {
        nvdm_log_msgid_info(nvdm_089, 1, peb_list[i]);
    }
    /* 3. begin to reclaim those pebs */
    garbage_reclaim_pebs(found_blocks, peb_list);

    /* free the memory alloc before */
    nvdm_port_free(peb_list);

    peb_print_info();
}

static int32_t find_free_peb(int32_t size)
{
    int32_t i, reserved_peb = -1, reserved_peb_cnt, target_peb = -1;
    int32_t min_free_space = (g_nvdm_peb_size - PEB_HEADER_SIZE);
    uint32_t least_erase_count;

    /* find in non-reserved pebs frist */
    least_erase_count = ERASE_COUNT_MAX;
    reserved_peb_cnt = 0;
    for (i = 0; i < g_nvdm_peb_count; i++) {
        if (g_pebs_info[i].is_reserved) {
            reserved_peb_cnt++;
            if (least_erase_count > g_pebs_info[i].erase_count) {
                reserved_peb = i;
                least_erase_count = g_pebs_info[i].erase_count;
            }
            continue;
        }
        if (g_pebs_info[i].free > size) {
            if ((target_peb < 0) || (g_pebs_info[i].free < min_free_space)) {
                min_free_space = g_pebs_info[i].free;
                target_peb = i;
            }
        }
    }

    nvdm_log_msgid_info(nvdm_090, 3, target_peb, reserved_peb, reserved_peb_cnt);

    if (target_peb >= 0) {
        return target_peb;
    }

    /* use reserved peb if we have (exclude backup peb) */
    if (reserved_peb_cnt > NVDM_RESERVED_PEB_COUNT) {
        target_peb = reserved_peb;
    }

    return target_peb;
}

/* allocate a logic erase block with at least free space size */
int32_t space_allocation(int32_t alloc_size, int32_t added_size, int32_t *poffset)
{
    int32_t target_peb = -1;

    target_peb = find_free_peb(alloc_size);
    while (target_peb < 0) {
        garbage_collection_peb();
        target_peb = find_free_peb(alloc_size);
        if (target_peb >= 0) {
            break;
        }
        nvdm_port_task_delay();
    }

    *poffset = (g_nvdm_peb_size - PEB_HEADER_SIZE) - g_pebs_info[target_peb].free;
    g_valid_data_size += added_size;

    return target_peb;
}

/* This function decides max avail size of NVDM's region.
 * Normally we consider two factors:
 *  - Max size of data item during all user defined data items.
 *  - Limit total size of data items so that garbage collection don't happen too frequently.
 */
#define NVDM_MAX_USAGE_RATIO 80
static uint32_t calculate_total_avail_space(void)
{
    uint32_t max_reasonable_size, criteria1, criteria2;

    criteria1 = (NVDM_MAX_USAGE_RATIO *
                 (g_nvdm_peb_size - PEB_HEADER_SIZE) *
                 (g_nvdm_peb_count - NVDM_RESERVED_PEB_COUNT)) / 100;
    criteria2 = (g_nvdm_peb_count - NVDM_RESERVED_PEB_COUNT) *
                (g_nvdm_peb_size - PEB_HEADER_SIZE - g_max_data_item_size - DATA_ITEM_HEADER_SIZE - DATA_ITEM_CHECKSUM_SIZE - g_max_group_name_size - g_max_data_item_name_size - 2);
    if (criteria1 > criteria2) {
        max_reasonable_size = criteria2;
    } else {
        max_reasonable_size = criteria1;
    }

    nvdm_log_msgid_warning(nvdm_091, 1, max_reasonable_size);

    return max_reasonable_size;
}

bool space_is_enough(int32_t size)
{
    nvdm_log_msgid_info(nvdm_092, 2, g_valid_data_size, size);

    if ((g_valid_data_size + size) > g_total_avail_space) {
        return false;
    }

    return true;
}

void space_sub_valid(int32_t size)
{
    g_valid_data_size -= size;
    if (g_valid_data_size < 0) {
        nvdm_log_msgid_error(nvdm_093, 0);
    }
}

void peb_add_drity(int32_t pnum, int32_t drity)
{
    g_pebs_info[pnum].dirty += drity;
}

void peb_add_free(int32_t pnum, int32_t free)
{
    g_pebs_info[pnum].free += free;
}

void peb_sub_free(int32_t pnum, int32_t free)
{
    g_pebs_info[pnum].free -= free;
}

static void peb_scan(void)
{
    int32_t i, j, ret;
    peb_header_t peb_hdr;
    uint8_t peb_status;
    int32_t reclaim_idx, *reclaiming_peb;
    int32_t transfering_peb, transfered_peb;
    uint32_t g_mean_erase_count = 0;

    reclaiming_peb = nvdm_port_malloc(g_nvdm_peb_count * sizeof(int32_t));
    if (reclaiming_peb == NULL) {
        nvdm_log_msgid_error(nvdm_094, 0);
        return;
    }
    memset(reclaiming_peb, 0, (g_nvdm_peb_count * sizeof(int32_t)));

    for (i = 0; i < g_nvdm_peb_count; i++) {
        g_pebs_info[i].erase_count = ERASE_COUNT_MAX;
    }

    nvdm_log_msgid_info(nvdm_095, 0);
    reclaim_idx = 0;
    transfering_peb = -1;
    transfered_peb = -1;
    for (i = 0; i < g_nvdm_peb_count; i++) {
        peb_read_header(i, &peb_hdr);
        nvdm_log_msgid_info(nvdm_096, 0);
        peb_header_print_info(i, &peb_hdr);
        peb_status = peb_hdr.status;
        switch (peb_status) {
            case PEB_STATUS_ACTIVING:
                /* need erase and erase count is valid */
                peb_erase(i);
                peb_write_hdr_magic_number(i);
                peb_write_hdr_erase_count(i, ++(peb_hdr.erase_count));
                peb_write_hdr_version(i);
                g_pebs_info[i].erase_count = peb_hdr.erase_count;
                peb_update_status(i, PEB_STATUS_EMPTY);
                nvdm_port_poweroff(17);
                g_pebs_info[i].is_reserved = 1;
                break;
            case PEB_STATUS_TRANSFERING:
            case PEB_STATUS_TRANSFERED:
            case PEB_STATUS_RECLAIMING:
                ret = peb_header_is_validate(&peb_hdr, 0);
                if (ret == false) {
                    nvdm_log_msgid_error(nvdm_097, 1, i);
                    nvdm_port_free(reclaiming_peb);
                    return;
                }
                g_pebs_info[i].erase_count = peb_hdr.erase_count;
                /* we just mark those pebs, and deal with them after init complete. */
                if (peb_status == PEB_STATUS_TRANSFERING) {
                    if (transfering_peb >= 0) {
                        nvdm_log_msgid_error(nvdm_098, 2, transfering_peb, i);
                        nvdm_port_free(reclaiming_peb);
                        return;
                    }
                    transfering_peb = i;
                } else if (peb_status == PEB_STATUS_TRANSFERED) {
                    if (transfered_peb >= 0) {
                        nvdm_log_msgid_error(nvdm_099, 2, transfered_peb, i);
                        nvdm_port_free(reclaiming_peb);
                        return;
                    }
                    transfered_peb = i;
                } else {
                    /* there may be multiple reclaiming pebs exist */
                    reclaiming_peb[reclaim_idx++] = i;
                }
                break;
            case PEB_STATUS_ACTIVED:
                ret = peb_header_is_validate(&peb_hdr, 0);
                if (ret == false) {
                    nvdm_log_msgid_error(nvdm_100, 1, i);
                    nvdm_port_free(reclaiming_peb);
                    return;
                }
                g_pebs_info[i].erase_count = peb_hdr.erase_count;
                break;
            case PEB_STATUS_EMPTY:
                ret = peb_header_is_validate(&peb_hdr, 1);
                if (ret == false) {
                    nvdm_log_msgid_error(nvdm_101, 1, i);
                    nvdm_port_free(reclaiming_peb);
                    return;
                }
                g_pebs_info[i].erase_count = peb_hdr.erase_count;
                g_pebs_info[i].is_reserved = 1;
                break;
            case PEB_STATUS_VIRGIN:
            case PEB_STATUS_ERASING:
            default:
                /* need erase and erase count is invalid */
                peb_erase(i);
                peb_write_hdr_magic_number(i);
                peb_write_hdr_version(i);
                g_pebs_info[i].is_reserved = 1;
        }
        nvdm_log_msgid_info(nvdm_102, 0);
        peb_read_header(i, &peb_hdr);
        peb_header_print_info(i, &peb_hdr);
    }
    peb_print_info();
    nvdm_log_msgid_info(nvdm_103, 1, transfering_peb);
    nvdm_log_msgid_info(nvdm_104, 1, transfered_peb);
    for (i = 0; i < reclaim_idx; i++) {
        nvdm_log_msgid_info(nvdm_105, 2, i, reclaiming_peb[i]);
    }

    /* update erase count for unknown pebs */
    nvdm_log_msgid_info(nvdm_106, 0);
    g_mean_erase_count = 0;
    for (i = 0; i < g_nvdm_peb_count; i++) {
        if (g_pebs_info[i].erase_count != ERASE_COUNT_MAX) {
            g_mean_erase_count += g_pebs_info[i].erase_count;
        }
    }
    g_mean_erase_count /= g_nvdm_peb_count;
    for (i = 0; i < g_nvdm_peb_count; i++) {
        if (g_pebs_info[i].erase_count == ERASE_COUNT_MAX) {
            /* peb header need to update here if erase count is invalid */
            peb_write_hdr_erase_count(i, g_mean_erase_count);
            g_pebs_info[i].erase_count = g_mean_erase_count;
            peb_update_status(i, PEB_STATUS_EMPTY);
            nvdm_port_poweroff(18);
        }
    }
    peb_print_info();

    /* scan all non-reserved pebs including reclaiming pebs and transfering peb */
    nvdm_log_msgid_info(nvdm_107, 0);
    for (i = 0; i < g_nvdm_peb_count; i++) {
        /* skip transfering peb */
        if (i == transfering_peb) {
            continue;
        }
        /* skip reclaiming pebs and use reference from transfered peb if it exist */
        if (transfered_peb >= 0) {
            for (j = 0; j < reclaim_idx; j++) {
                if (i == reclaiming_peb[j]) {
                    break;
                }
            }
            if (j < reclaim_idx) {
                continue;
            }
        }
        if (g_pebs_info[i].is_reserved == 0) {
            /* reclaiming, transfered and active pebs can be scanned here */
            data_item_scan(i);
        } else {
            g_pebs_info[i].free = (g_nvdm_peb_size - PEB_HEADER_SIZE);
        }
    }

    /* deal with break from garbage collection */
    if ((reclaim_idx > 0) && (transfered_peb < 0)) {
        /* when power-off last time, data transfering is going on.
             * so we just restart garbage collection breaked by last power-off.
             */
        nvdm_log_msgid_info(nvdm_108, 0);
        if (transfering_peb >= 0) {
            peb_reclaim(transfering_peb);
        }
        relocate_pebs(reclaiming_peb, reclaim_idx);
    } else if (transfered_peb >= 0) {
        /* when power-off last time, data transfer is complete,
             * but source pebs maybe have not put back to wear-weaving yet
             */
        nvdm_log_msgid_info(nvdm_109, 0);
        for (i = 0; i < reclaim_idx; i++) {
            peb_reclaim(reclaiming_peb[i]);
        }
        peb_update_status(transfered_peb, PEB_STATUS_ACTIVED);
        nvdm_port_poweroff(19);
    } else {
        if ((reclaim_idx > 0) || (transfered_peb >= 0) || (transfering_peb >= 0)) {
            nvdm_log_msgid_error(nvdm_110, 3,
                           reclaim_idx, transfered_peb, transfering_peb);
            nvdm_port_free(reclaiming_peb);
            return;
        }
    }

    /* calculate total valid data size */
    nvdm_port_free(reclaiming_peb);
    nvdm_log_msgid_info(nvdm_111, 0);
    g_valid_data_size = 0;
    for (i = 0; i < g_nvdm_peb_count; i++) {
        g_valid_data_size += (g_nvdm_peb_size - PEB_HEADER_SIZE) - g_pebs_info[i].free - g_pebs_info[i].dirty;
    }
}

nvdm_status_t nvdm_init(void)
{
    if (nvdm_init_status == true) {
        return NVDM_STATUS_ERROR;
    }

    nvdm_port_poweroff_time_set();

    data_item_init();

    g_nvdm_peb_size = nvdm_port_get_peb_config(&g_nvdm_peb_count);
    if (g_nvdm_peb_count < 2) {
        nvdm_log_msgid_error(nvdm_112, 0);
        return NVDM_STATUS_ERROR;
    }

    g_total_avail_space = calculate_total_avail_space();

    g_pebs_info = nvdm_port_malloc(g_nvdm_peb_count * sizeof(peb_info_t));
    if (g_pebs_info == NULL) {
        nvdm_log_msgid_error(nvdm_113, 0);
        return NVDM_STATUS_ERROR;
    }
    memset(g_pebs_info, 0, g_nvdm_peb_count * sizeof(peb_info_t));

    peb_scan();

    peb_print_info();

    nvdm_port_mutex_creat();

    nvdm_init_status = true;

    nvdm_log_msgid_warning(nvdm_114, 0);

    return NVDM_STATUS_OK;
}

nvdm_status_t nvdm_query_space_info(uint32_t *total_avail_space, uint32_t *curr_used_space)
{
    if ((total_avail_space == NULL) || (curr_used_space == NULL)) {
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    if (nvdm_init_status == false) {
        return NVDM_STATUS_ERROR;
    }

    nvdm_port_mutex_take();

    *total_avail_space = g_total_avail_space;
    *curr_used_space = g_valid_data_size;

    nvdm_port_mutex_give();

    return NVDM_STATUS_OK;
}

#endif


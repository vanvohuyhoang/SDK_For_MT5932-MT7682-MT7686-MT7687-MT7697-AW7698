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
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "mesh_app_util.h"

#include "bt_mesh_ota_distributor.h"

/******************************************************************************/
/* prototype                                                                  */
/******************************************************************************/
#define NEW_FW_ADDR 0x0825c000
#define NEW_FW_ADDR2 0x082d9000

/******************************************************************************/
/* prototype                                                                  */
/******************************************************************************/
static bool _cb_is_new_firmware_exist(uint32_t);
static uint32_t _cb_firmware_get_blk_size(bool);
static void _cb_firmware_get_object_id(uint8_t obj_id[8]);
static uint32_t _cb_firmware_get_object_size(void);
uint8_t *_cb_firmware_get_block_data(uint8_t);


/******************************************************************************/
/* global variables                                                           */
/******************************************************************************/
const bt_mesh_ota_distributor_new_firmware_t new_firmware = {
    .firmware_is_exist = _cb_is_new_firmware_exist,
    .firmware_get_blk_size = _cb_firmware_get_blk_size,
    .firmware_get_object_id = _cb_firmware_get_object_id,
    .firmware_get_object_size = _cb_firmware_get_object_size,
    .firmware_get_block_data = _cb_firmware_get_block_data,
};
bool g_first_fw_addr = true;

/******************************************************************************/
/* static functions                                                           */
/******************************************************************************/
//#define GET_U32_VALUE(U8_PTR) ((*(PTR) & 0xff << 24) | (*(PTR+1) & 0xff << 16) | (*(PTR+2) & 0xff << 8) | (*(PTR+3) & 0xff))

/*
                          ------------ 
                             XX -->CM4
                          ------------
                             XX -->N9
base_addr_1611            ------------
                             XX
code_area1_start_addr_p      (XXXX)-->*code_area1_start_addr_p
                             XXXX
code_area1_start_addr        (XXXX)-->*code_area1_start_addr  = LEN1
                             XXXX
                             XXXX
code_area2_start_addr        (XXXX)-->*code_area2_start_addr  = LEN2
                             XXXX
                          ------------ 
                             XX
                             XX
                             XX
                             XX
                          ------------

*/
#define FLASH_OFFSET_1611 (0x200000)
static uint32_t* get_fw_addr(void)
{
    //for only area2 bin with checksum
    return (uint32_t*)((g_first_fw_addr)? NEW_FW_ADDR : NEW_FW_ADDR2);
    
    //for whole 1611(area1+area2) bin with checksum
    /*uint32_t base_addr_1611 = (uint32_t)((g_first_fw_addr)? NEW_FW_ADDR : NEW_FW_ADDR2);
    uint32_t* code_area1_start_addr_p = (uint32_t*)(base_addr_1611 + 0x25e7);
    uint32_t* code_area1_start_addr =  (uint32_t*)(base_addr_1611 + *(code_area1_start_addr_p) - FLASH_OFFSET_1611);
    uint32_t code_area1_length = *(code_area1_start_addr); 
    LOG_I(mesh_app, "get_fw_addr code_area1_start_addr_p 0x%x, code_area1_start_addr 0x%x, code_area1_length 0x%x\n", 
        code_area1_start_addr_p, code_area1_start_addr, code_area1_length);
    uint32_t* code_area2_start_addr = (uint32_t*)((uint8_t *)code_area1_start_addr + code_area1_length);
    uint32_t code_area2_length = *(code_area2_start_addr);
    uint32_t checksum = *(uint32_t *)((uint8_t *)code_area2_start_addr + code_area2_length);
    LOG_I(mesh_app, "get_fw_addr code_area2_start_addr 0x%x, code_area2_length 0x%x, checksum 0x%x\n", code_area2_start_addr, code_area2_length, checksum);
    
    return code_area2_start_addr;*/
}

//check if the firmware_id is exist, which is request by initiator
static bool _cb_is_new_firmware_exist(uint32_t firmware_id)
{
    //uint32_t area2_fw_ver = (get_fw_addr())[1];
    
    uint32_t area2_fw_ver = 0;
	if (firmware_id == ((uint32_t*)(NEW_FW_ADDR))[1]) {
		area2_fw_ver = firmware_id;
		g_first_fw_addr = true;
	} else if (firmware_id == ((uint32_t*)(NEW_FW_ADDR2))[1]) {
	    area2_fw_ver = firmware_id;
		g_first_fw_addr = false;
	}
    LOG_I(mesh_app, "_cb_is_new_firmware_exist fw_er 0x%x\n", area2_fw_ver);
    return area2_fw_ver == firmware_id;
}

//return block size
static uint32_t _cb_firmware_get_blk_size(bool islog)
{
    LOG_I(mesh_app, "_cb_firmware_get_blk_size islog %d\n", islog);
    if (islog == true) {
        return 12;
    }

    return 4096; //2^12 = 4096
}

//return a unique ID
static void _cb_firmware_get_object_id(uint8_t obj_id[8])
{
    /*we choose version + checksum to the unique id*/
    uint32_t *area2_addr = get_fw_addr();
    uint32_t area2_fw_ver = area2_addr[1];
    uint32_t area2_size = area2_addr[0];

    uint32_t checksum = *(uint32_t *)((uint32_t)area2_addr + area2_size);

    memcpy(obj_id, &checksum, 4);
    memcpy(obj_id + 4, &area2_fw_ver, 4);
    LOG_I(mesh_app, "_cb_firmware_get_object_id area2_fw_ver 0x%x, checksum 0x%x\n", area2_fw_ver, checksum);
}

//return firmware size
static uint32_t _cb_firmware_get_object_size(void)
{
    LOG_I(mesh_app, "_cb_firmware_get_object_size\n");
    return *(get_fw_addr()) + 4;
}

//return firmware data pointer
uint8_t *_cb_firmware_get_block_data(uint8_t blk_idx)
{
    LOG_I(mesh_app, "_cb_firmware_get_block_data blk_idx %d\n", blk_idx);
    uint8_t *dat = (uint8_t *)get_fw_addr();
    return dat + (blk_idx * 4096);
}




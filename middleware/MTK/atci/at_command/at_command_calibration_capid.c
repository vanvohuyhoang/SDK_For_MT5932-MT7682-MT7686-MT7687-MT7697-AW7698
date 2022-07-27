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

#ifdef MTK_BT_AT_COMMAND_ENABLE

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "at_command.h"
#include "hal_gpt.h"
#include "hal_log.h"
#include "hal_irrx_internal.h"
#define AT_DCXO_CAP_ID                  ((volatile uint32_t*)0xA2110080)
#define AT_EFUSE_CAPID                  ((volatile uint32_t*)0xA2060180)
#define AT_EFUSE_MAGIC                  ((volatile uint32_t*)0xA2060008)
#define AT_EFUSE_REFRESH                ((volatile uint32_t*)(0xA2060000))


#define AT_EFUSE_CAPID0_MASK            0x000001FF
#define AT_EFUSE_CAPID1_MASK            0x0003FE00
#define AT_EFUSE_CAPID2_MASK            0x07FC0000
#define AT_DCXO_DEFAULT_VAL             431
#define AT_DCXO_GOLDEN_VAL              442
#define AT_EFUSE_CAPIS_MASK             0x3FF
#define AT_EFUSE_CAPIS_SHIFT            2
#define AT_EFUSE_REFRESH_SET            0x4
#define AT_EFUSE_MAGIC_CODE             0xA07923B6

#if defined(MTK_CAPID_IN_NVDM_AT_COMMAND_ENABLE) && defined(MTK_NVDM_ENABLE)
#include "nvdm.h"
void set_capid(uint32_t target_capid);
uint32_t get_capid(void);
#endif /* MTK_CAPID_IN_NVDM_AT_COMMAND_ENABLE && MTK_NVDM_ENABLE */


extern bool bt_enable_relay_mode(hal_uart_port_t port);
uint32_t at_get_register_value(uint32_t address, short int mask, short int shift) ;

uint32_t read_current_cpaid();
uint32_t at_read_efuse_capid();

void at_write_efuse_capid0(uint32_t cal_capid);
void at_write_efuse_capid1(uint32_t cal_capid);
void at_write_efuse_capid2(uint32_t cal_capid);


void at_set_capid(uint32_t target_capid,uint32_t current_capid);


uint32_t at_current_capid = AT_DCXO_DEFAULT_VAL;

uint32_t at_get_register_value(uint32_t address, short int mask, short int shift) {
    uint32_t change_value, mask_buffer;
    //uint32_t _savedMask;
    mask_buffer = (mask << shift);
    change_value = *((volatile uint32_t *)(address));
    change_value &=mask_buffer;
    change_value = (change_value>> shift);
    return change_value;
}

uint32_t read_current_cpaid() {
    uint32_t capid;
    capid=(uint32_t)at_get_register_value((uint32_t)AT_DCXO_CAP_ID,AT_EFUSE_CAPIS_MASK,AT_EFUSE_CAPIS_SHIFT);
    return capid;
}
uint32_t at_read_efuse_capid()
{
    uint32_t efuse_capid  = *AT_EFUSE_CAPID;
    uint32_t efuse_capid0 = efuse_capid & AT_EFUSE_CAPID0_MASK;
    uint32_t efuse_capid1 = efuse_capid & AT_EFUSE_CAPID1_MASK;
    uint32_t efuse_capid2 = efuse_capid & AT_EFUSE_CAPID2_MASK;
    efuse_capid1 = efuse_capid1>>9;
    efuse_capid2 = efuse_capid2>>18;
    if(efuse_capid2!=0) {
        log_hal_msgid_info(" read EFUSE CAPID2 %ld \n\r", 1, efuse_capid2);
        return efuse_capid2;
    } else if(efuse_capid1!=0) {
        log_hal_msgid_info("  read EFUSE CAPID1 %ld \n\r", 1, efuse_capid1);
        return efuse_capid1;
    } else if(efuse_capid0!=0) {
        log_hal_msgid_info("read EFUSE CAPID0 %ld \n\r", 1, efuse_capid0);
        return efuse_capid0;
    } else {
        log_hal_msgid_info("  EFUSE CAPID not exists , read golden value\n\r", 0);
        return AT_DCXO_GOLDEN_VAL;
    }
}
void at_write_efuse_capid0(uint32_t cal_capid)
{
    uint32_t efuse_capid  = *AT_EFUSE_CAPID;
    uint32_t efuse_capid0 = efuse_capid & AT_EFUSE_CAPID0_MASK;
    uint32_t efuse_capid1 = efuse_capid & AT_EFUSE_CAPID1_MASK;
    uint32_t efuse_capid2 = efuse_capid & AT_EFUSE_CAPID2_MASK;
    uint32_t temp = 0;
    int32_t  efuse_check = 0;

    (void)temp;
    (void)efuse_check;

    efuse_capid1 = efuse_capid1>>9;
    efuse_capid2 = efuse_capid2>>18;
    //log_hal_msgid_info( " EFUSE CAPID = %lu \n\r",efuse_capid);
    log_hal_msgid_info(" EFUSE CAPID = %lu \n\r", 1, efuse_capid);
    log_hal_msgid_info(" EFUSE CAPID0 = %lu \n\r", 1, efuse_capid0);
    log_hal_msgid_info(" EFUSE CAPID1 = %lu \n\r", 1, efuse_capid1);
    log_hal_msgid_info(" EFUSE CAPID2 = %lu \n\r", 1, efuse_capid2);
    log_hal_msgid_info(" target cal_capid = %lu \n\r", 1, cal_capid);


    if( efuse_capid0 == 0 ) {
        //log_hal_msgid_info(" write EFUSE CAPID0 \n\r");
        log_hal_msgid_info(" write EFUSE CAPID0 \n\r", 0);
        *AT_EFUSE_MAGIC = AT_EFUSE_MAGIC_CODE;
        temp = *AT_EFUSE_MAGIC;
        //log_hal_msgid_info( " AT_EFUSE_MAGIC = %lu \n\r",temp);
        log_hal_msgid_info(" AT_EFUSE_MAGIC = %lu \n\r", 1, temp);
        *AT_EFUSE_CAPID = (cal_capid & AT_EFUSE_CAPID0_MASK);
        temp = *AT_EFUSE_CAPID;
        //log_hal_msgid_info( " AT_EFUSE_CAPID = %lu \n\r",temp);
        log_hal_msgid_info(" AT_EFUSE_CAPID = %lu \n\r", 1, temp);
        *AT_EFUSE_REFRESH = AT_EFUSE_REFRESH_SET;
        log_hal_msgid_info(" CAPID Calibration DONE!!!! EFUSE CAPID0 write (calibration result = %ld )\n\r", 1, cal_capid);
        //log_hal_msgid_info( " CAPID Calibration DONE!!!! EFUSE CAPID0 write (calibration result = %ld )\n\r",cal_capid);
    } else {
        log_hal_msgid_info( " CAPID Calibrated!!!!! \n\r", 0);
        if(efuse_capid0 != cal_capid) {
            efuse_check = cal_capid - efuse_capid0;
            if( efuse_check>5||efuse_check<(-5) ) {
                log_hal_msgid_info( " WARNING!!!! EFUSE CAPID0 is not match calibration result ( efuse_capid0 = %ld , calibration result = %ld )\n\r", 2, efuse_capid0,cal_capid);
            }
        }
        log_hal_msgid_info( "efuse_capid0 = %ld \n\r", 1, efuse_capid0);
    }

}

void at_write_efuse_capid1(uint32_t cal_capid)
{
    uint32_t efuse_capid  = *AT_EFUSE_CAPID;
    uint32_t efuse_capid0 = efuse_capid & AT_EFUSE_CAPID0_MASK;
    uint32_t efuse_capid1 = efuse_capid & AT_EFUSE_CAPID1_MASK;
    uint32_t efuse_capid2 = efuse_capid & AT_EFUSE_CAPID2_MASK;
    uint32_t temp = 0;
    int32_t  efuse_check = 0;

    (void)temp;
    (void)efuse_check;

    efuse_capid1 = efuse_capid1>>9;
    efuse_capid2 = efuse_capid2>>18;
    log_hal_msgid_info( " EFUSE CAPID = %lu \n\r", 1, efuse_capid);
    log_hal_msgid_info( " EFUSE CAPID0 = %lu \n\r", 1, efuse_capid0);
    log_hal_msgid_info( " EFUSE CAPID1 = %lu \n\r", 1, efuse_capid1);
    log_hal_msgid_info( " EFUSE CAPID2 = %lu \n\r", 1, efuse_capid2);
    log_hal_msgid_info( " target cal_capid = %lu \n\r", 1, cal_capid);


    if( efuse_capid1 == 0 ) {
        log_hal_msgid_info( " write EFUSE CAPID1 \n\r", 0);
        *AT_EFUSE_MAGIC = AT_EFUSE_MAGIC_CODE;
        temp = *AT_EFUSE_MAGIC;
        log_hal_msgid_info( " AT_EFUSE_MAGIC = %lu \n\r", 1, temp);
        *AT_EFUSE_CAPID = ( (cal_capid<<9) & AT_EFUSE_CAPID1_MASK);
        temp = *AT_EFUSE_CAPID;
        log_hal_msgid_info( " AT_EFUSE_CAPID = %lu \n\r", 1, temp);
        *AT_EFUSE_REFRESH = AT_EFUSE_REFRESH_SET;
        log_hal_msgid_info( " CAPID Calibration DONE!!!! EFUSE CAPID1 write (calibration result = %ld )\n\r", 1, cal_capid);
    } else {
        log_hal_msgid_info( " CAPID Calibrated!!!!! \n\r", 0);
        if(efuse_capid1 != cal_capid) {
            efuse_check = cal_capid - efuse_capid1;
            if( efuse_check>5||efuse_check<(-5) ) {
                log_hal_msgid_info( " WARNING!!!! EFUSE CAPID1 is not match calibration result ( efuse_capid1 = %ld , calibration result = %ld )\n\r", 2, efuse_capid1,cal_capid);
            }
        }
        log_hal_msgid_info( "efuse_capid1 = %ld \n\r", 1, efuse_capid1);
    }

}

void at_write_efuse_capid2(uint32_t cal_capid)
{
    uint32_t efuse_capid  = *AT_EFUSE_CAPID;
    uint32_t efuse_capid0 = efuse_capid & AT_EFUSE_CAPID0_MASK;
    uint32_t efuse_capid1 = efuse_capid & AT_EFUSE_CAPID1_MASK;
    uint32_t efuse_capid2 = efuse_capid & AT_EFUSE_CAPID2_MASK;
    uint32_t temp = 0;
    int32_t  efuse_check = 0;

    (void)temp;
    (void)efuse_check;

    efuse_capid1 = efuse_capid1>>9;
    efuse_capid2 = efuse_capid2>>18;
    log_hal_msgid_info( " EFUSE CAPID = %lu \n\r", 1, efuse_capid);
    log_hal_msgid_info( " EFUSE CAPID0 = %lu \n\r", 1, efuse_capid0);
    log_hal_msgid_info( " EFUSE CAPID1 = %lu \n\r", 1, efuse_capid1);
    log_hal_msgid_info( " EFUSE CAPID2 = %lu \n\r", 1, efuse_capid2);
    log_hal_msgid_info( " target cal_capid = %lu \n\r", 1, cal_capid);


    if( efuse_capid2 == 0 ) {
        log_hal_msgid_info( " write EFUSE CAPID2 \n\r", 0);
        *AT_EFUSE_MAGIC = AT_EFUSE_MAGIC_CODE;
        temp = *AT_EFUSE_MAGIC;
        log_hal_msgid_info( " AT_EFUSE_MAGIC = %lu \n\r", 1, temp);
        *AT_EFUSE_CAPID = ( (cal_capid<<18) & AT_EFUSE_CAPID2_MASK);
        temp = *AT_EFUSE_CAPID;
        log_hal_msgid_info( " AT_EFUSE_CAPID = %lu \n\r", 1, temp);
        *AT_EFUSE_REFRESH = AT_EFUSE_REFRESH_SET;
        log_hal_msgid_info( " CAPID Calibration DONE!!!! EFUSE CAPID2 write (calibration result = %ld )\n\r", 1, cal_capid);
    } else {
        log_hal_msgid_info( " CAPID Calibrated!!!!! \n\r", 0);
        if(efuse_capid2 != cal_capid) {
            efuse_check = cal_capid - efuse_capid2;
            if( efuse_check>5||efuse_check<(-5) ) {
                log_hal_msgid_info( " WARNING!!!! EFUSE CAPID2 is not match calibration result ( efuse_capid2 = %ld , calibration result = %ld )\n\r", 2, efuse_capid2,cal_capid);
            }
        }
        log_hal_msgid_info( "efuse_capid2 = %ld \n\r", 1, efuse_capid2);
    }

}


void at_set_capid(uint32_t target_capid,uint32_t current_capid)
{
    uint32_t dcxo_cap_id     = *AT_DCXO_CAP_ID;
    int32_t  duration        = target_capid - current_capid;
    int32_t temp_capid      = current_capid;
    log_hal_msgid_info(" at_set_capid = %lx \n\r", 1, current_capid);
    if(duration>0) {
        /* increase CAPID */
        while (duration>0) {
            /* calculate temp capid */
            duration -= 4;
            temp_capid +=4;
            if(temp_capid>target_capid) {
                temp_capid = target_capid;
            }

            /* wait 4 us */
            hal_gpt_delay_us(4);
            /* set new val */
            dcxo_cap_id = *AT_DCXO_CAP_ID & 0xFFFFF803;
            *AT_DCXO_CAP_ID =  ((temp_capid<<2) | dcxo_cap_id);
            /* dump for debug */
            dcxo_cap_id = *AT_DCXO_CAP_ID;
#ifdef BL_DCXO_CAL_DEBUG
            log_hal_msgid_info(" dump BL_DCXO_CAP_ID = %u \n\r", 1, dcxo_cap_id);
#endif
        }
    } else {
        /* decrease CAPID */
        while (duration<0) {
            /* calculate temp capid */
            duration += 4;
            temp_capid -=4;
            if(temp_capid<target_capid) {
                temp_capid = target_capid;
            }

            if( temp_capid<0 ) {
                temp_capid = (int32_t)target_capid;
            }
            /* wait 4 us */
            hal_gpt_delay_us(4);
            /* set new val */
            dcxo_cap_id = *AT_DCXO_CAP_ID & 0xFFFFF803;
            *AT_DCXO_CAP_ID =  ((temp_capid<<2) | dcxo_cap_id);
            /* dump for debug */
            dcxo_cap_id = *AT_DCXO_CAP_ID;
#ifdef BL_DCXO_CAL_DEBUG
            log_hal_msgid_info(" dump temp_capid = %d , BL_DCXO_CAP_ID = %u \n\r", 2, temp_capid,dcxo_cap_id);
#endif
        }
    }
    /* dump final config */
    dcxo_cap_id = *AT_DCXO_CAP_ID;
    /* record current capid */
    at_current_capid = target_capid;
}

atci_status_t atci_cmd_hdlr_capid(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t resonse = {{0}};

    char *param1 = NULL;
    char *param2 = NULL;
    uint8_t things=0;
    uint8_t port=0;
    int32_t result=0;
    uint32_t debug_capid=0;
    //uint32_t read_efuse_capid=0;

    log_hal_msgid_info("atci_cmd_hdlr_capid \r\n", 0);

    resonse.response_flag = 0; /*    Command Execute Finish.  */

    switch (parse_cmd->mode) {
    case ATCI_CMD_MODE_TESTING:
        strncpy((char *)resonse.response_buf, "+CAPID=99\r\nOK\r\n",(strlen((char *)resonse.response_buf)+1));
        atci_send_response(&resonse);
        break;

    case ATCI_CMD_MODE_EXECUTION:
        strncpy((char *)resonse.response_buf, "OK\r\n",(strlen((char *)resonse.response_buf)+1));
        resonse.response_len = strlen((char *)resonse.response_buf);
        atci_send_response(&resonse);

        if (strncmp(parse_cmd->string_ptr, "AT+CAPID=", strlen("AT+CAPID=")) == 0) {
            param1 = (char *)strtok(parse_cmd->string_ptr, "=");
            param1 = (char *)strtok(NULL, ",");
            things = atoi(param1);
            log_hal_msgid_info("things=%d\r\n", 1, things);

            if(0 == things) { //AT+CAPID=0,2   enter relay mode on uart2
                param2=(char *)strtok(NULL, ",");
                port = atoi(param2);
                log_hal_msgid_info("things=%d,port=%d\r\n", 2, things,port);
                result = bt_enable_relay_mode((hal_uart_port_t)port);
                if(true == result) {
                    resonse.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                } else {
                    resonse.response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                }
            } else if(1 == things) { //AT+CAPID=1,420   modify capid=420 dynamic.
                param2=(char *)strtok(NULL, ",");
                debug_capid = atoi(param2);
#if defined(MTK_CAPID_IN_NVDM_AT_COMMAND_ENABLE) && defined(MTK_NVDM_ENABLE)
                set_capid(debug_capid);
                hal_gpt_delay_ms(1); /* waiting for signal stable */
                log_hal_msgid_info("Set %u to CAPID register.\r\n", 1, (int)debug_capid);
#else
                log_hal_msgid_info("things=%d,debug_capid=%ld\r\n", 2, things,debug_capid);
                at_set_capid(debug_capid ,read_current_cpaid());
#endif /* MTK_CAPID_IN_NVDM_AT_COMMAND_ENABLE && MTK_NVDM_ENABLE */
                resonse.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else if(2 == things) { //AT+CAPID=2,404,2  save capid to efuse CAPID2
                param2=(char *)strtok(NULL, ",");
                debug_capid = atoi(param2);
#if defined(MTK_CAPID_IN_NVDM_AT_COMMAND_ENABLE) && defined(MTK_NVDM_ENABLE)
                nvdm_status_t status;
                status = nvdm_write_data_item("DCXO", "CAPID Value", NVDM_DATA_ITEM_TYPE_RAW_DATA, (uint8_t *)&debug_capid, sizeof(debug_capid));
                if (status != NVDM_STATUS_OK) {
                    log_hal_msgid_info("DCXO CAPID write to NVDM failed !!\r\n", 0);
                } else{
                    log_hal_msgid_info("CAPID %u write to NVDM succeed\r\n", 1, (int)debug_capid);
                }
#else
                char *param3=(char *)strtok(NULL, ",");
                uint8_t capid_position = atoi(param3);
                log_hal_msgid_info("things=%d,debug_capid=%ld,capid_position=%d\r\n", 3, things,debug_capid,capid_position);
                if(0 == capid_position) {
                    at_write_efuse_capid0(debug_capid);
                } else if(1 == capid_position) {
                    at_write_efuse_capid1(debug_capid);
                } else if(2 == capid_position) {
                    at_write_efuse_capid2(debug_capid);
                }
#endif /* MTK_CAPID_IN_NVDM_AT_COMMAND_ENABLE && MTK_NVDM_ENABLE */
                resonse.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

            } else if(3 == things) { //AT+CAPID=3  read efuse capid
#if defined(MTK_CAPID_IN_NVDM_AT_COMMAND_ENABLE) && defined(MTK_NVDM_ENABLE)
                nvdm_status_t status;
                uint32_t size = sizeof(result);
                status = nvdm_read_data_item("DCXO", "CAPID Value", (uint8_t *)&result, &size);
                if (status != NVDM_STATUS_OK) {
                    log_hal_msgid_info("DCXO CAPID load from NVDM failed !!\r\n", 0);
                } else{
                    log_hal_msgid_info("CAPID load from NVDM = %u\r\n", 1, (int)result);
                }
#else
                result  = *AT_EFUSE_CAPID;
                log_hal_msgid_info("things=%d,efuse capid=%ld(%lx)\r\n", 3, things,result,result);
#endif /* MTK_CAPID_IN_NVDM_AT_COMMAND_ENABLE && MTK_NVDM_ENABLE */
                resonse.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else if(4 == things) { //AT+CAPID=4  read current capid
#if defined(MTK_CAPID_IN_NVDM_AT_COMMAND_ENABLE) && defined(MTK_NVDM_ENABLE)
                result = get_capid();
                log_hal_msgid_info("CAPID register = %d\r\n", 1, (int)result);
#else
                result  = read_current_cpaid();
                log_hal_msgid_info("things=%d,current capid=%ld(%lx)\r\n", 3, things,result,result);
#endif /* MTK_CAPID_IN_NVDM_AT_COMMAND_ENABLE && MTK_NVDM_ENABLE */
                resonse.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            }else if(5 == things) { //AT+CAPID=5  read current capid
#if defined(MTK_CAPID_IN_NVDM_AT_COMMAND_ENABLE) && defined(MTK_NVDM_ENABLE)
            log_hal_msgid_info("Support at_command to calibration capid\r\n", 0);
#else
            log_hal_msgid_info("Not support at_command to calibration capid\r\n", 0);
#endif /* MTK_CAPID_IN_NVDM_AT_COMMAND_ENABLE && MTK_NVDM_ENABLE */
                resonse.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            }

        }

        sprintf((char *)resonse.response_buf, "+CAPID:%d,%ld\r\n",things,result);
        resonse.response_len = strlen((char *)resonse.response_buf);
        atci_send_response(&resonse);


        break;
    default :
        /* others are invalid command format */
        strncpy((char *)resonse.response_buf, "ERROR\r\n",(strlen((char *)resonse.response_buf)+1));
        resonse.response_len = strlen((char *)resonse.response_buf);
        atci_send_response(&resonse);
        break;
    }
    return ATCI_STATUS_OK;
}

#endif
/*---ATA Command handler Function End---*/


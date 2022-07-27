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
#include "hal.h"
#include "hal_ccni_config.h"
#include "hal_ccni_internal.h"
#include "hal_resource_assignment.h"

#include "ccci.h"
#include "ccci_configure.h"
#include "ccci_log.h"
#include "assert.h"
#include "hal_core_status.h"
#include "hal_gpt.h"
typedef struct {
    ccci_event_t event;
    p_ccci_receive_callback ccci_callback;
} ccci_user_t;

//#define CCCI_DEBUG
log_create_module(ccci,PRINT_LEVEL_INFO);


#if defined CORE_DSP0

#ifndef DSP0_RECEIVE_CCCI_FROM_DSP1_USER_ARRAY
#define DSP0_RECEIVE_CCCI_FROM_DSP1_USER_ARRAY
#endif

#ifndef DSP0_RECEIVE_CCCI_FROM_CM4_USER_ARRAY
#define DSP0_RECEIVE_CCCI_FROM_CM4_USER_ARRAY
#endif

#ifndef DSP0_RECEIVE_CCCI_FROM_N9_USER_ARRAY
#define DSP0_RECEIVE_CCCI_FROM_N9_USER_ARRAY
#endif

static const ccci_user_t ccci_user_array_dsp0_receive_from_dsp1[]= {
                                              DSP0_RECEIVE_CCCI_FROM_DSP1_USER_ARRAY
                                            };
static const ccci_user_t ccci_user_array_dsp0_receive_from_cm4[]= {
                                              DSP0_RECEIVE_CCCI_FROM_CM4_USER_ARRAY
                                            };
static const ccci_user_t ccci_user_array_dsp0_receive_from_n9[]= {
                                              DSP0_RECEIVE_CCCI_FROM_N9_USER_ARRAY
                                            };
static const ccci_user_t* ccci_user_array_for_receive[] = {ccci_user_array_dsp0_receive_from_cm4,NULL,ccci_user_array_dsp0_receive_from_dsp1,ccci_user_array_dsp0_receive_from_n9};
static const uint32_t send_ccci_event_array[] = {CCNI_DSP0_TO_CM4_CCCI,0,CCNI_DSP0_TO_DSP1_CCCI,CCNI_DSP0_TO_N9_CCCI};
static const uint32_t ccci_send_event_start_array[] = {CCCI_EVENT_DSP0_TO_CM4_start,0,CCCI_EVENT_DSP0_TO_DSP1_start,CCCI_EVENT_DSP0_TO_N9_start};
static const uint32_t ccci_send_event_max_array[] = {CCCI_EVENT_DSP0_TO_CM4_end,0,CCCI_EVENT_DSP0_TO_DSP1_end,CCCI_EVENT_DSP0_TO_N9_end};
static const uint32_t ccci_receive_event_start_array[] = {CCCI_EVENT_CM4_TO_DSP0_start,0,CCCI_EVENT_DSP1_TO_DSP0_start,CCCI_EVENT_N9_TO_DSP0_start};
static const uint32_t ccci_receive_event_end_array[] = {CCCI_EVENT_CM4_TO_DSP0_end,0,CCCI_EVENT_DSP1_TO_DSP0_end,CCCI_EVENT_N9_TO_DSP0_end};
//for CCCI ACK
static const hal_ccni_event_t ccci_ack_array_for_send[] = {CCNI_DSP0_TO_CM4_CCCI_ACK,0,CCNI_DSP0_TO_DSP1_CCCI_ACK,0};
static const hal_ccni_event_t ccci_ack_array_for_receive[] = {CCNI_CM4_TO_DSP0_CCCI_ACK,0,CCNI_DSP1_TO_DSP0_CCCI_ACK,0};
#elif defined CORE_DSP1

#ifndef DSP1_RECEIVE_CCCI_FROM_DSP0_USER_ARRAY
#define DSP1_RECEIVE_CCCI_FROM_DSP0_USER_ARRAY
#endif

#ifndef DSP1_RECEIVE_CCCI_FROM_CM4_USER_ARRAY
#define DSP1_RECEIVE_CCCI_FROM_CM4_USER_ARRAY
#endif

#ifndef DSP1_RECEIVE_CCCI_FROM_N9_USER_ARRAY
#define DSP1_RECEIVE_CCCI_FROM_N9_USER_ARRAY
#endif

static const ccci_user_t ccci_user_array_dsp1_receive_from_dsp0[]= {
                                              DSP1_RECEIVE_CCCI_FROM_DSP0_USER_ARRAY
                                            };
static const ccci_user_t ccci_user_array_dsp1_receive_from_cm4[]= {
                                              DSP1_RECEIVE_CCCI_FROM_CM4_USER_ARRAY
                                            };
static const ccci_user_t ccci_user_array_dsp1_receive_from_n9[]= {
                                              DSP1_RECEIVE_CCCI_FROM_N9_USER_ARRAY
                                            };
static const ccci_user_t* ccci_user_array_for_receive[] = {ccci_user_array_dsp1_receive_from_cm4,ccci_user_array_dsp1_receive_from_dsp0,NULL,ccci_user_array_dsp1_receive_from_n9};
static const uint32_t send_ccci_event_array[] = {CCNI_DSP1_TO_CM4_CCCI,CCNI_DSP1_TO_DSP0_CCCI,0,CCNI_DSP1_TO_N9_CCCI};
static const uint32_t ccci_send_event_start_array[] = {CCCI_EVENT_DSP1_TO_CM4_start,CCCI_EVENT_DSP1_TO_DSP0_start,0,CCCI_EVENT_DSP1_TO_N9_start};
static const uint32_t ccci_send_event_max_array[] = {CCCI_EVENT_DSP1_TO_CM4_end,CCCI_EVENT_DSP1_TO_DSP0_end,0,CCCI_EVENT_DSP1_TO_N9_end};
static const uint32_t ccci_receive_event_start_array[] = {CCCI_EVENT_CM4_TO_DSP1_start,CCCI_EVENT_DSP0_TO_DSP1_start,0,CCCI_EVENT_N9_TO_DSP1_start};
static const uint32_t ccci_receive_event_end_array[] = {CCCI_EVENT_CM4_TO_DSP1_end,CCCI_EVENT_DSP0_TO_DSP1_end,0,CCCI_EVENT_N9_TO_DSP1_end};
//for CCCI ACK
static const hal_ccni_event_t ccci_ack_array_for_send[] = {CCNI_DSP1_TO_CM4_CCCI_ACK,CCNI_DSP1_TO_DSP0_CCCI_ACK,0,0};
static const hal_ccni_event_t ccci_ack_array_for_receive[] = {CCNI_CM4_TO_DSP1_CCCI_ACK,CCNI_DSP0_TO_DSP1_CCCI_ACK,0,0};
#elif defined CORE_CM4

#ifndef CM4_RECEIVE_CCCI_FROM_DSP0_USER_ARRAY
#define CM4_RECEIVE_CCCI_FROM_DSP0_USER_ARRAY
#endif

#ifndef CM4_RECEIVE_CCCI_FROM_DSP1_USER_ARRAY
#define CM4_RECEIVE_CCCI_FROM_DSP1_USER_ARRAY
#endif

#ifndef CM4_RECEIVE_CCCI_FROM_N9_USER_ARRAY
#define CM4_RECEIVE_CCCI_FROM_N9_USER_ARRAY
#endif

static const ccci_user_t ccci_user_array_cm4_receive_from_dsp0[]= {
                                              CM4_RECEIVE_CCCI_FROM_DSP0_USER_ARRAY
                                            };
static const ccci_user_t ccci_user_array_cm4_receive_from_dsp1[]= {
                                              CM4_RECEIVE_CCCI_FROM_DSP1_USER_ARRAY
                                            };
static const ccci_user_t ccci_user_array_cm4_receive_from_n9[]= {
                                              CM4_RECEIVE_CCCI_FROM_N9_USER_ARRAY
                                            };
static const ccci_user_t* ccci_user_array_for_receive[] = {NULL,ccci_user_array_cm4_receive_from_dsp0,ccci_user_array_cm4_receive_from_dsp1,ccci_user_array_cm4_receive_from_n9};
static const uint32_t send_ccci_event_array[] = {0,CCNI_CM4_TO_DSP0_CCCI,CCNI_CM4_TO_DSP1_CCCI,CCNI_CM4_TO_N9_CCCI};
static const uint32_t ccci_send_event_start_array[] = {0,CCCI_EVENT_CM4_TO_DSP0_start,CCCI_EVENT_CM4_TO_DSP1_start,CCCI_EVENT_CM4_TO_N9_start};
static const uint32_t ccci_send_event_max_array[] = {0,CCCI_EVENT_CM4_TO_DSP0_end,CCCI_EVENT_CM4_TO_DSP1_end,CCCI_EVENT_CM4_TO_N9_end};
static const uint32_t ccci_receive_event_start_array[] = {0,CCCI_EVENT_DSP0_TO_CM4_start,CCCI_EVENT_DSP1_TO_CM4_start,CCCI_EVENT_N9_TO_CM4_start};
static const uint32_t ccci_receive_event_end_array[] = {0,CCCI_EVENT_DSP0_TO_CM4_end,CCCI_EVENT_DSP1_TO_CM4_end,CCCI_EVENT_N9_TO_CM4_end};
//for CCCI ACK
static const hal_ccni_event_t ccci_ack_array_for_send[] = {0,CCNI_CM4_TO_DSP0_CCCI_ACK,CCNI_CM4_TO_DSP1_CCCI_ACK,0};
static const hal_ccni_event_t ccci_ack_array_for_receive[] = {0,CCNI_DSP0_TO_CM4_CCCI_ACK,CCNI_DSP1_TO_CM4_CCCI_ACK,0};
#else
    #error
#endif


#if defined CORE_DSP0
const uint32_t ccci_receive_data2_address[4] = {
        CCNI_MSG_BASE + CCNI_CM4_TO_N9_MSG_SIZE + (6*2 + 1)*4,    //DSP0 to CM4 channle 6 word_2,   DSP0 receive from CM4 will use this word to receive
		0,
        CCNI_MSG_BASE + CCNI_DSP0_TO_CM4_MSG_SIZE + (6*2 + 1)*4,  //DSP0 to DSP1 channle 6 word_2,  DSP0 receive from DSP1 will use this word to receive
        CCNI_MSG_BASE + CCNI_DSP0_TO_DSP1_MSG_SIZE + (6*2 + 1)*4  //DSP0 to N9 channle 6 word_2,    DSP0 receive from N9 will use this word to receive
};
const uint32_t ccci_send_data2_address[4] = {
        CCNI_MSG_BASE + (6*2 + 1)*4,                               //CM4 to DSP0 channle 6 word_2,  DSP0 send to CM4 will use this word to send
        0,
        CCNI_MSG_BASE + CCNI_DSP1_TO_CM4_MSG_SIZE + (6*2 + 1)*4,   //DSP1 to DSP0 channle 6 word_2, DSP0 send to DSP1 will use this word to send
        CCNI_MSG_BASE + CCNI_N9_TO_CM4_MSG_SIZE + (6*2 + 1)*4      //N9 to DSP0 channle 6 word_2,   DSP0 send to N9 will use this word to send
};
#elif defined CORE_DSP1
const uint32_t ccci_receive_data2_address[4] = {
        CCNI_MSG_BASE + CCNI_DSP0_TO_N9_MSG_SIZE + (6*2 + 1)*4,   //DSP1 to CM4 channle 6 word_2,   DSP1 receive from CM4 will use this word to receive
        CCNI_MSG_BASE + CCNI_DSP1_TO_CM4_MSG_SIZE + (6*2 + 1)*4,  //DSP1 to DSP0 channle 6 word_2,  DSP1 receive from DSP0 will use this word to receive
        0,
        CCNI_MSG_BASE + CCNI_DSP1_TO_DSP0_MSG_SIZE + (6*2 + 1)*4  //DSP1 to N9 channle 6 word_2,    DSP1 receive from DSP1 will use this word to receive
};
const uint32_t ccci_send_data2_address[4] = {
        CCNI_MSG_BASE + CCNI_CM4_TO_DSP0_MSG_SIZE + (6*2 + 1)*4,  //CM4 to DSP1 channle 6 word_2,   DSP1 send to CM4 will use this word to send
        CCNI_MSG_BASE + CCNI_DSP0_TO_CM4_MSG_SIZE + (6*2 + 1)*4,  //DSP0 to DSP1 channle 6 word_2,  DSP1 send to DSP0 will use this word to send
        0,
        CCNI_MSG_BASE + CCNI_N9_TO_DSP0_MSG_SIZE + (6*2 + 1)*4    //N9 to DSP1 channle 6 word_2,    DSP1 send to DSP1 will use this word to send
};
#elif defined CORE_CM4
const uint32_t ccci_receive_data2_address[4] = {
        0,
		CCNI_MSG_BASE + (6*2 + 1)*4,                              //CM4 to DSP0 channle 6 word_2,   DSP0 receive from CM4 will use this word to receive
        CCNI_MSG_BASE + CCNI_CM4_TO_DSP0_MSG_SIZE + (6*2 + 1)*4,  //CM4 to DSP1 channle 6 word_2,   DSP1 receive from CM4 will use this word to receive
        CCNI_MSG_BASE + CCNI_CM4_TO_DSP1_MSG_SIZE + (6*2 + 1)*4   //CM4 to N9 channle 6 word_2,     N9 receive from CM4 will use this word to receive
};
const uint32_t ccci_send_data2_address[4] = {
        0,
        CCNI_MSG_BASE + CCNI_CM4_TO_N9_MSG_SIZE + (6*2 + 1)*4,    //DSP0 to CM4 channle 6 word_2,   CM4 send to DSP0 will use this word to send
        CCNI_MSG_BASE + CCNI_DSP0_TO_N9_MSG_SIZE + (6*2 + 1)*4,   //DSP1 to CM4 channle 6 word_2,   CM4 send to DSP1 will use this word to send
        CCNI_MSG_BASE + CCNI_DSP1_TO_N9_MSG_SIZE + (6*2 + 1)*4    //N9 to CM4 channle 6 word_2,     CM4 send to N9 will use this word to send
};
#endif

//void ccci_receive_msg(hal_ccni_event_t event, void * msg)
#ifdef CORE_DSP0
void ccci_dsp0_receive_msg_from_others_core(hal_ccni_event_t event, void * msg)
#elif defined CORE_DSP1
void ccci_dsp1_receive_msg_from_others_core(hal_ccni_event_t event, void * msg)
#elif defined CORE_CM4
void ccci_cm4_receive_msg_from_others_core(hal_ccni_event_t event, void * msg)
#else
    #error
#endif
{
    ccci_msg_t ccci_msg;
    uint32_t *pMsg = msg;
    uint32_t index,src_core;
#ifdef CORE_DSP0
    //printf("[enter]This is DSP0 CCCI receive handle, event:0x%x",event);
#elif defined CORE_DSP1
    //printf("[enter]This is DSP1 CCCI receive handle, event:0x%x",event);
#elif defined CORE_CM4
    //printf("[enter]This is CM4 CCCI receive handle, event:0x%x",event);
#else
    #error
#endif
    ccci_msg.event = (ccci_event_t)pMsg[0];
    ccci_msg.data = (uint32_t)pMsg[1];
    index = (ccci_msg.event & 0xffff) - 1;
    src_core = (ccci_msg.event>>CCNI_EVENT_SRC_OFFSET) - 1;
    //ccci_msg.data2 = (uint32_t)pMsg[2];
    ccci_msg.data2 = *(uint32_t*)ccci_receive_data2_address[src_core];
#ifdef CCCI_DEBUG
    CCCI_LOG_I(ccci,"receive data2 addr:0x%x   src_core:0x%d",2,ccci_receive_data2_address[src_core],src_core);
#endif

    CCCI_LOG_I(ccci,"receive CCNI event 0x%x - CCCI event:0x%x, data1:0x%x, data2:0x%x",4,event,ccci_msg.event,ccci_msg.data,ccci_msg.data2);

    if ((ccci_msg.event <= ccci_receive_event_start_array[src_core]) || (ccci_msg.event >= ccci_receive_event_end_array[src_core])) {
#ifdef CCCI_DEBUG
        CCCI_LOG_E(ccci,"Error: Want to receive event:0x%x,out of range!!! start ID:0x%x, end ID:0x%x",3,ccci_msg.event,ccci_receive_event_start_array[src_core],ccci_receive_event_end_array[src_core]);
#endif
        assert(0);
        return;
    }
    if (ccci_user_array_for_receive[src_core] == NULL) {
        assert(0);
        return;
    }
    if ((ccci_user_array_for_receive[src_core] + index)->ccci_callback == NULL) {
#ifdef CCCI_DEBUG
        CCCI_LOG_E(ccci,"index:%d, callback as NULL!!!",1,index);
#endif
        assert(0);
        return;
    }

    (ccci_user_array_for_receive[src_core] + index)->ccci_callback(ccci_msg);
#ifdef CCCI_DEBUG
    CCCI_LOG_D(ccci,"clear CCNI event of 0x%x",1,event);
#endif
    hal_ccni_clear_event(event);
    hal_ccni_unmask_event(event); // unmask the event.


// for CCCI ack
    CCCI_LOG_I(ccci,"Send CCCI ACK to core of %d , event is :0x%x",2,src_core,ccci_ack_array_for_send[src_core]);
    //send back of the CCCI event ,msg is the event which received
    if (hal_ccni_set_event(ccci_ack_array_for_send[src_core],msg) == HAL_CCNI_STATUS_BUSY) {
#ifdef CCCI_DEBUG
        CCCI_LOG_E(ccci,"CCCI ACK-CCNI status error, should not busy!!!",0);
#endif
        assert(0);
    }
#ifdef CORE_DSP0
    //printf("[exit]This is DSP0 CCCI receive handle, event:0x%x",event);
#elif defined CORE_DSP1
    //printf("[exit]This is DSP1 CCCI receive handle, event:0x%x",event);
#elif defined CORE_CM4
    //printf("[exit]This is CM4 CCCI receive handle, event:0x%x",event);
#else
    #error
#endif
}

ccci_status_t ccci_send_event_check(hal_core_id_t dst_core_id,ccci_msg_t* p_ccci_msg)
{
    hal_core_id_t dest_core;
    dest_core = ((p_ccci_msg->event & CCNI_DST_MASK)>>CCNI_EVENT_DST_OFFSET) -1;
    if (dst_core_id != dest_core) {
         CCCI_LOG_E(ccci,"Error!!! Core not match!!!! Can't send to self, dst_core_id:0x%x,CCCI event:0x%x,dest_core:0x%x",3,dst_core_id,p_ccci_msg->event,dest_core);
        assert(0);
         return CCCI_STATUS_PARAMETER_ERROR;
    }

    if ((p_ccci_msg->event <= ccci_send_event_start_array[dest_core]) || (p_ccci_msg->event >= ccci_send_event_max_array[dest_core])) {
#ifdef CCCI_DEBUG
        CCCI_LOG_E(ccci,"Error: Want to send event:0x%x,out of range!!! start ID:0x%x, end ID:0x%x",3,
                        p_ccci_msg->event,ccci_send_event_start_array[dest_core],ccci_send_event_max_array[dest_core]);
#endif
        assert(0);
        return CCCI_STATUS_PARAMETER_ERROR;
    }
    return CCCI_STATUS_OK;
}


#define CCCI_SW_FIFO_SIZE  10

#define CCCI_SW_FIFO_NOTE_STATUS_OF_IDLE   0
#define CCCI_SW_FIFO_NOTE_STATUS_OF_PUSH   1
#define CCCI_SW_FIFO_NOTE_STATUS_OF_POP    2
#define CCCI_SW_FIFO_NOTE_STATUS_OF_RECEIVE_DONE   3

typedef struct {
    ccci_msg_t ccci_sw_fifo[CCCI_SW_FIFO_SIZE];
    volatile uint8_t ccci_sw_fifo_note_status[CCCI_SW_FIFO_SIZE];//CCCI_SW_FIFO_NOTE_STATUS_OF_IDLE/CCCI_SW_FIFO_NOTE_STATUS_OF_FILLED/CCCI_SW_FIFO_NOTE_STATUS_OF_RECEIVE_DONE
    volatile uint8_t ccci_sw_fifo_read_point;
    volatile uint8_t ccci_sw_fifo_write_point;
    volatile uint8_t ccci_sw_fifo_data_counter;
} ccci_sw_fifo_note_t;

#ifdef CORE_CM4
    ccci_sw_fifo_note_t cm4_to_dsp0_ccci_sw_fifo_note;
    ccci_sw_fifo_note_t cm4_to_dsp1_ccci_sw_fifo_note;
    ccci_sw_fifo_note_t* ccci_sw_fifo_table[] = {NULL,&cm4_to_dsp0_ccci_sw_fifo_note,&cm4_to_dsp1_ccci_sw_fifo_note,NULL};
#elif defined CORE_DSP0
    ccci_sw_fifo_note_t dsp0_to_cm4_ccci_sw_fifo_note;
    ccci_sw_fifo_note_t dsp0_to_dsp1_ccci_sw_fifo_note;
    ccci_sw_fifo_note_t* ccci_sw_fifo_table[] = {&dsp0_to_cm4_ccci_sw_fifo_note,NULL,&dsp0_to_dsp1_ccci_sw_fifo_note,NULL};
#elif defined CORE_DSP1
    ccci_sw_fifo_note_t dsp1_to_cm4_ccci_sw_fifo_note;
    ccci_sw_fifo_note_t dsp1_to_dsp0_ccci_sw_fifo_note;
    ccci_sw_fifo_note_t* ccci_sw_fifo_table[] = {&dsp1_to_cm4_ccci_sw_fifo_note,&dsp1_to_dsp0_ccci_sw_fifo_note,NULL,NULL};
#else
    #error "N9 CCCI no implement!!!"
#endif

uint8_t push_to_ccci_fifo(ccci_sw_fifo_note_t* p_ccci_sw_fifo,ccci_msg_t *p)
{
    uint8_t current_write_point;
    p_ccci_sw_fifo->ccci_sw_fifo_data_counter++;
    p_ccci_sw_fifo->ccci_sw_fifo[p_ccci_sw_fifo->ccci_sw_fifo_write_point].event = p->event;
    p_ccci_sw_fifo->ccci_sw_fifo[p_ccci_sw_fifo->ccci_sw_fifo_write_point].data = p->data;
    p_ccci_sw_fifo->ccci_sw_fifo[p_ccci_sw_fifo->ccci_sw_fifo_write_point].data2 = p->data2;
    p_ccci_sw_fifo->ccci_sw_fifo_note_status[p_ccci_sw_fifo->ccci_sw_fifo_write_point] = CCCI_SW_FIFO_NOTE_STATUS_OF_PUSH;
    current_write_point = p_ccci_sw_fifo->ccci_sw_fifo_write_point;
    p_ccci_sw_fifo->ccci_sw_fifo_write_point++;
    p_ccci_sw_fifo->ccci_sw_fifo_write_point %= CCCI_SW_FIFO_SIZE;
    return current_write_point;
}
uint8_t pop_from_ccci_fifo(ccci_sw_fifo_note_t* p_ccci_sw_fifo,ccci_msg_t *p)
{
    p->event=p_ccci_sw_fifo->ccci_sw_fifo[p_ccci_sw_fifo->ccci_sw_fifo_read_point].event;
    p->data = p_ccci_sw_fifo->ccci_sw_fifo[p_ccci_sw_fifo->ccci_sw_fifo_read_point].data;
    p->data2 = p_ccci_sw_fifo->ccci_sw_fifo[p_ccci_sw_fifo->ccci_sw_fifo_read_point].data2;
    p_ccci_sw_fifo->ccci_sw_fifo_note_status[p_ccci_sw_fifo->ccci_sw_fifo_read_point] = CCCI_SW_FIFO_NOTE_STATUS_OF_POP;
    //should not update read point to here!!!, Should update it on CCCI ACK handle
    //Actually, this just peek SW FIFO
    //p_ccci_sw_fifo->ccci_sw_fifo_data_counter--;
    //p_ccci_sw_fifo->ccci_sw_fifo_read_point++;
    //p_ccci_sw_fifo->ccci_sw_fifo_read_point %= CCCI_SW_FIFO_SIZE;
    //printf("pop: p_ccci_sw_fifo->ccci_sw_fifo_data_counter:%d,p_ccci_sw_fifo->ccci_sw_fifo_read_point:%d",p_ccci_sw_fifo->ccci_sw_fifo_data_counter,p_ccci_sw_fifo->ccci_sw_fifo_read_point);
    //printf("pop: status:%d",p_ccci_sw_fifo->ccci_sw_fifo_note_status[p_ccci_sw_fifo->ccci_sw_fifo_read_point]);
    return p_ccci_sw_fifo->ccci_sw_fifo_read_point;
}

/**** ccci_wait_arrary[]
    index as CCCI_SEND_MSG_NO_WAIT,no need wait
    index as CCCI_SEND_MSG_WAIT_FOR_SEND_RECEIVE_DONE, need wait for status of CCCI_SW_FIFO_NOTE_STATUS_OF_RECEIVE_DONE
****/
static const  uint32_t ccci_wait_arrary[]= {CCCI_SW_FIFO_NOTE_STATUS_OF_IDLE,CCCI_SW_FIFO_NOTE_STATUS_OF_RECEIVE_DONE};
void wait_for_one_ccci_event_send_or_receive_done(ccci_sw_fifo_note_t* p_ccci_sw_fifo,ccci_send_wait_type_t wait_type,uint8_t current_index)
{
    volatile uint8_t *p_ack_status;
    p_ack_status = &p_ccci_sw_fifo->ccci_sw_fifo_note_status[current_index];
    while(1) {
        if (*p_ack_status >= ccci_wait_arrary[wait_type]) {
            break;
        }
        CCCI_LOG_I(ccci,"wait type:%d,Wait for status of %d. current_index:%d,Now event status is %d",4,wait_type,ccci_wait_arrary[wait_type],current_index,*p_ack_status);
        hal_gpt_delay_ms(10);
    }
}

ccci_status_t ccci_send_msg(hal_core_id_t dst_core_id,ccci_msg_t ccci_msg,ccci_send_wait_type_t wait_type)
{
    hal_ccni_status_t status;
    hal_ccni_message_t info;
    uint32_t mask;
    ccci_sw_fifo_note_t* p_ccci_sw_fifo;
    uint8_t current_push;
    uint8_t current_pop;
    if (CCCI_STATUS_OK != ccci_send_event_check(dst_core_id,&ccci_msg))
        return CCCI_STATUS_PARAMETER_ERROR;

    p_ccci_sw_fifo = ccci_sw_fifo_table[dst_core_id];
    CCCI_LOG_I(ccci,"CCCI want to send event of 0x%x, wait type:%d",2,ccci_msg.event,wait_type);
    hal_nvic_save_and_set_interrupt_mask(&mask);
    //step1: Whether SW FIFO is full?
    if (p_ccci_sw_fifo->ccci_sw_fifo_data_counter >= CCCI_SW_FIFO_SIZE) {
        if (wait_type == CCCI_SEND_MSG_WAIT_FOR_SEND_RECEIVE_DONE) {
            while(1) {
                hal_nvic_restore_interrupt_mask(mask);
                CCCI_LOG_W(ccci,"step1:CCCI SW FIFO full!!! now delya 1ms wait for SW FIFO have some one space!!!,type if :%d",1,wait_type);
                hal_gpt_delay_ms(1);
                hal_nvic_save_and_set_interrupt_mask(&mask);
                if(p_ccci_sw_fifo->ccci_sw_fifo_data_counter < CCCI_SW_FIFO_SIZE) {
                    break;
                }
            }
        } else {
            hal_nvic_restore_interrupt_mask(mask);
            CCCI_LOG_W(ccci,"step1:CCCI SW FIFO full!!! ccci_sw_fifo_data_counter:%d,ccci_sw_fifo_write_point:%d,ccci_sw_fifo_read_point:%d",
                    3,p_ccci_sw_fifo->ccci_sw_fifo_data_counter,p_ccci_sw_fifo->ccci_sw_fifo_write_point,p_ccci_sw_fifo->ccci_sw_fifo_read_point);
            return CCCI_STATUS_SW_FIFO_FULL;
        }
    }
    //step2: Push this msg to SW FIFO
    current_push = push_to_ccci_fifo(p_ccci_sw_fifo,&ccci_msg);
#ifdef CCCI_DEBUG
    CCCI_LOG_I(ccci,"step2:Send CCNI event 0x%x - CCCI event:0x%x, data1:0x%x,data2:0x%x,type:%x, push to SW FIFO, current_push:%d,total counter:%d",7,
            send_ccci_event_array[dst_core_id],ccci_msg.event,ccci_msg.data,ccci_msg.data2,wait_type,current_push,p_ccci_sw_fifo->ccci_sw_fifo_data_counter);
#endif
    //step3: Whether CCCI SW FIFO is running?  if ccci_sw_fifo_data_counter != 0, means SW FIFO is running, no need send CCNI event to here!!!Should send one by one on CCCI ack callback
    if(p_ccci_sw_fifo->ccci_sw_fifo_data_counter > 1) {
        hal_nvic_restore_interrupt_mask(mask);
        if (wait_type != CCCI_SEND_MSG_NO_WAIT) {
            wait_for_one_ccci_event_send_or_receive_done(p_ccci_sw_fifo,wait_type,current_push);
        }  
        return CCCI_STATUS_OK;
    }
    //step4: pop one item
    current_pop = pop_from_ccci_fifo(p_ccci_sw_fifo,&ccci_msg);
    info.ccni_message[0] = ccci_msg.event;
    info.ccni_message[1] = ccci_msg.data;
    //Put the data2 in replied CCNI event memory as replied CCNI event don't use message memory.
    *(uint32_t*)ccci_send_data2_address[dst_core_id] = ccci_msg.data2;
#ifdef CCCI_DEBUG
     CCCI_LOG_I(ccci," send data2 addr:0x%x  dst_core_id:%d",2,ccci_send_data2_address[dst_core_id],dst_core_id);
#endif
    status = hal_ccni_set_event(send_ccci_event_array[dst_core_id],&info);
    hal_nvic_restore_interrupt_mask(mask);
#ifdef CCCI_DEBUG
    CCCI_LOG_I(ccci,"step4: On User task to POP CCCI event--Send CCNI event 0x%x - CCCI event:0x%x, data1:0x%x,data2:0x%x, current_push:%d,current_pop:%d",6,
    send_ccci_event_array[dst_core_id],ccci_msg.event,ccci_msg.data,ccci_msg.data2,current_push,current_pop);
#endif
    
    if (status == HAL_CCNI_STATUS_BUSY) {
#ifdef CCCI_DEBUG
        CCCI_LOG_E(ccci,"step4: CCCI-CCNI status error, should not busy!!!",0);
#endif
        assert(0);
        return CCCI_STATUS_FAIL;
    }

    //step5: if the type is not NO_WAIT, then wait for send done or receive done!!!
    wait_for_one_ccci_event_send_or_receive_done(p_ccci_sw_fifo,wait_type,current_push);
    return CCCI_STATUS_OK;
}


#ifdef CORE_DSP0
void ccci_dsp0_receive_msg_from_others_core_for_ccci_ack(hal_ccni_event_t event, void * msg)
#elif defined CORE_DSP1
void ccci_dsp1_receive_msg_from_others_core_for_ccci_ack(hal_ccni_event_t event, void * msg)
#elif defined CORE_CM4
void ccci_cm4_receive_msg_from_others_core_for_ccci_ack(hal_ccni_event_t event, void * msg)
#else
    #error
#endif
{
    uint32_t src_core;
    ccci_sw_fifo_note_t* p_ccci_sw_fifo;
    ccci_msg_t ccci_msg;
#ifdef CCCI_DEBUG
    ccci_msg_t ccci_msg_ack;
     uint32_t *pMsg = msg;
#endif
    uint32_t mask;
    hal_ccni_message_t info;
    uint8_t current_pop;
    hal_ccni_status_t status;
#ifdef CORE_DSP0
    //printf("[enter]This is DSP0 CCCI ACK receive handle, event:0x%x",event);
#elif defined CORE_DSP1
    //printf("[enter]This is DSP1 CCCI ACK receive handle, event:0x%x",event);
#elif defined CORE_CM4
    //printf("[enter]This is CM4 CCCI ACK receive handle, event:0x%x",event);
#else
    #error
#endif

    CCCI_LOG_I(ccci,"Receive CCCI ACK event:0x%x",1,event);
    (void) msg;
	
    //step1: get the current read point
    src_core = (event>>CCNI_EVENT_SRC_OFFSET) - 1;
    if(ccci_ack_array_for_receive[src_core] != event)
        assert(0);
    p_ccci_sw_fifo = ccci_sw_fifo_table[src_core];

#ifdef CCCI_DEBUG
    //step2: Check the ack event is right? For Debug
    ccci_msg_ack.event = (ccci_event_t)pMsg[0];
    //note: CCCI ACK just use the first word of CCNI data, the second data for CCCI data2

    if(p_ccci_sw_fifo->ccci_sw_fifo[p_ccci_sw_fifo->ccci_sw_fifo_read_point].event != ccci_msg_ack.event) {
        CCCI_LOG_E(ccci,"CCCI event of 0x%x ACK error!!! send event is:0x%x, receive ACK event is: 0x%x",2,p_ccci_sw_fifo->ccci_sw_fifo[p_ccci_sw_fifo->ccci_sw_fifo_read_point].event,ccci_msg_ack.event);
        assert(0);
    } else {
        CCCI_LOG_I(ccci,"CCCI event of 0x%x ACK done!!!",1,ccci_msg_ack.event);
    }
#endif

    hal_nvic_save_and_set_interrupt_mask(&mask);
    //step3: change the note statue is CCCI_SW_FIFO_NOTE_STATUS_OF_RECEIVE_DONE
    p_ccci_sw_fifo->ccci_sw_fifo_note_status[p_ccci_sw_fifo->ccci_sw_fifo_read_point] = CCCI_SW_FIFO_NOTE_STATUS_OF_RECEIVE_DONE;
    //printf("CCCI event of 0x%x ACK done!!! status 0x%x:%d",ccci_msg_ack.event,&p_ccci_sw_fifo->ccci_sw_fifo_note_status[p_ccci_sw_fifo->ccci_sw_fifo_read_point],p_ccci_sw_fifo->ccci_sw_fifo_note_status[p_ccci_sw_fifo->ccci_sw_fifo_read_point]);

    //update read point when this note transfer done!!!
    //Actually, this just real pop one item
    p_ccci_sw_fifo->ccci_sw_fifo_data_counter--;
    p_ccci_sw_fifo->ccci_sw_fifo_read_point++;
    p_ccci_sw_fifo->ccci_sw_fifo_read_point %= CCCI_SW_FIFO_SIZE;

    if(p_ccci_sw_fifo->ccci_sw_fifo_data_counter > 0) {
        current_pop = pop_from_ccci_fifo(p_ccci_sw_fifo,&ccci_msg);

        info.ccni_message[0] = ccci_msg.event;
        info.ccni_message[1] = ccci_msg.data;
        //info.ccni_message[2] = ccci_msg.data2;
        *(uint32_t*)ccci_send_data2_address[src_core] = ccci_msg.data2;
#ifdef CCCI_DEBUG
        CCCI_LOG_I(ccci,"send data2 addr:0x%x,src_core:%d",2,ccci_send_data2_address[src_core],src_core);
#endif
        status = hal_ccni_set_event(send_ccci_event_array[src_core],&info);
        hal_nvic_restore_interrupt_mask(mask);
#ifdef CCCI_DEBUG
        CCCI_LOG_I(ccci,"On CCCI ACK IRQ handle to POP CCCI event--Send CCNI event 0x%x - CCCI event:0x%x, data1:0x%x,data2:0x%x,current_pop:%d",5,
            send_ccci_event_array[src_core],ccci_msg.event,ccci_msg.data,ccci_msg.data2,current_pop);
#endif
        if(status == HAL_CCNI_STATUS_BUSY) {
#ifdef CCCI_DEBUG
            CCCI_LOG_I(ccci,"CCCI-CCNI status error, should not busy!!!",0);
#endif
            assert(0);
            return;
        }
    } else {
        hal_nvic_restore_interrupt_mask(mask);
#ifdef CCCI_DEBUG
        CCCI_LOG_I(ccci,"On CCCI ACK IRQ handle, all event had been done,not need pop!!!",0);
#endif
    }
#ifdef CCCI_DEBUG
    CCCI_LOG_I(ccci,"clear CCCI ACK event of 0x%x",1,event);
#endif
    hal_ccni_clear_event(event);
    hal_ccni_unmask_event(event); // unmask the event.

#ifdef CORE_DSP0
    //printf("[exit]This is DSP0 CCCI ACK receive handle, event:0x%x",event);
#elif defined CORE_DSP1
    //printf("[exit]This is DSP1 CCCI ACK receive handle, event:0x%x",event);
#elif defined CORE_CM4
    //printf("[exit]This is CM4 CCCI ACK receive handle, event:0x%x",event);
#else
    #error
#endif

}


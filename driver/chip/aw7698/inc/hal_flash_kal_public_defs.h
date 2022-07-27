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
 
#ifndef _KAL_PUBLIC_DEFS_H
#define _KAL_PUBLIC_DEFS_H

#if 0
/*******************************************************************************
 * Type Definitions
 *******************************************************************************/

/* opaque type for task identity */
typedef struct {
    uint8_t unused;
} *kal_taskid;

/* opaque type for hisr identity */
typedef struct {
    uint8_t unused;
} *kal_hisrid;

/* opaque type for semaphore identity */
typedef struct {
    uint8_t unused;
} *kal_semid;

/* opaque type for message queue identity */
typedef struct {
    uint8_t unused;
} *kal_msgqid;

/* contain message queue information */
typedef struct {
    /* current number of pending messages in message queue */
    uint32_t pending_msgs;
    /* maximum number of pending messages in message queue */
    uint32_t max_msgs;
} kal_msgq_info;

/* opaque type for mutex identity */
typedef struct {
    uint8_t unused;
} *kal_mutexid;

/* opaque type for event group identity */
typedef struct {
    uint8_t unused;
} *kal_eventgrpid;

/* opaque type for partition memory pool identity */
typedef struct {
    uint8_t unused;
} *kal_poolid;

/* opaque type for Application Dynamic Memory identity */
typedef struct {
    uint8_t unused;
} *KAL_ADM_ID;

/* opaque type for Application Fix Memory identity */
typedef struct {
    uint8_t unused;
} *KAL_AFM_ID;

/* opaque type for First Level Memory Manager identity */
typedef struct {
    uint8_t unused;
} *KAL_FLMM_ID;

/* opaque type for Second Level Memory Manager identity */
typedef struct {
    uint8_t unused;
} *KAL_SLMM_ID;

/* opaque type for enhance mutex identity */
typedef struct {
    uint8_t unused;
} *kal_enhmutexid;

/* opaque type for kal timer identity */
typedef struct {
    uint8_t unused;
} *kal_timerid;
#endif

#endif  /* _KAL_PUBLIC_DEFS_H */

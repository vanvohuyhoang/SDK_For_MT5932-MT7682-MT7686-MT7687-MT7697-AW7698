;/* Copyright Statement:
; *
; * (C) 2019  Airoha Technology Corp. All rights reserved.
; *
; * This software/firmware and related documentation ("Airoha Software") are
; * protected under relevant copyright laws. The information contained herein
; * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
; * Without the prior written permission of Airoha and/or its licensors,
; * any reproduction, modification, use or disclosure of Airoha Software,
; * and information contained herein, in whole or in part, shall be strictly prohibited.
; * You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
; * if you have agreed to and been bound by the applicable license agreement with
; * Airoha ("License Agreement") and been granted explicit permission to do so within
; * the License Agreement ("Permitted User").If you are not a Permitted User,
; * please cease any access or use of Airoha Software immediately.
; * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
; * THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
; * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
; * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
; * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
; * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
; * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
; * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
; * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
; * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
; * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
; * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
; * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
; * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
; * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
; * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
; * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
; */

; Macro defines
#define Cache_Ctrl_Base     0xE0180000
#define Cache_Disable       0x0

        MODULE  ?cstartup

        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:DATA:NOROOT(2)

        EXTERN  __iar_program_start
        EXPORT  JumpCmd
        PUBLIC  __vector_table

__iar_init$$done:               ; The vector table is not needed
                                ; until after copy initialization is done

        DATA
__vector_table

        DCD     SFE(CSTACK)                ; Top of Stack
        DCD     Reset_Handler              ; Reset Handler

        THUMB
        PUBWEAK Reset_Handler
        SECTION .reset_handler:CODE:REORDER:NOROOT(2)

Reset_Handler
        ;set stack pointer
        LDR     SP, =SFE(CSTACK)

        ;interrupt disable
        CPSID   I

        ;cache disable, make region init safer
        MOVS    R0, #Cache_Disable
        LDR     R2, =Cache_Ctrl_Base
        STR     R0, [R2], #4

        ;stack space zero init
        MOVS    R0, #0
        LDR     R1, =SFB(CSTACK)
        LDR     R2, =SFE(CSTACK)
FillZero
        STR     R0, [R1], #4
        CMP     R1, R2
        BCC     FillZero

        LDR     R0, =__iar_program_start
        BLX     R0


JumpCmd
        ORR     R0,R0, #0x01
        BX      R0

        END
;************************ (C) COPYRIGHT MEDIATEK *****END OF FILE*****

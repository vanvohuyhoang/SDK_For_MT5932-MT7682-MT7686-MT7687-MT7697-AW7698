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
 
COMBO_MEM_TYPE_MODIFIER COMBO_MEM_TYPE_NAME COMBO_MEM_INST_NAME = {
    COMBO_MEM_STRUCT_HEAD
    {
        // MTKSIP_2523_SF_32_01
        CMEM_TYPE_SERIAL_NOR_FLASH,
        3,  // Valid ID length
        {0xC2, 0x25, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00}  // Flash ID  MX25U3233FWJI
    },
    {
        // MTKSIP_2523_SF_32_02
        CMEM_TYPE_SERIAL_NOR_FLASH,
        3,  // Valid ID length
        {0xC2, 0x28, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00}  // Flash ID  MX25R3235FWJIH1
    },
    {
        // MTKSIP_2523_SF_32_03
        CMEM_TYPE_SERIAL_NOR_FLASH,
        3,  // Valid ID length
        {0xC2, 0x25, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00}  // Flash ID  MX25U8035F
    },
    {
        // MTKSIP_2523_SF_32_04
        CMEM_TYPE_SERIAL_NOR_FLASH,
        3,  // Valid ID length
        {0xEF, 0x60, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00}  // Flash ID  W25Q80EW
    },
    {
        // MTKSIP_2523_SF_32_05
        CMEM_TYPE_SERIAL_NOR_FLASH,
        3,  // Valid ID length
        {0xEF, 0x60, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00}  // Flash ID W25Q32FW
    }

    COMBO_MEM_STRUCT_FOOT
};

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
 
#ifndef __CUSTOM_SFI__
#define __CUSTOM_SFI__

/*
 ****************************************************************************
 Specify the chip select configuration
 Possible choices, NOR_FLASH, RAM, LPSDRAM, UNUSED
 ****************************************************************************
*/

/*
 ****************************************************************************
 Specify RAM size in Bytes
 ****************************************************************************
*/


/*
 ****************************************************************************
 Define the driving and cache/prefetch setting (optional)
 ****************************************************************************
*/

#define __SFI_CLK_78MHZ__

#define __SFI_DCM_COMBO_INDEX_UND   0xffffffff

typedef enum {
    SFC_MAC_CTL            = 0x0
    , SFC_DR_CTL             = 0x1
    , SFC_MISC_CTL           = 0x2
    , SFC_MISC_CTL2          = 0x3
    , SFC_DLY_CTL2           = 0x4
    , SFC_DLY_CTL3           = 0x5
    , SFC_Driving            = 0x6
    , SFC_Reserved           = 0x7
    , SFC_DLY_CTL4           = 0x8
    , SFC_DLY_CTL5           = 0x9
} SF_SFC_Control_Reg_Enum;

#if 0
static const CMEMEntrySFIList_dcm combo_mem_hw_list_dcm = {   // (to be renamed by SFI owner)

    //COMBO_SFI_VER,           // SFI structure version
    //COMBO_MEM_ENTRY_COUNT,   // defined in custom_Memorydevice.h
    {

        {
            // MTKSIP_2523_SF_32_01
            {
                // HW config 78Mhz Start
                0x52F80000,  // SFI_MISC_CTL
                0x0,  // SFI_DLY_CTL_2
                0x1B00001B,  // SFI_DLY_CTL_3
                0,  // SFI_DLY_CTL_4
                0,  // SFI_DLY_CTL_5
            },  // HW config END
            {
                // HW config 26Mhz Start
                0x52F80000,  // SFI_MISC_CTL
                0x0,  // SFI_DLY_CTL_2
                0x0,  // SFI_DLY_CTL_3
            }  // HW config END
        },
        {
            // MTKSIP_2523_SF_32_02
            {
                // HW config 78Mhz Start
                0x52F80000,  // SFI_MISC_CTL
                0x0,  // SFI_DLY_CTL_2
                0x1B00001B,  // SFI_DLY_CTL_3
                0,  // SFI_DLY_CTL_4
                0,  // SFI_DLY_CTL_5
            },  // HW config END
            {
                // HW config 26Mhz Start
                0x52F80000,  // SFI_MISC_CTL
                0x0,  // SFI_DLY_CTL_2
                0x0,  // SFI_DLY_CTL_3
            }  // HW config END
        },
        {
            // MTKSIP_2523_SF_32_03
            {
                // HW config 78Mhz Start
                0x52F80010,  // SFI_MISC_CTL
                0x0,  // SFI_DLY_CTL_2
                0x15000015,  // SFI_DLY_CTL_3
                0,  // SFI_DLY_CTL_4
                0,  // SFI_DLY_CTL_5
            },  // HW config END
            {
                // HW config 26Mhz Start
                0x32F80000,  // SFI_MISC_CTL
                0x0,  // SFI_DLY_CTL_2
                0x0,  // SFI_DLY_CTL_3
            }  // HW config END
        },
        {
            // MTKSIP_2523_SF_32_04
            {
                // HW config 78Mhz Start
                0x52F80310,  // SFI_MISC_CTL
                0x0,  // SFI_DLY_CTL_2
                0x17000017,  // SFI_DLY_CTL_3
                0,  // SFI_DLY_CTL_4
                0,  // SFI_DLY_CTL_5
            },  // HW config END
            {
                // HW config 26Mhz Start
                0x52F80310,  // SFI_MISC_CTL
                0x0,  // SFI_DLY_CTL_2
                0x0,  // SFI_DLY_CTL_3
            }  // HW config END
        },
        {
            // MTKSIP_2523_SF_32_05
            {
                // HW config 78Mhz Start
                0x52F80010,  // SFI_MISC_CTL
                0x0,  // SFI_DLY_CTL_2
                0x17000017,  // SFI_DLY_CTL_3
                0,  // SFI_DLY_CTL_4
                0,  // SFI_DLY_CTL_5
            },  // HW config END
            {
                // HW config 26Mhz Start
                0x52F80000,  // SFI_MISC_CTL
                0x0,  // SFI_DLY_CTL_2
                0x0,  // SFI_DLY_CTL_3
            }  // HW config END
        }

    }
};
#endif

/*
 ****************************************************************************
 Specify the related EMI Setting
 ****************************************************************************
*/

/*
 ****************************************************************************
 Specify additional information
 ****************************************************************************
*/

#endif /* __CUSTOM_SFI__ */


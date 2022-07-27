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


#ifndef __NVKEY_ID_LIST_H__
#define __NVKEY_ID_LIST_H__

#if (PRODUCT_VERSION == 1552)
// The valid NVKEY range is from 0x0001 to 0xFFFF.

typedef enum
{
    NVKEYID_BT_HOST_LINK_KEY_RECORD_ID_01              = 0x1800,
    NVKEYID_BT_HOST_LINK_KEY_RECORD_ID_02              = 0x1801,
    NVKEYID_BT_HOST_LINK_KEY_RECORD_ID_03              = 0x1802,
    NVKEYID_BT_HOST_LINK_KEY_RECORD_ID_04              = 0x1803,
    NVKEYID_BT_HOST_LINK_KEY_RECORD_ID_05              = 0x1804,
    NVKEYID_BT_HOST_LINK_KEY_RECORD_ID_06              = 0x1805,
    NVKEYID_BT_HOST_LINK_KEY_RECORD_ID_07              = 0x1806,
    NVKEYID_BT_HOST_LINK_KEY_RECORD_ID_08              = 0x1807,
    NVKEYID_BT_HOST_LINK_KEY_RECORD_ID_09              = 0x1808,
    NVKEYID_BT_HOST_LINK_KEY_RECORD_ID_10              = 0x1809,
    NVKEYID_BT_HOST_LINK_KEY_RECORD_ID_11              = 0x180A,
    NVKEYID_BT_HOST_LINK_KEY_RECORD_ID_12              = 0x180B,
    NVKEYID_BT_HOST_LINK_KEY_RECORD_ID_13              = 0x180C,
    NVKEYID_BT_HOST_LINK_KEY_RECORD_ID_14              = 0x180D,
    NVKEYID_BT_HOST_LINK_KEY_RECORD_ID_15              = 0x180E,
    NVKEYID_BT_HOST_LINK_KEY_RECORD_ID_16              = 0x180F,
    NVKEYID_BT_HOST_PACKET_TYPE                        = 0x1810,

    NVKEYID_NVKEY_READONLY_SECTOR_NUMBER               = 0x2200,
    NVKEYID_MP_CAL_CHG_CONFIG                          = 0x2000,
    NVKEYID_MP_CAL_CHG_ADC_CONFIG                      = 0x2001,
    NVKEYID_MP_CAL_INT_CHG_DAC_CONFIG                  = 0x2002,
    NVKEYID_MP_CAL_EXT_CHG_DAC_CONFIG                  = 0x2003,
    NVKEYID_MP_CAL_INT_CHG_TRICKLE_CURRENT_CONFIG      = 0x2004,
    NVKEYID_MP_CAL_EXT_CHG_TRICKLE_CURRENT_CONFIG      = 0x2005,
    NVKEYID_MP_CAL_INT_CHG_CC1_CURRENT_CONFIG          = 0x2006,
    NVKEYID_MP_CAL_EXT_CHG_CC1_CURRENT_CONFIG          = 0x2007,
    NVKEYID_MP_CAL_INT_CHG_CC2_CURRENT_CONFIG          = 0x2008,
    NVKEYID_MP_CAL_EXT_CHG_CC2_CURRENT_CONFIG          = 0x2009,
    NVKEYID_MP_CAL_BUCK_MV_CONFIG                      = 0x2010,
    NVKEYID_MP_CAL_LDO_LV2_CONFIG                      = 0x2011,
    NVKEYID_MP_CAL_BUCK_LV_CONFIG                      = 0x2012,
    NVKEYID_MP_CAL_LDO_LV1_CONFIG                      = 0x2013,
    NVKEYID_MP_CAL_LDO_HV_CONFIG                       = 0x2014,
    NVKEYID_MP_CAL_HV_MIC_CONFIG                       = 0x2015,
    NVKEYID_MP_CAL_VBATADC_CALIBRATIONABLE             = 0x2020,
    NVKEYID_MP_CAL_VINAD_CALIBRATIONABLE               = 0x2021,
    NVKEYID_MP_CAL_VBAT_VOLTAGE_CONFIG                 = 0x2022,
    NVKEYID_MP_CAL_AIO1_CALIBRATIONABLE                = 0x2030,
    NVKEYID_MP_CAL_AIO2_CALIBRATIONABLE                = 0x2031,
    NVKEYID_MP_CAL_AIO3_CALIBRATIONABLE                = 0x2032,
    NVKEYID_MP_CAL_AIO4_CALIBRATIONABLE                = 0x2033,

    NVKEYID_MP_CAL_PWR_CTL_MP_K                        = 0x2040,
    NVKEYID_MP_CAL_TEMP_COMPENSATION_MP_K              = 0x2041,

    NVKEYID_MP_CAL_XO_26M_CRTSTAL_TRIM                 = 0x2045,
    NVKEYID_MP_CAL_XO_32K_CRTSTAL_TRIM                 = 0x2046,

    NVKEYID_NTC_NTC_PARA_CONFIG1                       = 0x2100,
    NVKEYID_NTC_NTC_PARA_CONFIG2                       = 0x2101,
    NVKEYID_NTC_NTC_PARA_TEMP_TABLE                    = 0x2102,
    NVKEYID_NTC_NTC_PARA_RARIO_TABLE                   = 0x2103,

    NVKEYID_BT_CON_BR_EDR_BDADDR                       = 0x3600,
    NVKEYID_BT_CON_BLE_BDADDR                          = 0x3601,

    NVKEYID_RF_MISC_PWR_CTL                            = 0x3720,
    NVKEYID_RF_MISC_TEMP_COMPENSATION                  = 0x3721,
    NVKEYID_RF_MISC_TEMP_COMP_HW_WORKAROUND            = 0x3722,

    NVKEYID_LM_INFO_LMP_FEATURES_PAGE0                 = 0x3821,
    NVKEYID_LM_INFO_LMP_FEATURES_PAGE1                 = 0x3822,
    NVKEYID_LM_INFO_LMP_FEATURES_PAGE2                 = 0x3823,

    NVKEYID_BT_BLE_BLE_ADV_PARAMETER                   = 0x3900,
    NVKEYID_BT_BLE_BLE_ADV_NAME                        = 0x3901,
    NVKEYID_BT_BLE_BLE_ADV_ENABLE                      = 0x3902,
    NVKEYID_FOTA_FOTA_PARA_RECONNECT                   = 0x3A00,

    NVKEYID_DSP_FW_PARA_AUDIOSETTING                   = 0xE000,
    NVKEYID_DSP_FW_PARA_AFE                            = 0xE002,
    NVKEYID_DSP_FW_PARA_ANC                            = 0xE003,
    NVKEYID_DSP_FW_PARA_DIGITAL_GAINTABLE_A2DP         = 0xE010,
    NVKEYID_DSP_FW_PARA_DIGITAL_GAINTABLE_LINE         = 0xE011,
    NVKEYID_DSP_FW_PARA_DIGITAL_GAINTABLE_SCO          = 0xE012,
    NVKEYID_DSP_FW_PARA_DIGITAL_GAINTABLE_VC           = 0xE013,
    NVKEYID_DSP_FW_PARA_DIGITAL_GAINTABLE_VP           = 0xE014,
    NVKEYID_DSP_FW_PARA_DIGITAL_GAINTABLE_RT           = 0xE015,
    NVKEYID_DSP_FW_PARA_DIGITAL_GAINTABLE_AT           = 0xE016,
    NVKEYID_DSP_FW_PARA_DIGITAL_GAINTABLE_SCO_NB       = 0xE017,
    NVKEYID_DSP_FW_PARA_ANALOG_GAINTABLE_A2DP          = 0xE020,
    NVKEYID_DSP_FW_PARA_ANALOG_GAINTABLE_LINE          = 0xE021,
    NVKEYID_DSP_FW_PARA_ANALOG_GAINTABLE_SCO           = 0xE022,
    NVKEYID_DSP_FW_PARA_ANALOG_GAINTABLE_VC            = 0xE023,
    NVKEYID_DSP_FW_PARA_ANALOG_GAINTABLE_VP            = 0xE024,
    NVKEYID_DSP_FW_PARA_ANALOG_GAINTABLE_RT            = 0xE025,
    NVKEYID_DSP_FW_PARA_ANALOG_GAINTABLE_AT            = 0xE026,
    NVKEYID_DSP_FW_PARA_ANALOG_GAINTABLE_SCO_NB        = 0xE027,
    NVKEYID_DSP_FW_PARA_DIN_GAINTABLE_A2DP             = 0xE030,
    NVKEYID_DSP_FW_PARA_DIN_GAINTABLE_LINE             = 0xE031,
    NVKEYID_DSP_FW_PARA_DIN_GAINTABLE_SCO              = 0xE032,
    NVKEYID_DSP_FW_PARA_DIN_GAINTABLE_VC               = 0xE033,
    NVKEYID_DSP_FW_PARA_DIN_GAINTABLE_VP               = 0xE034,
    NVKEYID_DSP_FW_PARA_DIN_GAINTABLE_RT               = 0xE035,
    NVKEYID_DSP_FW_PARA_DIN_GAINTABLE_AT               = 0xE036,
    NVKEYID_DSP_FW_PARA_AIN_GAINTABLE_A2DP             = 0xE040,
    NVKEYID_DSP_FW_PARA_AIN_GAINTABLE_LINE             = 0xE041,
    NVKEYID_DSP_FW_PARA_AIN_GAINTABLE_SCO              = 0xE042,
    NVKEYID_DSP_FW_PARA_AIN_GAINTABLE_VC               = 0xE043,
    NVKEYID_DSP_FW_PARA_AIN_GAINTABLE_VP               = 0xE044,
    NVKEYID_DSP_FW_PARA_AIN_GAINTABLE_RT               = 0xE045,
    NVKEYID_DSP_FW_PARA_AIN_GAINTABLE_AT               = 0xE046,
    NVKEYID_DSP_FW_PARA_AIN_GAINTABLE_SCO_NB           = 0xE047,
    NVKEYID_DSP_FW_PARA_DIN_GP_TABLE_SCO               = 0xE050,
    NVKEYID_DSP_FW_PARA_DIN_GP_TABLE_SCO_NB            = 0xE051,
    NVKEYID_DSP_FW_PARA_DIN_GP_TABLE_AT                = 0xE052,
    NVKEYID_DSP_FW_PARA_AIN_GP_TABLE_SCO_L             = 0xE060,
    NVKEYID_DSP_FW_PARA_AIN_GP_TABLE_SCO_R             = 0xE061,
    NVKEYID_DSP_FW_PARA_AIN_GP_TABLE_SCO_NB_L          = 0xE062,
    NVKEYID_DSP_FW_PARA_AIN_GP_TABLE_SCO_NB_R          = 0xE063,
    NVKEYID_DSP_FW_PARA_AIN_GP_TABLE_AT_L              = 0xE064,
    NVKEYID_DSP_FW_PARA_AIN_GP_TABLE_AT_R              = 0xE065,
    NVKEYID_DSP_ALG_PARA_DRC                           = 0xE100,
    NVKEYID_DSP_ALG_PARA_WB_TX_VO_CPD                  = 0xE101,
    NVKEYID_DSP_ALG_PARA_NB_TX_VO_CPD                  = 0xE102,
    NVKEYID_DSP_ALG_PARA_WB_RX_VO_CPD                  = 0xE103,
    NVKEYID_DSP_ALG_PARA_NB_RX_VO_CPD                  = 0xE104,
    NVKEYID_DSP_ALG_PARA_VP_CPD                        = 0xE105,
    NVKEYID_DSP_ALG_PARA_A2DP_AU_CPD                   = 0xE106,
    NVKEYID_DSP_ALG_PARA_LINE_AU_CPD                   = 0xE107,
    NVKEYID_DSP_ALG_PARA_INS                           = 0xE110,
    NVKEYID_DSP_ALG_PARA_EFFECT                        = 0xE120,
    NVKEYID_DSP_ALG_PARA_VC                            = 0xE130,
    NVKEYID_DSP_ALG_PARA_PLC                           = 0xE140,
    NVKEYID_DSP_ALG_PARA_AEC_NR                        = 0xE150,
    NVKEYID_DSP_ALG_PARA_WB_RX_EQ                      = 0xE161,
    NVKEYID_DSP_ALG_PARA_WB_TX_EQ                      = 0xE162,
    NVKEYID_DSP_ALG_PARA_NB_RX_EQ                      = 0xE163,
    NVKEYID_DSP_ALG_PARA_NB_TX_EQ                      = 0xE164,
    NVKEYID_DSP_ALG_PARA_AT_MDSP                       = 0xE170,
    NVKEYID_DSP_ALG_PARA_MDSP_AT_AGC                   = 0xE171,

    NVKEYID_PERIPHERAL_SYS_POWER_LEVEL                 = 0xF006,

    NVKEYID_APP_DEVICE_NAME_DEFAULT                    = 0xF202,
    NVKEYID_APP_DEVICE_NAME_USER_DEFINED               = 0xF203,

    NVKEYID_APP_KEY1_SETTING                           = 0xF208,
    NVKEYID_APP_KEY2_SETTING                           = 0xF209,
    NVKEYID_APP_KEY3_SETTING                           = 0xF20A,
    NVKEYID_APP_KEY4_SETTING                           = 0xF20B,
    NVKEYID_APP_KEY5_SETTING                           = 0xF20C,
    NVKEYID_APP_KEY6_SETTING                           = 0xF20D,
    NVKEYID_APP_KEY7_SETTING                           = 0xF20E,
    NVKEYID_APP_KEY8_SETTING                           = 0xF20F,
    NVKEYID_APP_KEY9_SETTING                           = 0xF210,
    NVKEYID_APP_KEY10_SETTING                          = 0xF211,
    NVKEYID_APP_KEY_BTN_SEQ_PATTERN                    = 0xF212,

    NVKEYID_APP_VOLUME_INFORMATION                     = 0xF218,

    NVKEYID_APP_SYSTEM_INFO_CONFIG                     = 0xF21C,
    NVKEYID_APP_PAIRING_INFORMATION                    = 0xF21D,
    NVKEYID_APP_NVRAM_APP_FEATURE                      = 0xF21E,
    NVKEYID_APP_BAT_CHG_INFORMATION                    = 0xF21F,

    NVKEYID_APP_KEY_EVENT_START                        = 0xF220,
    NVKEYID_APP_KEY_ACTION_DOWN                        = 0xF22E,
    NVKEYID_APP_KEY_ACTION_UP                          = 0xF22F,

    NVKEYID_APP_VP_LANG                                = 0xF230, //hf.wang for VP lang
    NVKEYID_APP_PEQ_AUDIO_PATH_TABLE                   = 0xF233,
    NVKEYID_APP_PEQ_GROUP_TO_PARA_TABLE1               = 0xF234,
    NVKEYID_APP_PEQ_GROUP_TO_PARA_TABLE2               = 0xF235,
    NVKEYID_APP_PEQ_GROUP_TO_PARA_TABLE3               = 0xF236,
    NVKEYID_APP_PEQ_GROUP_TO_PARA_TABLE4               = 0xF237,
    NVKEYID_APP_PEQ_GROUP_TO_PARA_TABLE5               = 0xF238,
    NVKEYID_APP_PEQ_GROUP_TO_PARA_TABLE6               = 0xF239,
    NVKEYID_APP_VOLUME_SCO_SETTING                     = 0xF23A,
    NVKEYID_APP_VOLUME_A2DP_SETTING                    = 0xF23B,
    NVKEYID_APP_VOLUME_MP3_SETTING                     = 0xF23C,
    NVKEYID_APP_VOLUME_SCOMIC_SETTING                  = 0xF23D,
    NVKEYID_APP_VOLUME_VPRT_SETTING                    = 0xF23E,
    NVKEYID_APP_VOLUME_LINEIN_SETTING                  = 0xF23F,

    NVKEY_ID_LEFT_OR_RIGHT                             = 0xF240,

    NVKEYID_APP_POWER_INFORMATION                      = 0xF246,

    NVKEYID_APP_PEQ_PARAMETER1                         = 0xF260,
    NVKEYID_APP_PEQ_PARAMETER2                         = 0xF261,
    NVKEYID_APP_PEQ_PARAMETER3                         = 0xF262,
    NVKEYID_APP_PEQ_PARAMETER4                         = 0xF263,
    NVKEYID_APP_PEQ_PARAMETER5                         = 0xF264,
    NVKEYID_APP_PEQ_PARAMETER6                         = 0xF265,
    NVKEYID_APP_PEQ_PARAMETER7                         = 0xF266,
    NVKEYID_APP_PEQ_PARAMETER8                         = 0xF267,
    NVKEYID_APP_PEQ_PARAMETER9                         = 0xF268,
    NVKEYID_APP_PEQ_PARAMETER10                        = 0xF269,
    NVKEYID_APP_PEQ_PARAMETER11                        = 0xF26A,
    NVKEYID_APP_PEQ_PARAMETER12                        = 0xF26B,
    NVKEYID_APP_PEQ_PARAMETER13                        = 0xF26C,
    NVKEYID_APP_PEQ_PARAMETER14                        = 0xF26D,
    NVKEYID_APP_PEQ_PARAMETER15                        = 0xF26E,
    NVKEYID_APP_PEQ_PARAMETER16                        = 0xF26F,
    NVKEYID_APP_PEQ_PARAMETER17                        = 0xF270,
    NVKEYID_APP_PEQ_PARAMETER18                        = 0xF271,
    NVKEYID_APP_PEQ_PARAMETER19                        = 0xF272,
    NVKEYID_APP_PEQ_PARAMETER20                        = 0xF273,
    NVKEYID_APP_PEQ_PARAMETER21                        = 0xF274,
    NVKEYID_APP_PEQ_PARAMETER22                        = 0xF275,
    NVKEYID_APP_PEQ_PARAMETER23                        = 0xF276,
    NVKEYID_APP_PEQ_PARAMETER24                        = 0xF277,
    NVKEYID_APP_PEQ_PARAMETER25                        = 0xF278,
    NVKEYID_APP_PEQ_PARAMETER26                        = 0xF279,
    NVKEYID_APP_PEQ_PARAMETER27                        = 0xF27A,
    NVKEYID_APP_PEQ_PARAMETER28                        = 0xF27B,
    NVKEYID_APP_PEQ_PARAMETER29                        = 0xF27C,
    NVKEYID_APP_PEQ_PARAMETER30                        = 0xF27D,
    NVKEYID_APP_PEQ_PARAMETER31                        = 0xF27E,
    NVKEYID_APP_PEQ_PARAMETER32                        = 0xF27F,

    NVKEYID_APP_LED_PATTERN                             = 0xF280,

    NVKEYID_APP_FAST_PAIR                              = 0xF2E0,
    
    NVKEYID_BT_MUSIC_AAC_ENABLE                        = 0xF2E1,
    NVKEYID_BT_RELAY_ENABLE                            = 0xF2E2,
    NVKEYID_BT_DUT_ENABLE                              = 0xF2E3,
    NVKEYID_BT_DEFAULT_TX_POWER                        = 0xF2E4,

    NVKEYID_PROFILE_AWS_MCE_CONFIG                     = 0xF2E5,
} NVKey_ID_list;

#endif /* #if (PRODUCT_VERSION == 1552) */

#endif /* __NVKEY_ID_LIST_H__ */

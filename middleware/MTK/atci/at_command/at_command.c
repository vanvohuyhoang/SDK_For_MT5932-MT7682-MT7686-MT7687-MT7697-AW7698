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

// System head file
#include "FreeRTOS.h"
#include "task.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// For Register AT command handler
#include "at_command.h"
#include "hal_feature_config.h"

#include "verno.h"
#include "syslog.h"

#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
log_create_module(atcmd, PRINT_LEVEL_WARNING);
log_create_module(atci, PRINT_LEVEL_WARNING);
#else
log_create_module(atcmd, PRINT_LEVEL_WARNING);
#endif

#define LOGE(fmt,arg...)   LOG_E(atcmd, "ATCMD: "fmt,##arg)
#define LOGW(fmt,arg...)   LOG_W(atcmd, "ATCMD: "fmt,##arg)
#define LOGI(fmt,arg...)   LOG_I(atcmd ,"ATCMD: "fmt,##arg)

#define LOGMSGIDE(fmt,cnt,arg...)   LOG_MSGID_E(atcmd, "ATCMD: "fmt,cnt,##arg)
#define LOGMSGIDW(fmt,cnt,arg...)   LOG_MSGID_W(atcmd, "ATCMD: "fmt,cnt,##arg)
#define LOGMSGIDI(fmt,cnt,arg...)   LOG_MSGID_I(atcmd ,"ATCMD: "fmt,cnt,##arg)


/*--- Function ---*/
extern uint16_t atci_local_caculate_hash_value(uint8_t *at_name,uint32_t *hash_value1,uint32_t *hash_value2);

#if !defined ATCI_TEST_AT_COMMAND_DISABLE
atci_status_t atci_cmd_hdlr_test(atci_parse_cmd_param_t *parse_cmd);
#endif
#if defined MTK_SWITCH_TO_RACE_COMMAND_ENABLE
atci_status_t atci_cmd_hdlr_switch(atci_parse_cmd_param_t *parse_cmd);
#endif
#if defined(MTK_QUERY_SDK_VERSION) && defined(MTK_FW_VERSION)
extern atci_status_t atci_cmd_hdlr_sdkinfo(atci_parse_cmd_param_t *parse_cmd);
#endif //  defined(MTK_QUERY_SDK_VERSION) && defined(MTK_FW_VERSION)

#ifndef MTK_AT_CMD_DISABLE
/*--- Function ---*/
#if defined(__GNUC__) && defined (TOOL_APP_MODULE)
extern atci_status_t atci_cmd_hdlr_testframework(atci_parse_cmd_param_t *parse_cmd);
#endif

#if !defined(MTK_DEBUG_LEVEL_PRINTF)
extern atci_status_t atci_cmd_hdlr_syslog(atci_parse_cmd_param_t *parse_cmd);
#endif

#ifndef MTK_MEM_AT_COMMAND_DISABLE
extern atci_status_t atci_cmd_hdlr_mem(atci_parse_cmd_param_t *parse_cmd);
#endif

#if defined(DEVICE_HDK) || defined(DEVICE_BAND)

#ifdef MTK_SENSOR_BIO_USE
extern atci_status_t atci_cmd_hdlr_biogui(atci_parse_cmd_param_t *parse_cmd);
#endif

#else // defined(DEVICE_HDK) || defined(DEVICE_BAND)
extern atci_status_t atci_cmd_hdlr_camera(atci_parse_cmd_param_t *parse_cmd);

#ifdef MTK_GNSS_ENABLE
extern atci_status_t gnss_power_control_at_handler(atci_parse_cmd_param_t *parse_cmd);
extern atci_status_t gnss_send_command_at_handler(atci_parse_cmd_param_t *parse_cmd);
#endif

#ifdef HAL_DISPLAY_LCD_MODULE_ENABLED
extern	atci_status_t atci_cmd_hdlr_lcm(atci_parse_cmd_param_t *parse_cmd);
#endif

#ifdef HAL_CHARGER_MODULE_ENABLED
extern atci_status_t atci_cmd_hdlr_charger(atci_parse_cmd_param_t *parse_cmd);
#endif

#if defined (MTK_EXTERNAL_GAUGE) && defined (MTK_BUILD_SMT_LOAD)
extern atci_status_t atci_cmd_hdlr_external_gauge(atci_parse_cmd_param_t *parse_cmd);
#endif

#ifndef MTK_EXTERNAL_PMIC
extern atci_status_t atci_cmd_hdlr_pmu(atci_parse_cmd_param_t *parse_cmd);
#endif

#if !defined (MTK_PMIC_AT_COMMAND_DISABLE) && defined (MTK_EXTERNAL_PMIC)
extern atci_status_t atci_cmd_hdlr_external_pmic(atci_parse_cmd_param_t *parse_cmd);
#endif

#if !defined (MTK_REG_AT_COMMAND_DISABLE) && defined (HAL_REG_MODULE_ENABLED)
extern atci_status_t atci_cmd_hdlr_reg(atci_parse_cmd_param_t *parse_cmd);
#endif

#ifdef HAL_RTC_MODULE_ENABLED
extern atci_status_t atci_cmd_hdlr_rtc(atci_parse_cmd_param_t *parse_cmd);
#endif

#if defined(HAL_SD_MODULE_ENABLED) && (defined(MTK_BUILD_SMT_LOAD) || defined(MTK_BUILD_HQA_LOAD))
extern atci_status_t atci_cmd_hdlr_msdc(atci_parse_cmd_param_t *parse_cmd);
#endif


#if defined(MTK_NVDM_ENABLE)
extern atci_status_t atci_cmd_hdlr_nvdm(atci_parse_cmd_param_t *parse_cmd);
#endif

#if defined(MTK_NVDM_MODEM_ENABLE)
extern atci_status_t atci_cmd_hdlr_nvdm_modem(atci_parse_cmd_param_t *parse_cmd);
#endif

#if defined(MTK_PORT_SERVICE_ENABLE)
extern atci_status_t atci_cmd_hdlr_serial_port(atci_parse_cmd_param_t *parse_cmd);
#endif

#if defined(HAL_GPIO_MODULE_ENABLED)
#ifdef MTK_BUILD_SMT_LOAD
extern atci_status_t atci_cmd_hdlr_led(atci_parse_cmd_param_t *parse_cmd);
#endif
extern atci_status_t atci_cmd_hdlr_gpio(atci_parse_cmd_param_t *parse_cmd);
#endif

#if defined(HAL_CAPTOUCH_MODULE_ENABLED)
extern atci_status_t atci_cmd_hdlr_captouch(atci_parse_cmd_param_t *parse_cmd);
#endif

#if defined(MTK_EINT_KEY_ENABLE)
extern atci_status_t atci_cmd_hdlr_eint_key(atci_parse_cmd_param_t *parse_cmd);
#endif

#if !defined(MTK_AES_AT_COMMAND_DISABLE) && defined(__GNUC__) && defined(HAL_AES_MODULE_ENABLED)
extern atci_status_t atci_cmd_hdlr_crypto(atci_parse_cmd_param_t *parse_cmd);
#endif

#if !defined(MTK_DVFS_AT_COMMAND_DISABLE) && defined(HAL_DVFS_MODULE_ENABLED)
extern atci_status_t atci_cmd_hdlr_dvfs_get(atci_parse_cmd_param_t *parse_cmd);
extern atci_status_t atci_cmd_hdlr_dvfs_set(atci_parse_cmd_param_t *parse_cmd);
extern atci_status_t atci_cmd_hdlr_dvfs_dbg(atci_parse_cmd_param_t *parse_cmd);
#endif

#if !defined(MTK_CLOCK_AT_COMMAND_DISABLE) && defined(HAL_CLOCK_MODULE_ENABLED)
extern atci_status_t atci_cmd_hdlr_clock(atci_parse_cmd_param_t *parse_cmd);
#endif

#if (PRODUCT_VERSION == 2523) || (PRODUCT_VERSION == 2533)
#if defined(HAL_DISPLAY_PWM_MODULE_ENABLED) || defined(HAL_ISINK_MODULE_ENABLED)
extern atci_status_t atci_cmd_hdlr_backlight(atci_parse_cmd_param_t *parse_cmd);
#endif
#endif

#if (PRODUCT_VERSION == 2523 || PRODUCT_VERSION == 2533 || PRODUCT_VERSION == 2625 || PRODUCT_VERSION == 1552)
#ifdef HAL_SLEEP_MANAGER_ENABLED
extern atci_status_t atci_cmd_hdlr_sleep_manager(atci_parse_cmd_param_t *parse_cmd);
#endif
#endif

#if !defined(MTK_AUDIO_AT_COMMAND_DISABLE) && defined(__GNUC__) && defined(HAL_AUDIO_MODULE_ENABLED)
extern atci_status_t atci_cmd_hdlr_audio(atci_parse_cmd_param_t *parse_cmd);
#endif

#if defined(MTK_AUDIO_TUNING_ENABLED)
extern atci_status_t atci_cmd_hdlr_eaps(atci_parse_cmd_param_t *parse_cmd);
extern atci_status_t atci_cmd_hdlr_attdet(atci_parse_cmd_param_t *parse_cmd);
extern atci_status_t atci_cmd_hdlr_atttest(atci_parse_cmd_param_t *parse_cmd);
#endif

#ifdef __CFW_CONFIG_MODE__
extern atci_status_t atci_cmd_hdlr_cfw(atci_parse_cmd_param_t *parse_cmd);
#endif
#if defined(MTK_BT_AT_COMMAND_ENABLE)
extern atci_status_t atci_cmd_hdlr_bt_ata(atci_parse_cmd_param_t *parse_cmd);
extern atci_status_t atci_cmd_hdlr_bt_power(atci_parse_cmd_param_t *parse_cmd);
extern atci_status_t atci_cmd_hdlr_bt_relay(atci_parse_cmd_param_t *parse_cmd);
extern atci_status_t atci_cmd_hdlr_bt_send_hci_command(atci_parse_cmd_param_t *parse_cmd);
extern atci_status_t atci_cmd_hdlr_bt_send_hci_command_to_host(atci_parse_cmd_param_t *parse_cmd);
extern atci_status_t atci_cmd_hdlr_bt_testbox(atci_parse_cmd_param_t *parse_cmd);
extern atci_status_t atci_cmd_hdlr_capid(atci_parse_cmd_param_t *parse_cmd);
extern atci_status_t atci_cmd_hdlr_bt_testbox_btaddr(atci_parse_cmd_param_t *parse_cmd);
extern atci_status_t atci_cmd_hdlr_bt_testbox_CW_tone_readRSSI(atci_parse_cmd_param_t *parse_cmd);
extern atci_status_t atci_cmd_hdlr_bt_enter_test_mode(atci_parse_cmd_param_t *parse_cmd);
#endif

#ifdef __BT_DEBUG__
extern atci_status_t atci_cmd_hdlr_bt_enable_driver_dump_log(atci_parse_cmd_param_t *parse_cmd);
#endif

#ifndef MTK_EXTERNAL_PMIC
extern atci_status_t atci_cmd_hdlr_vibrator(atci_parse_cmd_param_t *parse_cmd);
#endif


#ifdef MTK_CTP_ENABLE
extern atci_status_t atci_cmd_hdlr_ctp(atci_parse_cmd_param_t *parse_cmd);
extern atci_status_t atci_cmd_hdlr_ctp_simulate(atci_parse_cmd_param_t *parse_cmd);
#endif

#if !defined(MTK_KEYPAD_AT_COMMAND_DISABLE) && defined(HAL_KEYPAD_MODULE_ENABLED)
atci_status_t atci_cmd_hdlr_keypad(atci_parse_cmd_param_t *parse_cmd);
extern atci_status_t atci_cmd_hdlr_keypad_simulate(atci_parse_cmd_param_t *parse_cmd);
#endif

#ifdef HAL_WDT_MODULE_ENABLED
atci_status_t atci_cmd_hdlr_wdt(atci_parse_cmd_param_t *parse_cmd);
#endif


#ifdef MTK_SENSOR_AT_COMMAND_ENABLE
extern  atci_status_t atci_cmd_hdlr_sensor(atci_parse_cmd_param_t *parse_cmd);
#endif

#ifdef MTK_SENSOR_ACCELEROMETER_USE_BMA255
extern atci_status_t atci_cmd_hdlr_gsensor(atci_parse_cmd_param_t *parse_cmd);
#endif

#ifdef MTK_SENSOR_BIO_USE
extern atci_status_t atci_cmd_hdlr_biogui(atci_parse_cmd_param_t *parse_cmd);
#endif

#endif //end of DEVICE_HDK || DEVICE_BAND

#ifdef MTK_SYSTEM_AT_COMMAND_ENABLE
extern atci_status_t atci_cmd_hdlr_system(atci_parse_cmd_param_t *parse_cmd);
#endif

#ifdef MTK_OS_CPU_UTILIZATION_ENABLE
extern atci_status_t atci_cmd_hdlr_utilization(atci_parse_cmd_param_t *parse_cmd);
#endif

#if defined(MTK_FOTA_ENABLE) && defined(MTK_FOTA_AT_COMMAND_ENABLE)
extern atci_status_t atci_cmd_hdlr_fota_cmd(atci_parse_cmd_param_t *parse_cmd);
#endif

#if defined(MTK_FW_VERSION) && ((PRODUCT_VERSION == 7687) || (PRODUCT_VERSION == 7697) || (PRODUCT_VERSION == 7698) || (PRODUCT_VERSION == 7686) || (PRODUCT_VERSION == 7682) || (PRODUCT_VERSION == 5932))
extern atci_status_t atci_cmd_hdlr_sys(atci_parse_cmd_param_t *parse_cmd);
#endif // defined(MTK_FW_VERSION) && ((PRODUCT_VERSION == 7687) || (PRODUCT_VERSION == 7697) || (PRODUCT_VERSION == 7686) )	

extern atci_status_t atci_cmd_hdlr_usimsmt(atci_parse_cmd_param_t *parse_cmd);

#ifdef MTK_SPI_EXTERNAL_SERIAL_FLASH_ENABLED
extern atci_status_t atci_cmd_hdlr_serial_flash(atci_parse_cmd_param_t *parse_cmd);
#endif

#ifdef HAL_PWM_MODULE_ENABLED
extern atci_status_t atci_cmd_hdlr_pwm(atci_parse_cmd_param_t *parse_cmd);
#endif

#endif // MTK_AT_CMD_DISABLE
/*---  Variant ---*/



/* the comands in atcmd_table are supported when atci feature is on.
put all the default at cmd handler into atcmd_table, then they will be  registed in atci_init() */
#ifndef ATCI_APB_PROXY_ADAPTER_ENABLE
atci_cmd_hdlr_item_t atcmd_table[] = {

#ifndef MTK_AT_CMD_DISABLE

#if defined(DEVICE_HDK) || defined(DEVICE_BAND)
#ifdef MTK_SENSOR_BIO_USE
    {"AT+EBIO", atci_cmd_hdlr_biogui, 277605, 0},
#endif

#else //else defined(DEVICE_HDK) || defined(DEVICE_BAND)

#ifdef MTK_ATCI_CAMERA_ENABLE
    {"AT+ECMP",        atci_cmd_hdlr_camera,       279202, 0},
#endif


#ifdef HAL_DISPLAY_LCD_MODULE_ENABLED
    {"AT+ELCM",        atci_cmd_hdlr_lcm,       291815, 0},
#endif
#ifdef MTK_GNSS_ENABLE
    {"AT+EGPSC", gnss_power_control_at_handler, 10833613, 0},
    {"AT+EGPSS", gnss_send_command_at_handler , 10833629, 0},
#endif

#ifdef HAL_CHARGER_MODULE_ENABLED
    {"AT+ECHAR", atci_cmd_hdlr_charger, 10601904, 0},
#endif

#ifdef MTK_EXTERNAL_GAUGE
#ifdef MTK_BUILD_SMT_LOAD
    {"AT+EEXTGAUGE", atci_cmd_hdlr_external_gauge, 10735463, 85467},
#endif
#endif

#ifndef MTK_EXTERNAL_PMIC
    {"AT+EPMUREG",	atci_cmd_hdlr_pmu,	    11323220, 197},
#endif

#if !defined (MTK_PMIC_AT_COMMAND_DISABLE) && defined (MTK_EXTERNAL_PMIC)
    {"AT+EPMICREG",	atci_cmd_hdlr_external_pmic,	    11322749, 26189},
#endif


#if !defined (MTK_REG_AT_COMMAND_DISABLE) && defined (HAL_REG_MODULE_ENABLED)
    {"AT+EREG", atci_cmd_hdlr_reg,          300549, 0},
#endif

#ifdef HAL_RTC_MODULE_ENABLED
    {"AT+ERTCREG", atci_cmd_hdlr_rtc, 11442388, 197},
#ifdef MTK_BUILD_SMT_LOAD
    {"AT+ERTC", atci_cmd_hdlr_rtc, 301115, 0},
    {"AT+ERTC32K", atci_cmd_hdlr_rtc, 11442374, 125},
#endif
#endif

#if defined(HAL_SD_MODULE_ENABLED) && (defined(MTK_BUILD_SMT_LOAD) || defined(MTK_BUILD_HQA_LOAD))
{"AT+EMSDC", atci_cmd_hdlr_msdc, 11166607, 0},
#endif

#if defined(MTK_NVDM_ENABLE)
    {"AT+ENVDM", atci_cmd_hdlr_nvdm, 11225821, 0},
#endif

#if defined(MTK_NVDM_MODEM_ENABLE)
    {"AT+ENVDMMD", atci_cmd_hdlr_nvdm_modem, 11225821, 498},
#endif

#if defined(MTK_PORT_SERVICE_ENABLE)
    {"AT+EPORT", atci_cmd_hdlr_serial_port, 11325996, 0},
#endif

#if defined(HAL_GPIO_MODULE_ENABLED)
#ifdef MTK_BUILD_SMT_LOAD
    {"AT+ELED", atci_cmd_hdlr_led, 291882, 0},
#endif
    {"AT+EGPIO", atci_cmd_hdlr_gpio, 10833245, 0},
#endif

#if defined(HAL_CAPTOUCH_MODULE_ENABLED)
	{"AT+ECPT", atci_cmd_hdlr_captouch, 279320, 0},
#endif

#if defined(MTK_EINT_KEY_ENABLE)
	{"AT+EKEY", atci_cmd_hdlr_eint_key, 290459, 0},
#endif

#if (PRODUCT_VERSION == 2523) || (PRODUCT_VERSION == 2533)
#if defined(HAL_DISPLAY_PWM_MODULE_ENABLED) || defined(HAL_ISINK_MODULE_ENABLED)
    {"AT+EBLT", atci_cmd_hdlr_backlight, 277724, 0},
#endif
#endif

#if (PRODUCT_VERSION == 2523 || PRODUCT_VERSION == 2533 || PRODUCT_VERSION == 2625 || PRODUCT_VERSION == 1552)
#ifdef HAL_SLEEP_MANAGER_ENABLED
    {"AT+SM", atci_cmd_hdlr_sleep_manager, 735, 0},
#endif
#endif

#if !defined(MTK_AUDIO_AT_COMMAND_DISABLE) && defined(__GNUC__) && defined(HAL_AUDIO_MODULE_ENABLED)
    {"AT+EAUDIO", atci_cmd_hdlr_audio, 10511037, 15},
#endif

#ifndef MTK_EXTERNAL_PMIC
    {"AT+EVIB", atci_cmd_hdlr_vibrator, 306472, 0},
#endif

#if defined(MTK_AUDIO_TUNING_ENABLED)
    {"AT+EAPS", atci_cmd_hdlr_eaps, 276431, 0},
    {"AT+ATTDET", atci_cmd_hdlr_attdet, 3211613, 20},
    {"AT+ATTTEST", atci_cmd_hdlr_atttest, 3212221, 742},
#endif

#ifdef MTK_CTP_ENABLE
    {"AT+EPENURC", atci_cmd_hdlr_ctp, 11311405, 687},
    {"AT+ECTP", atci_cmd_hdlr_ctp_simulate, 279468, 0},
#endif

#if !defined(MTK_KEYPAD_AT_COMMAND_DISABLE) && defined(HAL_KEYPAD_MODULE_ENABLED)
    {"AT+EKEYURC", atci_cmd_hdlr_keypad, 11037463, 687},
    {"AT+EKP", atci_cmd_hdlr_keypad_simulate, 7654, 0},
#endif

#ifdef MTK_SENSOR_AT_COMMAND_ENABLE
    {"AT+ESENSOR", atci_cmd_hdlr_sensor, 11476019, 588},
    {"AT+ESENSORTEST", atci_cmd_hdlr_sensor, 11476019, 32293825},
#endif

#ifdef __CFW_CONFIG_MODE__
    {"AT+ECFG", atci_cmd_hdlr_cfw, 278927, 0},
#endif
#ifdef MTK_BT_AT_COMMAND_ENABLE
    {"AT+EBTAT", atci_cmd_hdlr_bt_ata, 10564362, 0},
    {"AT+EBTPW", atci_cmd_hdlr_bt_power, 10564935, 0},
    {"AT+EBTER", atci_cmd_hdlr_bt_relay, 10564512, 0},
    {"AT+EBTSHC", atci_cmd_hdlr_bt_send_hci_command, 10565034, 3},
    {"AT+EBTSHCD", atci_cmd_hdlr_bt_send_hci_command_to_host, 10565034, 118},
    {"AT+EBTTB", atci_cmd_hdlr_bt_testbox, 10565066, 0},
    {"AT+CAPID", atci_cmd_hdlr_capid, 6333730, 0},
    {"AT+EBTADDR", atci_cmd_hdlr_bt_testbox_btaddr, 10564346, 170},
    {"AT+EBBTRXRSSI", atci_cmd_hdlr_bt_testbox_CW_tone_readRSSI, 10539090, 51059127},
    {"AT+EBTENTEST", atci_cmd_hdlr_bt_enter_test_mode, 10564508, 1105402},
#endif

#ifdef __BT_DEBUG__
    {"AT+EBTEDL", atci_cmd_hdlr_bt_enable_driver_dump_log, 10564498, 12},
#endif

#ifdef HAL_WDT_MODULE_ENABLED
    {"AT+EWDT", atci_cmd_hdlr_wdt, 307744, 0},
#endif

#if !defined(MTK_AES_AT_COMMAND_DISABLE) && defined(__GNUC__) && defined(HAL_AES_MODULE_ENABLED)
    {"AT+ECRYPTO", atci_cmd_hdlr_crypto, 10617254, 775},
#endif

#if !defined(MTK_DVFS_AT_COMMAND_DISABLE) && defined(HAL_DVFS_MODULE_ENABLED)
    {"AT+CPUFGET", atci_cmd_hdlr_dvfs_get, 7163919, 210},
    {"AT+CPUFSET", atci_cmd_hdlr_dvfs_set, 7163931, 210},
    {"AT+DVFSDBG", atci_cmd_hdlr_dvfs_dbg, 9557118, 83},
#endif

#if !defined(MTK_CLOCK_AT_COMMAND_DISABLE) && defined(HAL_CLOCK_MODULE_ENABLED)
    {"AT+CLOCK",       atci_cmd_hdlr_clock,      6935657, 0},
#endif //HAL_CLOCK_MODULE_ENABLED

#if defined(__GNUC__) && defined (TOOL_APP_MODULE)
    {"AT+TF",atci_cmd_hdlr_testframework,766, 0},
#endif

#if !defined(MTK_DEBUG_LEVEL_PRINTF)
#if !defined(MTK_DEBUG_LEVEL_NONE)
    {"AT+SYSLOG",   atci_cmd_hdlr_syslog, 41017291, 7},
#endif
#endif

#ifdef MTK_SENSOR_ACCELEROMETER_USE_BMA255
    {"AT+EGSENSOR", atci_cmd_hdlr_gsensor, 10837424, 28024},
#endif

#ifdef MTK_SENSOR_BIO_USE
    {"AT+EBIO", atci_cmd_hdlr_biogui, 277605, 0},
#endif

#endif //end of defined(DEVICE_HDK) || defined(DEVICE_BAND)

#if defined(MTK_FOTA_ENABLE) && defined(MTK_FOTA_AT_COMMAND_ENABLE)
    {"AT+FOTA", atci_cmd_hdlr_fota_cmd, 351653, 0},
#endif

#ifdef MTK_OS_CPU_UTILIZATION_ENABLE
    {"AT+UTILIZATION", atci_cmd_hdlr_utilization, 44898757, 54297645},
#endif
#ifdef MTK_SYSTEM_AT_COMMAND_ENABLE
    {"AT+SYSTEM", atci_cmd_hdlr_system, 41017585, 13},
#endif

#ifndef MTK_MEM_AT_COMMAND_DISABLE
    {"AT+MEM",          atci_cmd_hdlr_mem,          18975, 0},
#endif

#if defined(MTK_FW_VERSION) && ((PRODUCT_VERSION == 7687) || (PRODUCT_VERSION == 7697) || (PRODUCT_VERSION == 7698) || (PRODUCT_VERSION == 7686) || (PRODUCT_VERSION == 7682) || (PRODUCT_VERSION == 5932))
    {"AT+SYS", 		atci_cmd_hdlr_sys, 		28405, 0},
#endif // defined(MTK_FW_VERSION) && ((PRODUCT_VERSION == 7687) || (PRODUCT_VERSION == 7697) || (PRODUCT_VERSION == 7686) )

#if ((PRODUCT_VERSION == 1552) || (PRODUCT_VERSION == 2811) ||(PRODUCT_VERSION == 7698))
#ifdef HAL_PWM_MODULE_ENABLED
    {"AT+EPWM",         atci_cmd_hdlr_pwm,          298351, 0},
#endif
#endif

#endif // MTK_AT_CMD_DISABLE

#if defined(MTK_USIMSMT_ENABLED) && defined(MTK_BUILD_SMT_LOAD)
    {"AT+EUSIMSMT",     atci_cmd_hdlr_usimsmt,         11605783, 27950},
#endif

#if defined(MTK_SPI_EXTERNAL_SERIAL_FLASH_ENABLED)
    {"AT+EFLASH",     atci_cmd_hdlr_serial_flash,    10772297, 8},
#endif
#if defined MTK_SWITCH_TO_RACE_COMMAND_ENABLE
    {"AT+SWITCH", atci_cmd_hdlr_switch, 40893399, 8},
#endif

#if defined(MTK_QUERY_SDK_VERSION) && defined(MTK_FW_VERSION)
    {"AT+EVERINFO",       atci_cmd_hdlr_sdkinfo,      11640777, 20459},
#endif //  defined(MTK_QUERY_SDK_VERSION) && defined(MTK_FW_VERSION)

#if !defined ATCI_TEST_AT_COMMAND_DISABLE
    {"AT+TEST",         atci_cmd_hdlr_test,         1105402, 0},
#endif

};
#else
/*For AP Bridge Proxy adapter case, the at commands table is in file: apb_proxy_atci_adapter_cmd_def.h*/
atci_cmd_hdlr_item_t atcmd_table[] = {
    {NULL, NULL, 0}
};
#endif

atci_status_t at_command_init(void)
{
    atci_status_t ret = ATCI_STATUS_REGISTRATION_FAILURE;
    //int32_t item_size;

    /* -------  Scenario: register AT handler in CM4 -------  */
    ret = atci_register_handler(atcmd_table, sizeof(atcmd_table) / sizeof(atci_cmd_hdlr_item_t));
    if (ret == ATCI_STATUS_OK) {
        LOGMSGIDI("at_cmd_init register success\r\n", 0);
    } else {
        LOGMSGIDE("at_cmd_init register fail\r\n", 0);
    }

    return ret;
}

#if !defined ATCI_TEST_AT_COMMAND_DISABLE
// AT command handler
atci_status_t atci_cmd_hdlr_test(atci_parse_cmd_param_t *parse_cmd)
{
    int read_value = 0;
    atci_response_t response = {{0}};
    char *param = NULL;
    uint32_t *hash1 = NULL;
    uint32_t *hash2 = NULL;
    int index = 7;    //char '=' position    
    LOGMSGIDI("atci_cmd_hdlr_test\r\n", 0);

    if(NULL == parse_cmd)
    {
        response.response_len = 0;
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
        atci_send_response(&response);

        return ATCI_STATUS_ERROR;
    }
    
    response.response_flag = 0; // Command Execute Finish.
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    response.cmd_id = parse_cmd->cmd_id;
#endif

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:    // rec: AT+TEST=?
            LOGMSGIDI("AT Test OK.\n", 0);
            strcpy((char *)response.response_buf, "+TEST:(0,1)\r\n");
            response.response_len = strlen((char *)response.response_buf);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;  // ATCI will help append OK at the end of resonse buffer
            atci_send_response(&response);
            break;

        case ATCI_CMD_MODE_READ:    // rec: AT+TEST?
            LOGMSGIDI("AT Read done.\n", 0);
            snprintf((char *)response.response_buf, sizeof(response.response_buf), "+TEST:%d\r\n", read_value);
            response.response_len = strlen((char *)response.response_buf);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;  // ATCI will help append OK at the end of resonse buffer
            atci_send_response(&response);
            break;

        case ATCI_CMD_MODE_ACTIVE:  // rec: AT+TEST
            LOGMSGIDI("AT Active OK.\n", 0);
            // assume the active mode is invalid and we will return "ERROR"
            //strcpy((char *)response.response_buf, "ERROR\r\n");
            response.response_len = strlen((char *)response.response_buf);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR; // ATCI will help append "ERROR" at the end of response buffer
            atci_send_response(&response);
            break;

        case ATCI_CMD_MODE_EXECUTION: // rec: AT+TEST=<p1>  the handler need to parse the parameters
            LOGMSGIDI("AT Executing...\r\n", 0);
            param = parse_cmd->string_ptr;
            //parsing the parameter
            if(param == NULL)
            {
                response.response_len = 0;
                response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
                atci_send_response(&response);
                break;
            }
            param = param+index+1;
            param = strtok(param, ",\n\r");
            if (param != NULL && strlen(param) > 0) {
                LOGI("AT Executing...param=%s\r\n,param[0]=%c",param,param[0]);
                if ((param[0] == '0' || param[0] == '1')) {
                    // valid parameter, update the data and return "OK"
                    response.response_len = 0;
                    response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK; // ATCI will help append "OK" at the end of response buffer
                    atci_send_response(&response);
                } else if ((param[0] == '2' || param[0] == '3')) {
                    if (param[0] == '2') {
                        log_config_print_level(atci, PRINT_LEVEL_INFO);
                        log_config_print_level(atcmd, PRINT_LEVEL_INFO);
                        LOGMSGIDI("change atci log print level into info.\n", 0);
                        strcpy((char *)response.response_buf, "PRINT_LEVEL_INFO\r\nOK\r\n");
                        response.response_len = strlen((char *)response.response_buf);
                        atci_send_response(&response);
                    } else {
                        log_config_print_level(atci, PRINT_LEVEL_WARNING);
                        log_config_print_level(atcmd, PRINT_LEVEL_WARNING);
                        LOGMSGIDW("change atci log print level into warning.\n", 0);
                        strcpy((char *)response.response_buf, "PRINT_LEVEL_WARNING\r\nOK\r\n");
                        response.response_len = strlen((char *)response.response_buf);
                        atci_send_response(&response);
                    }
                    break;
                } else if (param[0] == '4') {
                    if (hash1 == NULL) {
                        hash1 = (uint32_t *)atci_mem_alloc(sizeof(uint32_t));
                        memset(hash1, 0, sizeof(uint32_t));
                    }
                    if (hash2 == NULL) {
                        hash2 = (uint32_t *)atci_mem_alloc(sizeof(uint32_t));
                        memset(hash2, 0, sizeof(uint32_t));
                    }
                    atci_local_caculate_hash_value((uint8_t *)(param+2),hash1,hash2);
                    LOGMSGIDW("AT Read mode after caculate, hash1=%x,hash2=%x",2,*hash1,*hash2);
                    snprintf((char *)response.response_buf, sizeof(response.response_buf), "+hash1:%lx,hash2:%lx\r\n", *hash1,*hash2);
                    response.response_len = strlen((char *)response.response_buf);
                    response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;  // ATCI will help append OK at the end of resonse buffer
                    atci_send_response(&response);
                    atci_mem_free(hash1);
                    hash1 = NULL;
                    atci_mem_free(hash2);
                    hash2 = NULL;
                    break;
                } else {
                    // invalide parameter, return "ERROR"
                    response.response_len = 0;
                    response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR; // ATCI will help append "ERROR" at the end of response buffer
                    atci_send_response(&response);
                }
            }
            for(int i = 0; i < (ATCI_UART_TX_FIFO_BUFFER_SIZE - 10); i++) {
                response.response_buf[i] = '0' + i%10;
            }
            response.response_len = ATCI_UART_TX_FIFO_BUFFER_SIZE - 10;
            response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            response.response_flag |= ATCI_RESPONSE_FLAG_URC_FORMAT;
            atci_send_response(&response);
            param = NULL;
            break;
        default :
            //strcpy((char *)response.response_buf, "ERROR\r\n");
            response.response_len = strlen((char *)response.response_buf);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR; // ATCI will help append "ERROR" at the end of response buffer
            atci_send_response(&response);
            break;
    }
    return ATCI_STATUS_OK;
}
#endif /* ATCI_TEST_AT_COMMAND_DISABLE */


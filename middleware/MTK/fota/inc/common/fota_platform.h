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


#ifndef __FOTA_LOG_H__
#define __FOTA_LOG_H__


#include <stdint.h>
#include "hal_flash.h"


#ifndef UNUSED
#define UNUSED(n)
#endif

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

#ifndef ALIGN
#define ALIGN(n)  __attribute__((aligned((n))))
#endif

#if defined(MOD_CFG_FOTA_BL_RESERVED) || defined(BL_FOTA_ENABLE)
/* Running in the bootloader project */
#define FOTA_BL_ENABLE
#endif

#if defined(MOD_CFG_FOTA_BL_RESERVED) || (defined(BL_FOTA_ENABLE) && defined(BL_FOTA_DEBUG))
/* Enable fota log in the bootloader project */
#define FOTA_BL_DEBUG_ENABLE
#endif

#if (!defined(FOTA_BL_ENABLE) && !defined(MTK_DEBUG_LEVEL_NONE)) || defined(FOTA_BL_DEBUG_ENABLE)
/* Enable FOTA log */
#define FOTA_LOG_ENABLE    (1)
#else
/* Disable FOTA log */
#define FOTA_LOG_ENABLE    (0)
#endif

/* Enable FOTA_LOG_D logs */
#define FOTA_DEBUG_LOG_ENABLE   (0)

#if FOTA_LOG_ENABLE
#ifdef FOTA_BL_ENABLE
/* Used in bootloader project */
#ifdef MTK_FOTA_ON_7687
#include "hw_uart.h"
#define FOTA_LOG_I(fmt, args...)    hw_uart_printf(fmt"\r\n", ##args)
#define FOTA_LOG_W(fmt, args...)    hw_uart_printf(fmt"\r\n", ##args)
#define FOTA_LOG_E(fmt, args...)    hw_uart_printf(fmt"\r\n", ##args)
#if FOTA_DEBUG_LOG_ENABLE
#define FOTA_LOG_D(fmt, args...)    hw_uart_printf(fmt"\r\n", ##args)
#else
#define FOTA_LOG_D(fmt, args...)
#endif

#define FOTA_LOG_MSGID_I(fmt,cnt,arg...)    hw_uart_printf(fmt"\r\n", ##arg)
#define FOTA_LOG_MSGID_W(fmt,cnt,arg...)    hw_uart_printf(fmt"\r\n", ##arg)
#define FOTA_LOG_MSGID_E(fmt,cnt,arg...)    hw_uart_printf(fmt"\r\n", ##arg)
#if FOTA_DEBUG_LOG_ENABLE
#define FOTA_LOG_MSGID_D(fmt,cnt,arg...)    hw_uart_printf(fmt"\r\n", ##arg)
#else
#define FOTA_LOG_MSGID_D(fmt,cnt,arg...)
#endif

#define DEBUG_ASSERT(x)    {if (!(x)) {hw_uart_printf("assert\r\n"); while (1); } }
#else
#include "bl_common.h"
#define FOTA_LOG_I(fmt, ...)    bl_print(LOG_DEBUG, fmt"\r\n", ##__VA_ARGS__)
#define FOTA_LOG_W(fmt, ...)    bl_print(LOG_DEBUG, fmt"\r\n", ##__VA_ARGS__)
#define FOTA_LOG_E(fmt, ...)    bl_print(LOG_DEBUG, fmt"\r\n", ##__VA_ARGS__)
#if FOTA_DEBUG_LOG_ENABLE
#define FOTA_LOG_D(fmt, ...)    bl_print(LOG_DEBUG, fmt"\r\n", ##__VA_ARGS__)
#else
#define FOTA_LOG_D(fmt, ...)
#endif

#define FOTA_LOG_MSGID_I(fmt,cnt,arg...)    bl_print(LOG_DEBUG, fmt"\r\n", ##arg)
#define FOTA_LOG_MSGID_W(fmt,cnt,arg...)    bl_print(LOG_DEBUG, fmt"\r\n", ##arg)
#define FOTA_LOG_MSGID_E(fmt,cnt,arg...)    bl_print(LOG_DEBUG, fmt"\r\n", ##arg)
#if FOTA_DEBUG_LOG_ENABLE
#define FOTA_LOG_MSGID_D(fmt,cnt,arg...)    bl_print(LOG_DEBUG, fmt"\r\n", ##arg)
#else
#define FOTA_LOG_MSGID_D(fmt,cnt,arg...)
#endif

#define DEBUG_ASSERT(x)    {if (!(x)) {bl_print(LOG_DEBUG, "assert\r\n"); while (1); } }
#endif
#else /* FOTA_BL_ENABLE */
/* Used in CM4 project */
#include "syslog.h"
#include "FreeRTOSConfig.h"

#define FOTA_LOG_I(fmt, ...)    LOG_I(fota, (fmt), ##__VA_ARGS__)
#define FOTA_LOG_W(fmt, ...)    LOG_W(fota, (fmt), ##__VA_ARGS__)
#define FOTA_LOG_E(fmt, ...)    LOG_E(fota, (fmt), ##__VA_ARGS__)
#if FOTA_DEBUG_LOG_ENABLE
#define FOTA_LOG_D(fmt, ...)    LOG_I(fota, (fmt), ##__VA_ARGS__)
#else
#define FOTA_LOG_D(fmt, ...)
#endif

#define FOTA_LOG_MSGID_I(fmt,cnt,arg...)    LOG_MSGID_I(fota,fmt,cnt,##arg)
#define FOTA_LOG_MSGID_W(fmt,cnt,arg...)    LOG_MSGID_W(fota,fmt,cnt,##arg)
#define FOTA_LOG_MSGID_E(fmt,cnt,arg...)    LOG_MSGID_E(fota,fmt,cnt,##arg)
#if FOTA_DEBUG_LOG_ENABLE
#define FOTA_LOG_MSGID_D(fmt,cnt,arg...)    LOG_MSGID_I(fota,fmt,cnt,##arg)
#else
#define FOTA_LOG_MSGID_D(fmt,cnt,arg...)
#endif

#define DEBUG_ASSERT(x)    configASSERT(x)
#endif /* FOTA_BL_ENABLE */
#else /* FOTA_LOG_ENABLE */
#define FOTA_LOG_I(fmt, args...)
#define FOTA_LOG_W(fmt, args...)
#define FOTA_LOG_E(fmt, args...)
#define FOTA_LOG_D(fmt, args...)

#define FOTA_LOG_MSGID_I(fmt,cnt,arg...)
#define FOTA_LOG_MSGID_W(fmt,cnt,arg...)
#define FOTA_LOG_MSGID_E(fmt,cnt,arg...)
#define FOTA_LOG_MSGID_D(fmt,cnt,arg...)

#ifdef FOTA_BL_ENABLE
#define DEBUG_ASSERT(x)    {if (!(x)) { while (1); } }
#else
#include "FreeRTOSConfig.h"
#define DEBUG_ASSERT(x)    configASSERT(x)
#endif
#endif /* FOTA_LOG_ENABLE */

#endif /* __FOTA_LOG_H__ */


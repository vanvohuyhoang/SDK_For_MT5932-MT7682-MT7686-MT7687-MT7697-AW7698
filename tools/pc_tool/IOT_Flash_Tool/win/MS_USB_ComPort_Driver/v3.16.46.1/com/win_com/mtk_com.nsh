/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2012
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*******************************************************************************
 * Filename:
 * ---------
 *   mtk_com.nsh
 *
 ****************************************************************************/

;+------------------------------------------------------------------------------
; Version definition
;
!define MAJOR_VER               3  
!define MINOR_VER               16  
!define MICRO_VER               46
!define BUILD_VER               1
!define VERSION                 "${MAJOR_VER}.${MINOR_VER}.${MICRO_VER}.${BUILD_VER}"
!define HOTFIX_INTERNAL_URL     1   ; 1: prompt internal hotfix URL

;+------------------------------------------------------------------------------
; USB HW ID definition
;
!define VID                       "0e8d"    
!define VENDOR                  "MediaTek"

;+------------------------------------------------------------------------------
; INF file name definition
;
!define COM_INF_FILE_NAME		 "usb2ser"
!define MODEM_INF_FILE_NAME		 "modem"
!define HWID_START_TOKEN   "USB\VID"
!define HWID_END_TOKEN     "["


;+------------------------------------------------------------------------------
; General
;
!define UNINSTALLER             "UninstallDriver.exe"
!define PACKAGE                 "COM_Driver"

;+------------------------------------------------------------------------------
; ETW log utility parameters
;
!define ETW_UTIL_EXE           "mtk_etw_log.exe"
!define ETW_UNINST_EXE         "UninstallLog.exe"
!define ETW_LOGGER_NAME        "COM_LOG"    
!define ETW_MAX_FILE_SIZE      "256"
!define ETW_FILE_MAX           "3"
!define ETW_GUID               "8ffa488b-07d9-4ef5-b1b2-a0bea188dc1b" 
!define ETW_ENABLE_LEVEL       "4"
!define ETW_ENABLE_FLAGS       "0xffffff"
!define ETW_VENDOR             ${VENDOR}

!define ETW_INST_DIR           "$PROGRAMFILES\MediaTek\${ETW_VENDOR} ${ETW_LOGGER_NAME}"



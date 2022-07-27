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
 *   log.nsh
 *
 ****************************************************************************/

!ifndef LOG_NSH
!define LOG_NSH

!include "time.nsh"
 
!define INST_LOG_PATH   "$SYSDIR\LogFiles\${VENDOR}\${PACKAGE}_Install"
!define INST_LOG_NAME   "install.log"
!define UNINST_LOG_NAME "uninstall.log"

var cur_time

;+------------------------------------------------------------------------------
; LogInit
;
!define LogInit     `!insertmacro LogInit`

!macro LogInit
    CreateDirectory "${INST_LOG_PATH}"
!ifdef __UNINSTALL__
    Delete "${INST_LOG_PATH}\${UNINST_LOG_NAME}"
!else
    Delete "${INST_LOG_PATH}\${INST_LOG_NAME}"
!endif
!macroend

;+------------------------------------------------------------------------------
; LogPrint
;
!define LogPrint    `!insertmacro LogPrint`
!macro LogPrint _text 
    ${time::GetLocalTime} $cur_time
    DetailPrint "${_text}"  
    IfFileExists "${INST_LOG_PATH}\*.*" 0 +2
!ifdef __UNINSTALL__
    nsislog::log "${INST_LOG_PATH}\${UNINST_LOG_NAME}" "[$cur_time, ${__FILE__}, ${__LINE__}]$\t${_text}"
!else
    nsislog::log "${INST_LOG_PATH}\${INST_LOG_NAME}" "[$cur_time, ${__FILE__}, ${__LINE__}]$\t${_text}"
!endif
  	${time::Unload}
!macroend

!endif

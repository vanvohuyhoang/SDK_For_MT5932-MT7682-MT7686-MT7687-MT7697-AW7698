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
 *   hotfix.nsh
 *
 ****************************************************************************/

!ifndef HOTFIX_NSH
!define HOTFIX_NSH

!include "strstr.nsh"

; microsoft hostfix
!define HOTFIX_KEY                      "Software\Microsoft\Windows NT\CurrentVersion\HotFix\"
!define HOTFIX_KEY_WIN7                 "SOFTWARE\Microsoft\Windows\CurrentVersion\Component Based Servicing\Packages"
!define HOTFIX_RNDIS_COMPO              "KB901122"  ; http://support.microsoft.com/kb/901122
!define HOTFIX_USBSER_IAD               "KB918365"  ; http://support.microsoft.com/kb/918365
!define HOTFIX_RNDIS_SURPRISE_REMOVE    "KB959765"  ; http://support.microsoft.com/kb/959765
!define HOTFIX_RNDIS_YELLOW_BANG        "KB2471472" ; http://support.microsoft.com/kb/2471472

; mtk internal hotfix url
!define MTK_MS_HOTFIX_URL               "http://teams.mediatek.inc/sites/LTE/LTE%20Public/Forms/AllItems.aspx?RootFolder=%2fsites%2fLTE%2fLTE%20Public%2fSoftware%20Release%2fHost%20Utility%2fMicrosoft%20Hotfix&FolderCTID=0x01200066E52970ECF41741B3BE7877A5A9299F&View=%7bDDB14017%2dBB98%2d4C71%2dA907%2dCE77538B16E4%7d"

;+------------------------------------------------------------------------------
; * Function
;     IsXpSp2WoHotFixKB901122
;
; * Description 
;     http://support.microsoft.com/kb/901122
;
; * Input
;     None
;
; * Return
;     the return value is stored in the stack
;     1: hotfix installation is required
;     0: hotfix installation is not required
;
Function IsXpSp2WoHotFixKB901122
    ${If} ${IsWinXP}             ; xp x64 sp1
    ${AndIf} ${RunningX64}       ; no hotfix for rndis driver in an 
    ${AndIf} ${IsServicePack} 1  ; USB composite device  
        MessageBox MB_OK "Please upgrade to XP64 SP2 first" 
        Goto exit
    ${EndIf}

    ${If} ${IsWinXP} 
    ${AndIfNot} ${RunningX64}                
    ${AndIfNot} ${AtLeastServicePack} 3
        StrCpy $0 0
loop:
        EnumRegKey $1 HKLM '${HOTFIX_KEY}' $0
        ;DetailPrint "EnumRegKey $1"
        StrCmp $1 "" hf_not_found
        StrCmp $1 ${HOTFIX_RNDIS_COMPO} hf_found ; hotfix found
        IntOp $0 $0 + 1
        Goto loop
    
hf_not_found:
        MessageBox MB_OK "Please install hotfix KB901122 first"
        Push 1
        Goto exit
    ${EndIf}

hf_found:
    Push 0
    Goto exit

exit:
FunctionEnd

;+------------------------------------------------------------------------------
; * Function
;     IsXpSp2WoHotFixKB918365
;
; * Description 
;     http://support.microsoft.com/kb/918365
;
; * Input
;     None
;
; * Return
;     the return value is stored in the stack
;     1: hotfix installation is required
;     0: hotfix installation is not required
;
Function IsXpSp2WoHotFixKB918365
    ${If} ${IsWinXP} 
    ${AndIfNot} ${RunningX64}                
    ${AndIfNot} ${AtLeastServicePack} 3
        StrCpy $0 0
loop:
        EnumRegKey $1 HKLM '${HOTFIX_KEY}' $0
        ;DetailPrint "EnumRegKey $1"
        StrCmp $1 "" hf_not_found
        StrCmp $1 ${HOTFIX_USBSER_IAD} hf_found ; hotfix found
        IntOp $0 $0 + 1
        Goto loop
    
hf_not_found:
        MessageBox MB_OK "Please install hotfix KB918365 first" 
        Push 1
        Goto exit
    ${EndIf}

hf_found:
    Push 0
    Goto exit

exit:
FunctionEnd

;+------------------------------------------------------------------------------
; * Function
;     IsXpWoHotFixKB959765
;
; * Description 
;     http://support.microsoft.com/kb/959765
;
; * Input
;     None
;
; * Return
;     the return value is stored in the stack
;     1: hotfix installation is required
;     0: hotfix installation is not required
;
Function IsXpWoHotFixKB959765
    ${If} ${IsWinXP} 
    ${AndIfNot} ${RunningX64}                
        StrCpy $0 0
loop:
        EnumRegKey $1 HKLM '${HOTFIX_KEY}' $0
        ;DetailPrint "EnumRegKey $1"
        StrCmp $1 "" hf_not_found
        StrCmp $1 ${HOTFIX_RNDIS_SURPRISE_REMOVE} hf_found ; hotfix found
        IntOp $0 $0 + 1
        Goto loop
    
hf_not_found:
        MessageBox MB_OK "Please install hotfix KB959765 first" 
        Push 1
        Goto exit
    ${EndIf}

hf_found:
    Push 0
    Goto exit

exit:
FunctionEnd

;+------------------------------------------------------------------------------
; * Function
;     IsWin72WoHotFixKB2471472
;
; * Description 
;     http://support.microsoft.com/kb/2471472
;
; * Input
;     None
;
; * Return
;     the return value is stored in the stack
;     1: hotfix installation is required
;     0: hotfix installation is not required
;
Function IsWin7WoHotFixKB2471472
    ${If} ${IsWin7} 
        StrCpy $0 0

    ; x64 ?    
	ReadRegStr $0 HKLM \
            "System\CurrentControlSet\Control\Session Manager\Environment" \
            "PROCESSOR_ARCHITECTURE"
	StrCmp $0 'x86' +2
    SetRegView 64

loop:
        EnumRegKey $1 HKLM '${HOTFIX_KEY_WIN7}' $0
        ;DetailPrint "EnumRegKey $1"
        StrCmp $1 "" hf_not_found   ; all searched, not found
        Push $1
        Push ${HOTFIX_RNDIS_YELLOW_BANG}     
        ${StrStr} 
        Pop $2
        ;${StrStr} $2 $1 ${HOTFIX_RNDIS_YELLOW_BANG} 
        StrCmp $2 "" next           ; search next
        Goto hf_found               ; hotfix found
next:
        IntOp $0 $0 + 1
        Goto loop    

hf_not_found:
        Push 1
        MessageBox MB_OK "Please install hotfix KB2471472 first" 
        Goto exit
    ${EndIf}

hf_found:
    Push 0
    Goto exit

exit:
    SetRegView 32
FunctionEnd

;+------------------------------------------------------------------------------
; * Function
;     CheckHotFix
;
; * Description 
;     check if the required hotfixes are all installed
;
; * Input
;     None
;
; * Return
;     the return value is stored in the stack
;     1: error 
;     0: no error
;
Function CheckHotFix

    Call IsXpSp2WoHotFixKB901122
    Pop $3
    IntCmpU $3 1 hf1_required
    Goto hf2
hf1_required:
    Push 1
    Pop  $4
    
hf2:    
    Call IsXpSp2WoHotFixKB918365
    Pop $3
    IntCmpU $3 1 hf2_required
    Goto hf3
hf2_required:
    Push 1
    Pop  $4
    
hf3:
    Call IsXpWoHotFixKB959765
    Pop $3
    IntCmpU $3 1 hf3_required
    Goto hf4
hf3_required:
    Push 1
    Pop  $4
    
hf4:
    Call IsWin7WoHotFixKB2471472
    Pop $3
    IntCmpU $3 1 hf4_required
    Goto exit

hf4_required:
    Push 1
    Pop  $4

exit:
    Push $4
    
    ; see if the hotfix installation path should be prompted
    Push $4
    Pop $3
    IntCmpU $3 1 hot_fix_required
    goto hot_fix_installed
hot_fix_required:
!ifdef HOTFIX_INTERNAL_URL
    ExecShell "" "iexplore.exe" ${MTK_MS_HOTFIX_URL} 
!endif
hot_fix_installed:
FunctionEnd

!endif


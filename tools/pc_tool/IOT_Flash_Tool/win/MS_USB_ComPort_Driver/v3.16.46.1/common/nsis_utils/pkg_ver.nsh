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
 *   pkg_ver.nsh
 *
 ****************************************************************************/

!ifndef PKG_VER_NSH
!define PKG_VER_NSH

!include "strstr.nsh"
!include "param.nsh"
!include "log.nsh"

!define REG_SW_UNINST "Software\Microsoft\Windows\CurrentVersion\Uninstall\"
!define KEY_DEPENDENCY_COUNT "DepCnt"
!define KEY_DEPENDENCY_NAME  "DepName_"

!define DEP_PREFIX "DEP_"

Var cur_ver

;+------------------------------------------------------------------------------
; * Function
;     AddDependency
;
; * Description
;     If dependency is specified in the command line parameter by "/ADD_DEP=XXX",
;     this dependency should be written in the software registry key
;
; * Input
;     package name
;     vendor name
;
; * Return
;
Function AddDependency
    POP $R5 ; vendor name
    POP $R6 ; package name

    ; /ADD_DEP=xxx in the command line parameter ? 
    Push "ADD_DEP"
    Push ""
    ${GetParameterValue}
    Pop $2
    StrCmp $2 "" ad_exit
    WriteRegStr HKLM "SOFTWARE\$R5\$R6" "${DEP_PREFIX}$2" "1" 
ad_exit:
FunctionEnd

;+------------------------------------------------------------------------------
; * Function
;     DelSwUninstEntry
;
; * Description
;     Delete the software unisntall entry in "Add / Remove Programs" panel
;
; * Input
;     package name
;     vendor name
;
; * Return
;
Function DelSwUninstEntry
    POP $R5 ; vendor name
    POP $R6 ; package name
    ${LogPrint} "Deleting ${REG_SW_UNINST}$R5_$R6"
    DeleteRegKey HKLM "${REG_SW_UNINST}$R5_$R6"
FunctionEnd

;+------------------------------------------------------------------------------
; * Function
;     InstallPackage
;
; * Description
;     Install the package with
;       - Version checking
;         Installation will be cancelled if the same version has already 
;         been installed. 
;         If a different version has already been installed, users will 
;         be prompted to remove it.
;       - Registries creation
;         version information
;         uninstaller path
;       - Shortcut creation
;         Creates shortcut to the start menu
;       - Uninstaller creation
;         Write uninstaller
;
; * Input
;     uninstaller name
;     package name
;     vendor name
;     version
;
; * Return
;     1: the package is installed
;     0: the package is NOT installed 
;
Function InstallPackage
    POP $R4 ; version (a.b.c.d)
    POP $R5 ; vendor name
    POP $R6 ; package name
    POP $R7 ; uninstaller name

    ; get current version and compare
    ${LogPrint} "Get current version and compare it with new version"
	${LogPrint} "SOFTWARE\$R5\$R6 new version"
    ReadRegStr $cur_ver HKLM "SOFTWARE\$R5\$R6" "Version"
	${LogPrint} "$cur_ver"
    StrCmp "" $cur_ver ip_uninst
    StrCmp $R4 $cur_ver ip_alrdy_inst ; the same version

ip_uninst:
    ; get uninstall string
    ReadRegStr $R0 HKLM "${REG_SW_UNINST}${VENDOR}_$R6" "UninstallString"
    StrCmp $R0 "" ip_install ; no un-installer
    
    ; uninstall previous version
    ${LogPrint} "Uninstalling v$cur_ver"     
    ReadRegStr $R1 HKLM "${REG_SW_UNINST}${VENDOR}_$R6" "UninstallerPath"
	${LogPrint} "Begin Uninstall $R0; $R5; $R6; $R1"
    ExecWait '$R0 /UPDATE=1 /S /Vendor=$R5 /Package=$R6 _?=$R1'

    Goto ip_install

ip_install:
    ; write new version
    ${LogPrint} "Writing new version $R4"
    WriteRegStr HKLM "SOFTWARE\$R5\$R6" "Version" $R4        

    ; write package inforamtion
    ${LogPrint} "Writing package information to ${REG_SW_UNINST}"
    WriteRegStr HKLM "${REG_SW_UNINST}$R5_$R6" "UninstallString" '$INSTDIR\$R7'
    WriteRegStr HKLM "${REG_SW_UNINST}$R5_$R6" "UninstallerPath" '$INSTDIR'
    WriteRegStr HKLM "${REG_SW_UNINST}$R5_$R6" "DisplayName"     '$R5 $R6 $R4'
    WriteRegStr HKLM "${REG_SW_UNINST}$R5_$R6" "DisplayIcon"     '$INSTDIR\${UNINSTALLER}'
    WriteRegStr HKLM "${REG_SW_UNINST}$R5_$R6" "DisplayVersion"  '$R4'

!if 0
    ; create shortcut to programs menu
    ${LogPrint} "Creating shortcut in startup menu"
    CreateDirectory "$SMPROGRAMS\$R5 $R6 $R4"
    CreateShortCut "$SMPROGRAMS\$R5 $R6 $R4\Uninstall.lnk" \
                    "$INSTDIR\$R7" "" "$INSTDIR\$R7" 0
!endif
    CreateDirectory $INSTDIR

    ; write uninstaller
    ${LogPrint} "Writing uninstaller"
    WriteUninstaller "$R7"
   
    goto ip_installed

ip_alrdy_inst:
    Push 0
    goto ip_end
ip_installed:
    Push 1
    goto ip_end
ip_end:

FunctionEnd

;+------------------------------------------------------------------------------
; * Function
;     un.IfAnUpdateUninstall
;
; * Description
;     If this uninstallation is requested due to an update 
;
; * Input
;
; * Return
;     in the top of stack
;       "1": an update uninstallation request
;       "0": not an update unisntallation request
;
Function un.IfAnUpdateUninstall
    ; /UPDATE=1 in the command line parameter ?
    Push "UPDATE"   
    Push "0"
    ${GetParameterValue}
FunctionEnd

;+------------------------------------------------------------------------------
; * Function
;     un.DeleteDependency
;
; * Description
;     If /DEL_DEP is specified in the command line parameters, delete the 
;     dependency
;
; * Input
;
; * Return
;
Function un.DeleteDependency
    POP $R5 ; vendor name
    POP $R6 ; package name

    ;/DEL_DEP=XXX in the command line parameter ?
    Push "DEL_DEP"
    Push "0"
    ${GetParameterValue}
    Pop $0
    DeleteRegValue HKLM "SOFTWARE\$R5\$R6" ${DEP_PREFIX}$0
FunctionEnd


;+------------------------------------------------------------------------------
; * Function
;     un.CheckDependency
;
; * Description
;     If dependency exists 
;
; * Input
;     package name
;     vendor name
;
; * Return
;     in the top of stack
;       "": dependency string not found
;       "XXX" dependency string found
;
Function un.CheckDependency
    POP $R5 ; vendor name
    POP $R6 ; package name

    ; check if there are still DEP_XXX in the registry key
    ; if yes, no uninstallation should be perforked    
    StrCpy $0 0
id_loop:
    ClearErrors
    EnumRegValue $1 HKLM "SOFTWARE\$R5\$R6" $0
    StrCmp $1 "" no_more_value

    ; find if the key contains "DEP_"
    Push $1         
    Push ${DEP_PREFIX}
    ${StrStr}
    Pop $2
    StrCmp $2 "" id_enum_next

    /* dependency found */
    Goto dep_found
id_enum_next:
    IntOp $0 $0 + 1
    Goto id_loop  

dep_found:
    Push $2
    Goto id_exit
no_more_value:
    Push ""
id_exit:
FunctionEnd

;+------------------------------------------------------------------------------
; * Function
;     UninstallPackage
;
; * Description
;     Uninstall the package with
;       - Registries deletion
;         version information
;         uninstaller path
;       - Shortcut deletion
;         Delete shortcut to the start menu
;       - Uninstaller deletion
;         Delete uninstaller
;
; * Input
;       uninstaller name
;           "UninstallDriver.exe"
;       package name
;           "RemoteNDIS"
;       vendor name
;           "MediaTek"
;       version
;           "1.2.3.4"
;       an update uninstallation request?
;           "0" or "1"
;
; * Return
;
Function un.UninstallPackage
    POP $R3 ; update uninstallation?
    POP $R4 ; version
    POP $R5 ; vendor name
    POP $R6 ; package name
    POP $R7 ; uninstaller name

    StrCmp $R3 "1" update_uninst
    goto normal_uninst
update_uninst:
    ; delete registry value "Version", and leave all DEP_XXX registry 
    ; key values
    ${LogPrint} "Leave all DEP_XXX"
    DeleteRegValue HKLM "SOFTWARE\$R5\$R6" "Version" 
    goto del_uninst_reg_key

normal_uninst:
    ; remove software registry key
    ${LogPrint} "Deleting SOFTWARE\$R5\$R6"
    DeleteRegKey HKLM "SOFTWARE\$R5\$R6"

del_uninst_reg_key:
    ; remove uninstall registry key
	StrCpy $1 0
	ClearErrors
    ${LogPrint} "Deleting ${REG_SW_UNINST}$R5_$R6"
    DeleteRegKey HKLM "${REG_SW_UNINST}$R5_$R6"
	IfErrors del_uninst_reg_key_repeat del_uninst_reg_key_finish
	
del_uninst_reg_key_repeat:
	; repeat remove uninstall registry key
	ClearErrors
	Sleep 100
	${LogPrint} "Trying Deleting ${REG_SW_UNINST}$R5_$R6 again"
	DeleteRegKey HKLM "${REG_SW_UNINST}$R5_$R6"
	IntOp $1 $1 + 1
	StrCmp $1 50 del_uninst_reg_key_unfinish
	IfErrors del_uninst_reg_key_repeat del_uninst_reg_key_finish
    
del_uninst_reg_key_unfinish:
	${LogPrint} "Delete ${REG_SW_UNINST}$R5_$R6 failed! Please do it manually"
	
del_uninst_reg_key_finish:
    ; delete uninstaller
    ${LogPrint} "Deleting $INSTDIR\$R7"
    Delete $INSTDIR\$R7
    
    ; remove install directory
    ${LogPrint} "Removing $INSTDIR"
    RMDir $INSTDIR
    
!if 0
    ; remove start menu shortcut
    Delete "$SMPROGRAMS\$R5 $R6 $R4\Uninstall.lnk" 
    
    ; remove start menu directory
    RMDir  "$SMPROGRAMS\$R5 $R6 $R4"
!endif
FunctionEnd

!define LVM_GETITEMCOUNT 0x1004
!define LVM_GETITEMTEXT 0x102D

!define DumpLog "!insertmacro DumpLog"
!macro DumpLog
  Exch $5
  Push $0
  Push $1
  Push $2
  Push $3
  Push $4
  Push $6

  FindWindow $0 "#32770" "" $HWNDPARENT
  GetDlgItem $0 $0 1016
  StrCmp $0 0 error
  FileOpen $5 $5 "w"
  StrCmp $5 0 error
    SendMessage $0 ${LVM_GETITEMCOUNT} 0 0 $6
    System::Alloc ${NSIS_MAX_STRLEN}
    Pop $3
    StrCpy $2 0
    System::Call "*(i, i, i, i, i, i, i, i, i) i \
      (0, 0, 0, 0, 0, r3, ${NSIS_MAX_STRLEN}) .r1"
    loop: StrCmp $2 $6 done
      System::Call "User32::SendMessageA(i, i, i, i) i \
        ($0, ${LVM_GETITEMTEXT}, $2, r1)"
      System::Call "*$3(&t${NSIS_MAX_STRLEN} .r4)"
      FileWrite $5 "$4$\r$\n"
      IntOp $2 $2 + 1
      Goto loop
    done:
      FileClose $5
      System::Free $1
      System::Free $3
      Goto exit
  error:
    MessageBox MB_OK error
  exit:
    Pop $6
    Pop $4
    Pop $3
    Pop $2
    Pop $1
    Pop $0
    Exch $5
!macroend

!endif

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
 *   mtk_com.nsi
 *
 ****************************************************************************/

!addincludedir ..\..\common\nsis_utils

!include "x64.nsh" 
!include "winver.nsh" 
!include "mtk_com.nsh"
;!include "hotfix.nsh"
!include "pkg_ver.nsh"
!include "drv_inst.nsh"

!define DRV_PATH		"driver\v3.1646.1"
!define INF_PATH        "$INSTDIR\${DRV_PATH}"

!define CONFIG_INI		$INSTDIR\${INI}

Var updt_uninst

;==============================================================================

LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"

VIProductVersion "${VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${VENDOR} ${PACKAGE} Installer"
VIAddVersionKey /LANG=${LANG_ENGLISH} "Comments" ""
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "${VENDOR}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalTrademarks" ""
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" ""
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" ""
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${VERSION}"

;==============================================================================

Name "${VENDOR} ${PACKAGE} ${MAJOR_VER}.${MINOR_VER}.${MICRO_VER}.${BUILD_VER}"

OutFile "InstallDriver.exe"

;==============================================================================

InstallDir "$PROGRAMFILES\${VENDOR}\${VENDOR} ${PACKAGE}"

RequestExecutionLevel admin

;==============================================================================

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "MediaTek COM Driver"
    SectionIn RO
	
	; initialize log
    ${LogInit}
    
	${LogPrint} '$SYSDIR'
	${LogPrint} '$WINDIR'
	${LogPrint} '$PROGRAMFILES'
	
    ; check hotfix installation
    ; ${LogPrint} "Checking hotfix"
    ; Call CheckHotFix
    ; Pop $3
    ; IntCmpU $3 1 install_end ; request user to install hotfix first
    ;	
    ; install package
    ${LogPrint} "Installing driver package"
    Push ${UNINSTALLER}
    Push "${PACKAGE}"
    Push ${VENDOR}
    Push ${VERSION}
    Call InstallPackage
    Pop $3 ; 0: not installed, 1: installed
    IntCmpU $3 0 install_end 
	
	; add dependency if specified in the command parameters
    Push ${PACKAGE}
    Push ${VENDOR}
    Call AddDependency
	
	; write etw utility
    SetOutPath $InstDir
    File /oName=${ETW_UTIL_EXE} ..\..\common\nsis_utils\${ETW_UTIL_EXE}
    
    ; install driver
    CreateDirectory ${INF_PATH}
	CreateDirectory "${INF_PATH}\x86"
	CreateDirectory "${INF_PATH}\x64"
	
	SetOutPath $INSTDIR	
	
	; install driver package
    SetOutPath ${INF_PATH}    
    File "${DRV_PATH}\${COM_INF_FILE_NAME}.inf"
    File "${DRV_PATH}\${COM_INF_FILE_NAME}.cat"
	File "${DRV_PATH}\${MODEM_INF_FILE_NAME}.inf"
    File "${DRV_PATH}\${MODEM_INF_FILE_NAME}.cat"
	
	SetOutPath "${INF_PATH}\x86"
	File "${DRV_PATH}\x86\*"
	SetOutPath "${INF_PATH}\x64"
	File "${DRV_PATH}\x64\*"	
		
    ; write dev_remove.exe for uninstaller
    SetOutPath $InstDir
    
    ReadRegStr $0 HKLM \
            "System\CurrentControlSet\Control\Session Manager\Environment" \
            "PROCESSOR_ARCHITECTURE"
    StrCmp $0 'x86' proc_arch_x86
    Goto proc_arch_x64

proc_arch_x86:
    ${LogPrint} "x86"
    File "..\..\common\nsis_utils\dev_remove.exe"
	Goto preinstall_driver

proc_arch_x64:
    ${LogPrint} "x64"
    File /oName=dev_remove.exe "..\..\common\nsis_utils\dev_remove64.exe"
	File /oName=dev_inst.exe "..\..\common\nsis_utils\dev_inst64.exe"
	Goto preinstall_driver
	
preinstall_driver:

	; install etw utility
    ${LogPrint} "Install etw utility"
    ExecWait "$INSTDIR\${ETW_UTIL_EXE} /S /Vendor=${ETW_VENDOR} /LoggerName=${ETW_LOGGER_NAME} /MaxFileSize=${ETW_MAX_FILE_SIZE} /FileMax=${ETW_FILE_MAX} /Guid=${ETW_GUID} /EnableLevel=${ETW_ENABLE_LEVEL} /EnableFlags=${ETW_ENABLE_FLAGS} /D=$INSTDIR\.."
	
	Push ${INF_PATH}
	Push "${INF_PATH}\${COM_INF_FILE_NAME}.inf"
	Push ${HWID_START_TOKEN} 
	Push ${HWID_END_TOKEN}
	
	Call InstallUpgradeDriversInInf
	
	Push ${INF_PATH}
	Push "${INF_PATH}\${MODEM_INF_FILE_NAME}.inf"
	Push ${HWID_START_TOKEN}
	Push ${HWID_END_TOKEN}
	Call InstallUpgradeDriversInInf	
	
install_end:
SectionEnd

;==============================================================================

Section "Uninstall"

    ; initialize log
    ${LogInit}
	
	${LogPrint} "Uninstalling v${VERSION}"

	; delete dependency if specified in the command line parameter
    ${LogPrint} "Delete dependency"
    Push ${PACKAGE}
    Push ${VENDOR}
    call un.DeleteDependency
	
	; an update uninstallation request?
    ${LogPrint} "If an update uninstallation request?"
    Call un.IfAnUpdateUninstall
    Pop $updt_uninst
	${LogPrint} "update uninstallation=$updt_uninst"
    StrCmp $updt_uninst "1" remove_devices_in_inf

    ; dependency there ?
    ${LogPrint} "Check dependency"
    Push ${PACKAGE}
    Push ${VENDOR}
    Call un.CheckDependency
    Pop $6
    StrCmp $6 "" remove_devices_in_inf dep_found
    
dep_found:
    StrCpy $6 $6 "" 4 ;"DEP_"
    MessageBox MB_OK "Remove $6 first!"
    SetErrorLevel 3
    Goto uninst_sec_exit

remove_devices_in_inf:

	DetailPrint "Start remove MediaTek COM driver"
	Push "${INF_PATH}\${COM_INF_FILE_NAME}.inf"
	Push ${HWID_START_TOKEN} 
	Push ${HWID_END_TOKEN}
	
	Call un.RemoveDevicesInInf
	
	Push "${INF_PATH}\${MODEM_INF_FILE_NAME}.inf"
	Push ${HWID_START_TOKEN}
	Push ${HWID_END_TOKEN}
	Call un.RemoveDevicesInInf
	
	; uninstall etw utility
    ${LogPrint} "Uninstall etw utility"
    ExecWait '${ETW_INST_DIR}\${ETW_UNINST_EXE} /Vendor=${ETW_VENDOR} /LoggerName=${ETW_LOGGER_NAME} /S'
	
delete_files:   

    ; delete etw utility
    ${LogPrint} "Delete etw utility"
    Delete $INSTDIR\${ETW_UTIL_EXE} 
	
	; remove device_remove.exe
    Delete $INSTDIR\dev_remove.exe  
	IfFileExists $INSTDIR\dev_inst.exe 0 +2
	Delete $INSTDIR\dev_inst.exe
    
    ; uninstall usb2ser driver
    Push ${INF_PATH}
    Push ${INF_PATH}\${COM_INF_FILE_NAME}.inf
    Call un.UninstallDriver
	
	; uninstall modem driver
	Push ${INF_PATH}
    Push ${INF_PATH}\${MODEM_INF_FILE_NAME}.inf
    Call un.UninstallDriver
    
    ; delete inf, cat and inf directory
    Delete ${INF_PATH}\${COM_INF_FILE_NAME}.inf
	Delete ${INF_PATH}\${COM_INF_FILE_NAME}.cat
    Delete ${INF_PATH}\${MODEM_INF_FILE_NAME}.inf
    Delete ${INF_PATH}\${MODEM_INF_FILE_NAME}.cat
	Delete ${INF_PATH}\x86\*
	Delete ${INF_PATH}\x64\*
	RMDir ${INF_PATH}\x86
	RMDir ${INF_PATH}\x64
    RMDir ${INF_PATH}
	RMDir $INSTDIR\driver
	    
    ; remove the install package    
    Push ${UNINSTALLER}
    Push ${PACKAGE}
    Push ${VENDOR}
    Push ${VERSION}
	Push $updt_uninst
    call un.UninstallPackage
uninst_sec_exit:
SectionEnd

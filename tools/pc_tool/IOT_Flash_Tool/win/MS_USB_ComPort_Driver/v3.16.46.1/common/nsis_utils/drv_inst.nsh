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
 *   drv_inst.nsh
 *
 ****************************************************************************/

!include "log.nsh"

;
; UpdateDriverForPlugAndPlayDevices definitions
;
!define sysUpdateDriverForPlugAndPlayDevices    "newdev::UpdateDriverForPlugAndPlayDevices(i, t, t, i, *i) i"
!define ERROR_NO_SUCH_DEVINST   -536870389
 
;
; SetupCopyOEMInf definitions
;
!define sysSetupCopyOEMInf      "setupapi::SetupCopyOEMInf(t, t, i, i, i, i, *i, t) i"
!define SPOST_NONE              0
!define SPOST_PATH              1
!define SPOST_URL               2
!define SP_COPY_DELETESOURCE    0x1
!define SP_COPY_REPLACEONLY     0x2
!define SP_COPY_NOOVERWRITE     0x8
!define SP_COPY_OEMINF_CATALOG_ONLY 0x40000

!define ERROR_FILE_EXISTS       80
!define SUOI_FORCEDELETE        0x0001

;+------------------------------------------------------------------------------
; * Function
;       InstallUpgradeDriver
;
; * Description 
;       referenced from http://nsis.sourceforge.net/Driver_installation_and_update
;
; * Input
;       Push "c:\program files\yoursoftware\driver" 
;           -- the directory of the .inf file
;       Push "c:\program files\yoursoftware\driver\driver.inf"
;           -- the filepath of the .inf file (directory + filename)
;       Push "USB\VID_1234&PID_5678"
;           -- the HID (Hardware ID) of your device
;
Function InstallUpgradeDriver
 
    Pop $R0 ; HID
    Pop $R1 ; INFPATH
    Pop $R2 ; INFDIR

    ${LogPrint} "Installing driver for $R0"
    ${LogPrint} "INFPATH:$R1"

    ; x64 ?    
	ReadRegStr $0 HKLM \
            "System\CurrentControlSet\Control\Session Manager\Environment" \
            "PROCESSOR_ARCHITECTURE"
	StrCmp $0 'x86' proc_x86
    Goto proc_x64

proc_x86:
    ; Upgrade the driver if the device is already plugged in
    System::Get '${sysUpdateDriverForPlugAndPlayDevices}'
    Pop $0
    StrCmp $0 'error' lbl_noapi
    ${LogPrint} "Updating the driver..."
    ; 0, HID, INFPATH, 0, 0
    Push $INSTDIR ; Otherwise this function will swallow it, dunno why
    System::Call '${sysUpdateDriverForPlugAndPlayDevices}?e (0, R0, R1, 0, 0) .r0'
    Pop $1 ; last error
    Pop $INSTDIR
    IntCmp $0 1 lbl_done
    IntCmp $1 ${ERROR_NO_SUCH_DEVINST} lbl_notplugged
    
    ${LogPrint} "Driver update has failed. ($R3:$0,$1)"
    Goto lbl_noupgrade
lbl_notplugged:
    ${LogPrint} "The device is not plugged in, cannot update the driver."
    Goto lbl_noupgrade
lbl_noapi:
    ${LogPrint} "Your Windows $R3 doesn't support driver updates."
 
lbl_noupgrade:
    ; Pre-install the driver
    System::Get '${sysSetupCopyOEMInf}'
    Pop $0
    StrCmp $0 'error' lbl_inoapi
    ${LogPrint} "Installing the driver..."
    ; INFPATH, INFDIR, SPOST_PATH, "", 0, 0, 0, 0
    System::Call '${sysSetupCopyOEMInf}?e (R1, R2, ${SPOST_PATH}, 0, 0, 0, 0, 0) .r0'
    Pop $1 ; last error
    IntCmp $0 1 lbl_nodriver
    ${LogPrint} 'Driver pre-installation has failed with error #$1 ($R3)'
    Goto lbl_done
lbl_inoapi:
    ${LogPrint} "Your Windows $R3 doesn't support driver pre-installation."
lbl_nodriver:
    Goto lbl_done
proc_x64:   
	${LogPrint} 'x64 detected!'
	ExecWait '"$InstDir\dev_inst.exe" "$R0" "$R1"'
	Goto lbl_done
lbl_done:
FunctionEnd

Function un.RemoveDevice
    Pop $R1 ; HWID
     ${LogPrint} "Removing $R1"
    ExecWait '"$InstDir\dev_remove.exe" $R1'
FunctionEnd

Function un.RemoveMOD
    Pop $R1 ; VID
    Pop $R2 ; PID
    Pop $R3 ; REV
    DeleteRegKey HKLM "SYSTEM\CurrentControlSet\Control\UsbFlags\$R1$R2$R3"
FunctionEnd

Function un.UninstallDriver
    Pop $R1 ; INFPATH
    Pop $R2 ; INFDIR
    
    ${LogPrint} "Start driver un-installation ..."
    ${LogPrint} "INFPATH: $R1"
    ${LogPrint} "INFDIR: $R2"  
    
remove_oem:
    ; get oemN.inf
    ;
    System::Get 'setupapi::SetupCopyOEMInf(t, t, i, i, t, i, *i, t) i'
    Pop $0
    StrCmp $0 'error' uid_api_not_found1
    
    System::Call 'setupapi::SetupCopyOEMInf(t R1, t R2, i ${SPOST_PATH}, i ${SP_COPY_NOOVERWRITE}, t .R3, i ${NSIS_MAX_STRLEN}, *i 0, t 0) i .r0 ? e'
    Pop $1 ; last error
    IntCmp $0 1 oem_already_removed ; SetupCopyOEMInf success -> oem has already been remvoed
    IntCmp $1 ${ERROR_FILE_EXISTS} handle_oem_inf_name
    ${LogPrint} 'setupapi::SetupCopyOEMInf has failed with unexpected error #$1'
    Goto uid_exit
  
handle_oem_inf_name:
    StrCpy $R4 $R3 "" 15 ; reduce c:\windows\inf\oemN.inf to oemN.inf
    ${LogPrint} "oemN.inf: $R4"
    
    ; uninstall oem inf
    ;
    System::Get 'setupapi::SetupUninstallOEMInf(t, i, i) i'
    Pop $0
    StrCmp $0 'error' uid_api_not_found2
    
    ; uninstall driver
    ;
    ${LogPrint} "Uninstalling the driver..."
    System::Call 'setupapi::SetupUninstallOEMInf(t R4, i ${SUOI_FORCEDELETE},i 0) i .r0 ? e'
    Pop $1 ; last error
    IntCmp $0 1 uid_exit
    MessageBox MB_OK "Driver uninstallation has failed with error #$1"
    Goto uid_exit

oem_already_removed:
    ${LogPrint} "oem has already been removed"
    Goto remove_oem

uid_api_not_found1:
    ${LogPrint} "setupapi::SetupCopyOEMInf not found!"
    Goto uid_exit

uid_api_not_found2:
    ${LogPrint} "setupapi::SetupUninstallOEMInf not found!"
  
uid_exit: 
FunctionEnd

Var removing   ; first hwid is found, and thus the devcies removing is in progress
Var installing ; first hwid is found, and thus the devices installing is in progress
Var vidpid     ; vid and pid for removing device
Var mi         ; mi number for removing device

Function InstallUpgradeDriversInInf
    pop $r9 ; end token
    pop $r8 ; hwid start token
    pop $r7 ; inf file name
    pop $r6 ; inf path

    ${LogPrint} "==> InstallUpgradeDriversInInf, $r6, $r7, $r8, $r9"

    ; initialization
    Push 0
    Pop $installing
    Push ""
    Pop $vidpid
    Push ""
    Pop $mi

    ; open file
    ClearErrors
    FileOpen $9 $r7 r
    IfErrors "" iudi_next
    ${LogPrint} "Open $r7 failed"
    Goto iudi_done

iudi_next:
    ; read the file line by line
    ClearErrors
    FileRead $9 $8 
    IfErrors "" installing_check
    ${LogPrint} "FileRead failed"
    goto iudi_fclose
    ;${LogPrint} "line read: $8"

installing_check:
    IntCmp $installing 0 iudi_check_hwid_start
    
    ; end token ?    
    ;${LogPrint} "checking end token"
    Push $8
    Push $r9
    ${StrStr}
    Pop $7
    StrCmp $7 "" iudi_check_hwid_start 
    ${LogPrint} "end token $r9 found"
    Goto iudi_fclose

iudi_check_hwid_start:
    ; find the hwid start token
    ;${LogPrint} "Finding $r8 in $8"
    Push $8
    Push $r8
    ${StrStr}
    Pop $7
    StrCmp $7 "" iudi_next        
      
;    ${LogPrint} "start token [$r8] found"
    Push 1
    Pop $installing
    
    ; example hwid in an INF file: USB\VID_0E8D&PID_7106&REV_0001&MI_00
    StrCpy $vidpid $7 21 ; USB\VID_0E8D&PID_7106
;    ${LogPrint} "hwid: $7"

    ; get MI_XX
    Push $7
    Push "&MI"
    ${StrStr}
    Pop $6
    StrCmp $6 "" inst_updt_drvr
    StrCpy $mi $6 6 ; &MI_00. Remove possible <CR><LF>
    
inst_updt_drvr:    
    Push $r6
    Push $r7     
    Push "$vidpid$mi"
    Call InstallUpgradeDriver

    goto iudi_next

iudi_fclose:
    FileClose $9
iudi_done:
    ${LogPrint} "<== InstallUpgradeDriversInInf"
FunctionEnd

Function Un.RemoveDevicesInInf
    pop $r3 ; end token
    pop $r2 ; hwid start token
    pop $r1 ; inf file name

    ${LogPrint} "==> RemoveDevicesInInf, $r1, $r2, $r3"

    ; initialization
    Push 0
    Pop $removing
    Push ""
    Pop $vidpid
    Push ""
    Pop $mi

    ; open file
    ClearErrors
    FileOpen $9 $r1 r
    IfErrors "" next
    ${LogPrint} "Open $r1 failed"
    Goto done

next:
    ; read the file line by line
    ClearErrors
    FileRead $9 $8 
    IfErrors "" check_removing
    ${LogPrint} "FileRead failed"
    goto fclose
    ;${LogPrint} "line read: $8"

check_removing:
    IntCmp $removing 0 check_hwid_start
    
    ; end token ?    
    ;${LogPrint} "checking end token"
    Push $8
    Push $r3
    ${StrStr}
    Pop $7
    StrCmp $7 "" check_hwid_start 
    ${LogPrint} "end token $r3 found"
    Goto fclose

check_hwid_start:
    ; find the hwid start token
    Push $8
    Push $r2
    ${StrStr}
    Pop $7
    StrCmp $7 "" next        
      
;    ${LogPrint} "line read: $7"
    ${LogPrint} "start token [$r2] found"
    Push 1
    Pop $removing
    
    ; example hwid in an INF file: USB\VID_0E8D&PID_7106&REV_0001&MI_00
    StrCpy $vidpid $7 21 ; USB\VID_0E8D&PID_7106

    ; get MI_XX
    Push $7
    Push "&MI"
    ${StrStr}
    Pop $6
    StrCmp $6 "" rm_dev
    StrCpy $mi $6 6 ; &MI_00. Remove possible <CR><LF>
    
rm_dev:    
    ; remove device
    PUSH "$vidpid$mi"
    Call un.RemoveDevice
    goto next   ; next hwid

fclose:
    FileClose $9
done:
    ${LogPrint} "<== RemoveDevicesInInf"
FunctionEnd

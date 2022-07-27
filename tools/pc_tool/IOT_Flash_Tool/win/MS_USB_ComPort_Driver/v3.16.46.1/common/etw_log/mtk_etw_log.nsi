!addincludedir ..\common\nsis_utils

!include "x64.nsh" 
!include "winver.nsh" 
!include "mtk_etw_log.nsh"
!include "pkg_ver.nsh"

!define LOG_REG_MELS_CTRL_PATH       "SOFTWARE\${VENDOR}\Modem Driver Logger" 
!define LOG_REG_MELS_COMP_PATH       "SYSTEM\CurrentControlSet\Control\WMI\Autologger"

Var updt_uninst
Var os_vers_g
Var vendor_g
Var loggerName_g
Var serviceVersion_g
Var guid_g
Var level_g
Var flags_g
Var maxFileSize_g
Var sys_root_g

Var tmp_g

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

OutFile "mtk_etw_log.exe"

;==============================================================================

InstallDir "$PROGRAMFILES\${VENDOR}"

RequestExecutionLevel admin

;==============================================================================

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "MTK ETW Log"
    SectionIn RO

;init:
	${LogInit}

;init_variable:
    ; [ServiceVersion]
    IntOp $tmp_g ${MAJOR_VER} * 1000000
	IntOp $serviceVersion_g $serviceVersion_g + $tmp_g
    IntOp $tmp_g ${MINOR_VER} * 10000
	IntOp $serviceVersion_g $serviceVersion_g + $tmp_g
    IntOp $tmp_g ${MICRO_VER} * 100
	IntOp $serviceVersion_g $serviceVersion_g + $tmp_g
    IntOp $tmp_g ${BUILD_VER} * 1
	IntOp $serviceVersion_g $serviceVersion_g + $tmp_g

;check_os:
    ${IF} ${IsNT}
    ${AndIf} ${AtLeastWinXP}
        ${If} ${IsWinXP}
            StrCpy $os_vers_g "xp"
            ${LogPrint} "Running on Windows XP."
        ${Else}
            StrCpy $os_vers_g ">xp"
            ${LogPrint} "Running better than XP."
        ${EndIf}
    ${Else}
        ${LogPrint} "Unsupported OS version!"
        ${LogPrint} "Abort installation."
        goto complete
    ${EndIf}

;read_param:
    ; [SYSDIR]
    StrCpy $sys_root_g $SYSDIR

    ; [Vendor]
	StrCpy $vendor_g ${VENDOR}

    ; [LoggerName]
    ; /LoggerName=xxx in the command line parameter
    StrCpy $1 "LoggerName"
    Push $1
    Push ""
    call GetParameterValue_F
    Pop $loggerName_g
    StrCmp $loggerName_g "" read_param_fail

;set_instdir:
    strcpy $INSTDIR "$INSTDIR\$vendor_g $loggerName_g"

;install:
    ; install package
    ${LogPrint} "Installing Modem Driver Logger package"
    Push ${UNINSTALLER}
    Push $loggerName_g
    Push $vendor_g
    Push ${VERSION}
    Call InstallPackage
    Pop $3 ; 0: not installed, 1: installed
    IntCmpU $3 0 install_fail 

;add_dep:
    ; add dependency if specified in the command parameters
    Push ${PACKAGE}
    Push ${VENDOR}
    Call AddDependency

;silent_install_check:
    ; Silent installation if parameter /s is in the command line
    StrCpy $1 "s"
    Push $1
    Push "n/a"
    call GetParameterValue_F
    Pop $2
    StrCmp $2 "n/a" create_etl_dir

    ; Delete the software uninstall entry in Add/Remove Program panel
    Push $loggerName_g
    Push $vendor_g
    call DelSwUninstEntry

create_etl_dir:
    ${LogPrint} "Create ETL directory"
    CreateDirectory $sys_root_g\LogFiles\WMI\$vendor_g

;write_reg_autolog:
    ; Enable AutoLog
    StrCmp $os_vers_g "xp" write_reg_autolog_session    ; AutoLog start entry only set if os_vers is better than XP. (">xp")
    StrCpy $1 "Start"
    WriteRegDWORD HKLM "${LOG_REG_MELS_COMP_PATH}" $1 1

write_reg_autolog_session:
    ; [FileMax]
    ; /FileMax=xxx in the command line parameter
    StrCpy $1 "FileMax"
    Push $1
    Push ""
    call GetParameterValue_F
    Pop $2
    StrCmp $2 "" write_reg_fail
    WriteRegDWORD HKLM "${LOG_REG_MELS_COMP_PATH}\$vendor_g_$loggerName_g" $1 $2

    ; [MaxFileSize]
    ; /MaxFileSize=xxx in the command line parameter
    StrCpy $1 "MaxFileSize"
    Push $1
    Push ""
    call GetParameterValue_F
    Pop $maxFileSize_g
    StrCmp $maxFileSize_g "" write_reg_fail
    WriteRegDWORD HKLM "${LOG_REG_MELS_COMP_PATH}\$vendor_g_$loggerName_g" $1 $maxFileSize_g

    ; [Guid]
    ; /Guid=xxx in the command line parameter
    StrCpy $1 "Guid"
    Push $1
    Push ""
    call GetParameterValue_F
    Pop $guid_g
    StrCmp $guid_g "" write_reg_fail
    WriteRegStr HKLM "${LOG_REG_MELS_COMP_PATH}\$vendor_g_$loggerName_g" $1 {$guid_g}

    ; [FlushTimer]
    ; Default value is 1
    StrCpy $1 "FlushTimer"
    WriteRegDWORD HKLM "${LOG_REG_MELS_COMP_PATH}\$vendor_g_$loggerName_g" $1 1

    ; [LogFileMode]
    ; Default value is 0x8002
    ; EVENT_TRACE_USE_LOCAL_SEQUENCE | EVENT_TRACE_FILE_MODE_CIRCULAR
    StrCpy $1 "LogFileMode"
    WriteRegDWORD HKLM "${LOG_REG_MELS_COMP_PATH}\$vendor_g_$loggerName_g" $1 0x8002 

    ; [FileName]
    StrCpy $1 "FileName"
    StrCpy $2 "$sys_root_g\LogFiles\WMI\$vendor_g\$loggerName_g.etl"
    WriteRegStr HKLM "${LOG_REG_MELS_COMP_PATH}\$vendor_g_$loggerName_g" $1 $2

    ; [FileCounter]
    ; Default value is 1
    StrCpy $1 "FileCounter"
    WriteRegDWORD HKLM "${LOG_REG_MELS_COMP_PATH}\$vendor_g_$loggerName_g" $1 1

    ; [LoggerName]
    StrCpy $1 "LoggerName"
    WriteRegStr HKLM "${LOG_REG_MELS_COMP_PATH}\$vendor_g_$loggerName_g" $1 $loggerName_g

    ; [VendorName]
    StrCpy $1 "VendorName"
    WriteRegStr HKLM "${LOG_REG_MELS_COMP_PATH}\$vendor_g_$loggerName_g" $1 $vendor_g

    ; [Start]
    ; Default value is 0x99
    StrCpy $1 "Start"
    WriteRegDWORD HKLM "${LOG_REG_MELS_COMP_PATH}\$vendor_g_$loggerName_g" $1 0x99


    ; For Vista or better OS version, the 2nd level registry named autolog_provider_config is used.
    ; For XP, there is only one level registry config.
    StrCmp $os_vers_g ">xp" write_reg_autolog_provider_for_vista_or_better
    goto write_reg_autolog_for_xp

write_reg_autolog_provider_for_vista_or_better:     ; Write 2nd level registry named autolog_provider_config on Windows Vista or better OS version.
    ; [Level]
    ; /Level=xxx in the command line parameter
    StrCpy $1 "EnableLevel"
    Push $1
    Push ""
    call GetParameterValue_F
    Pop $level_g
    StrCmp $level_g "" write_reg_fail
    WriteRegDWORD HKLM "${LOG_REG_MELS_COMP_PATH}\$vendor_g_$loggerName_g\{$guid_g}" $1 $level_g

    ; [Flags]
    ; /Flags=xxx in the command line parameter
    StrCpy $1 "EnableFlags"
    Push $1
    Push ""
    call GetParameterValue_F
    Pop $flags_g
    StrCmp $flags_g "" write_reg_fail
    WriteRegDWORD HKLM "${LOG_REG_MELS_COMP_PATH}\$vendor_g_$loggerName_g\{$guid_g}" $1 $flags_g

    ; [Enabled]
    ; Default value is 1
    StrCpy $1 "Enabled"
    WriteRegDWORD HKLM "${LOG_REG_MELS_COMP_PATH}\$vendor_g_$loggerName_g\{$guid_g}" $1 1

    ; [ComponentItem]
    StrCpy $1 "ComponentItem_$vendor_g_$loggerName_g"
    WriteRegStr HKLM "${LOG_REG_MELS_CTRL_PATH}" $1 $vendor_g_$loggerName_g

    ; [ComponentCount]
    StrCpy $1 "ComponentCount"
    ReadRegDWORD $2 HKLM "${LOG_REG_MELS_CTRL_PATH}" $1
    ${LogPrint} "$1 = $2"
    IntOp $2 $2 + 1
    ${LogPrint} "Update $1 = $2"
    WriteRegDWORD HKLM "${LOG_REG_MELS_CTRL_PATH}" $1 $2

;init_etw_session:
    nsExec::Exec 'logman.exe create trace $vendor_g_$loggerName_g_INIT -mode 0x8002 -ln $vendor_g_$loggerName_g_INIT -max $maxFileSize_g -ft 1 -o $sys_root_g\LogFiles\WMI\$vendor_g\$loggerName_g.etl.001 -p {$guid_g} $flags_g $level_g -f bincirc -ets'

    goto done
 
write_reg_autolog_for_xp:                           ; Write 1st level registry for XP Modem ETW Logger Service on Wondows XP.
    ; [Level]
    ; /Level=xxx in the command line parameter
    StrCpy $1 "EnableLevel"
    Push $1
    Push ""
    call GetParameterValue_F
    Pop $level_g
    StrCmp $level_g "" write_reg_fail
    WriteRegDWORD HKLM "${LOG_REG_MELS_COMP_PATH}\$vendor_g_$loggerName_g" $1 $level_g

    ; [Flags]
    ; /Flags=xxx in the command line parameter
    StrCpy $1 "EnableFlags"
    Push $1
    Push ""
    call GetParameterValue_F
    Pop $flags_g
    StrCmp $flags_g "" write_reg_fail
    WriteRegDWORD HKLM "${LOG_REG_MELS_COMP_PATH}\$vendor_g_$loggerName_g" $1 $flags_g

	; [LogPath]
    StrCpy $1 "LogPath"
    StrCpy $2 "$sys_root_g\LogFiles\WMI\$vendor_g"
    WriteRegStr HKLM "${LOG_REG_MELS_COMP_PATH}\$vendor_g_$loggerName_g" $1 $2

    ; [ComponentItem]
    StrCpy $1 "ComponentItem_$vendor_g_$loggerName_g"
    WriteRegStr HKLM "${LOG_REG_MELS_CTRL_PATH}" $1 $vendor_g_$loggerName_g

    ; [ComponentCount]
    StrCpy $1 "ComponentCount"
    ReadRegDWORD $2 HKLM "${LOG_REG_MELS_CTRL_PATH}" $1
    ${LogPrint} "$1 = $2"
    IntOp $2 $2 + 1
    ${LogPrint} "Update $1 = $2"
    WriteRegDWORD HKLM "${LOG_REG_MELS_CTRL_PATH}" $1 $2

    IntCmp $2 1 install_service_for_xp install_service_for_xp service_version_check_for_xp

service_version_check_for_xp:
    ${LogPrint} "Service version checking"

    StrCpy $1 "ServiceVersion"
    ReadRegDWORD $2 HKLM "${LOG_REG_MELS_CTRL_PATH}" $1
    ${LogPrint} "[Installed package] $1 = $2"
    ${LogPrint} "[New package] $1 = $serviceVersion_g"
	IntCmp $serviceVersion_g $2 service_is_installed_for_xp service_is_installed_for_xp upgrade_service_for_xp

service_is_installed_for_xp:
	; Don't install service program if and only if component_count > 0 and the ServiceVersion is less than that of installed package.
    ${LogPrint} "Service is installed. Restart $vendor_g_$loggerName_g"

    ${LogPrint} "sc continue ${VENDOR} ${SERVICE_NAME}"
    nsExec::Exec 'sc continue "${VENDOR} ${SERVICE_NAME}"'

	goto done

upgrade_service_for_xp:
	; The ServiceVersion is larger than that of installed package, so upgrade the new one.
    ${LogPrint} "Upgrade Service $vendor_g_$loggerName_g"

    ${LogPrint} "sc pause ${VENDOR} ${SERVICE_NAME}"
    nsExec::Exec 'sc pause "${VENDOR} ${SERVICE_NAME}"'
		
    nsSCM::Remove "${VENDOR} ${SERVICE_NAME}"
	Pop $3
    ${LogPrint} "nsSCM Remove ${VENDOR} ${SERVICE_NAME} return $3"

    StrCpy $1 "ServicePath"
    ReadRegStr $2 HKLM "${LOG_REG_MELS_CTRL_PATH}" $1
   	${LogPrint} "Delete File: $2\${SERVICE_EXE_FILE}"
   	Delete "$2\${SERVICE_EXE_FILE}"
   	${LogPrint} "Remove $2"
   	RmDir /r "$2"

install_service_for_xp:
	; Install service program if and only if component_count <= 0 or the ServiceVersion is larger than that of installed package.
    ${LogPrint} "Install Service $vendor_g_$loggerName_g"

    SetOutPath "$InstDir\..\${VENDOR} ${SERVICE_NAME}"
    File "${SERVICE_EXE_FILE}"

	StrCpy $1 "ServiceVersion"
    WriteRegDWORD HKLM "${LOG_REG_MELS_CTRL_PATH}" $1 $serviceVersion_g
	StrCpy $1 "ServicePath"
    WriteRegStr HKLM "${LOG_REG_MELS_CTRL_PATH}" $1 "$InstDir\..\${VENDOR} ${SERVICE_NAME}"

    nsSCM::Install "${VENDOR} ${SERVICE_NAME}" "${VENDOR} ${SERVICE_NAME}" 16 2 "$InstDir\..\${VENDOR} ${SERVICE_NAME}\${SERVICE_EXE_FILE} -service" "" "" "" ""
	Pop $3
    ${LogPrint} "nsSCM Install ${VENDOR} ${SERVICE_NAME} return $3"

    nsSCM::Start "${VENDOR} ${SERVICE_NAME}"
	Pop $3
    ${LogPrint} "nsSCM Start ${VENDOR} ${SERVICE_NAME} return $3"

    goto done

done:
    ; [Done]
    ; Successfully completion.
    goto complete


;install_service_fail:
    goto complete

install_fail:
    goto complete

write_reg_fail:
read_param_fail:
    MessageBox MB_OK "Please input the parameter $1"
    goto complete

complete:
SectionEnd

;==============================================================================

Section "Uninstall"

;init:
	${LogInit}

;check_os:
    ${IF} ${IsNT}
    ${AndIf} ${AtLeastWinXP}
        ${If} ${IsWinXP}
            StrCpy $os_vers_g "xp"
            ${LogPrint} "Running on Windows XP."
        ${Else}
            StrCpy $os_vers_g ">xp"
            ${LogPrint} "Running better than XP."
        ${EndIf}
    ${Else}
        ${LogPrint} "Unsupported OS version!"
        ${LogPrint} "Abort installation."
        goto complete
    ${EndIf}

;read_param:
    ; [SYSDIR]
    Strcpy $sys_root_g $SYSDIR

    ; [Vendor]
	StrCpy $vendor_g ${VENDOR}

    ; [LoggerName]
    ; /LoggerName=xxx in the command line parameter
    StrCpy $1 "LoggerName"
    Push $1
    Push ""
    call un.GetParameterValue_F
    Pop $loggerName_g
    StrCmp $loggerName_g "" read_param_package_name dep_check

read_param_package_name:
    ; [Package]
    ; /Package=xxx in the command line parameter
    StrCpy $1 "Package"
    Push $1
    Push ""
    call un.GetParameterValue_F
    Pop $loggerName_g
    StrCmp $loggerName_g "" read_param_fail

dep_check:
    ; delete dependency if specified in the command line parameter
    ${LogPrint} "Delete dependency"
    Push ${PACKAGE}
    Push ${VENDOR}
    call un.DeleteDependency

    ; an update uninstallation request?
    ${LogPrint} "If an update uninstallation request?"
    Call un.IfAnUpdateUninstall
    Pop $updt_uninst
    StrCmp $updt_uninst "1" delete_all_others

    ; dependency there ?
    ${LogPrint} "Check dependency"
    Push ${PACKAGE}
    Push ${VENDOR}
    Call un.CheckDependency
    Pop $6
    StrCmp $6 "" delete_all_others dep_found
    
dep_found:
    StrCpy $6 $6 "" 4 ;"DEP_"
    MessageBox MB_OK "Remove $6 first!"
    SetErrorLevel 3
    Goto uninst_sec_exit
    
delete_all_others:

;remove_service:
    StrCmp $os_vers_g ">xp" remove_service_for_vista_or_better 
    goto remove_service_for_xp 

remove_service_for_vista_or_better:
;stop_etw_session_for_vista_or_better:
    nsExec::Exec 'logman.exe stop $vendor_g_$loggerName_g_INIT -ets'
    nsExec::Exec 'logman.exe stop $vendor_g_$loggerName_g -ets'

;delete_reg_autolog_for_vista_or_better:
	; [ComponentItem]
    ${LogPrint} "Delete KEY: ${LOG_REG_MELS_CTRL_PATH}\ComponentItem_$vendor_g_$loggerName_g"
    DeleteRegValue HKLM "${LOG_REG_MELS_CTRL_PATH}" "ComponentItem_$vendor_g_$loggerName_g"

    ; [ComponentCount]
    StrCpy $1 "ComponentCount"
    ReadRegDWORD $2 HKLM "${LOG_REG_MELS_CTRL_PATH}" $1
    ${LogPrint} "$1 = $2"
    IntOp $2 $2 - 1
    IntCmp $2 0 delete_reg_component_count_for_vista_or_better delete_reg_component_count_for_vista_or_better update_reg_component_count_for_vista_or_better

	; Case 1. Component is equal to 0. It means that there is no other MELS component.
	; Therefore, delete MELS Control registry and remove ETW directory.
	delete_reg_component_count_for_vista_or_better:
        ${LogPrint} "Delete KEY: ${LOG_REG_MELS_CTRL_PATH}\$1"
        DeleteRegValue HKLM "${LOG_REG_MELS_CTRL_PATH}" $1
        DeleteRegKey HKLM "${LOG_REG_MELS_CTRL_PATH}\"

	;remove_etl_dir_for_vista_or_better:
        ${LogPrint} "Delete File: $sys_root_g\LogFiles\WMI\$vendor_g\$loggerName_g.etl.*"
        Delete $sys_root_g\LogFiles\WMI\$vendor_g\$loggerName_g.etl.*
        ${LogPrint} "RmDir $sys_root_g\LogFiles\WMI\$vendor_g"
        RmDir /r $sys_root_g\LogFiles\WMI\$vendor_g

        goto delete_reg_autolog_session

	; Case 2. Component is larger than 0. It means that there is other MELS component.
	; Therefore, only update MELS Control registry.
    update_reg_component_count_for_vista_or_better:
        ${LogPrint} "Update KEY ${LOG_REG_MELS_CTRL_PATH}\$1 to $2"
        WriteRegDWORD HKLM "${LOG_REG_MELS_CTRL_PATH}" $1 $2

	;remove_etl_dir:
        ${LogPrint} "Delete File: $sys_root_g\LogFiles\WMI\$vendor_g\$loggerName_g.etl.*"
        Delete $sys_root_g\LogFiles\WMI\$vendor_g\$loggerName_g.etl.*

		goto delete_reg_autolog_session 

remove_service_for_xp:
;delete_reg_autolog_for_xp:
	nsExec::Exec 'logman.exe stop $vendor_g_$loggerName_g -ets'

    ; [ComponentItem]
    ${LogPrint} "Delete KEY: ${LOG_REG_MELS_CTRL_PATH}\ComponentItem_$vendor_g_$loggerName_g"
    DeleteRegValue HKLM "${LOG_REG_MELS_CTRL_PATH}" "ComponentItem_$vendor_g_$loggerName_g"

    ; [ComponentCount]
    StrCpy $1 "ComponentCount"
    ReadRegDWORD $2 HKLM "${LOG_REG_MELS_CTRL_PATH}" $1
    ${LogPrint} "$1 = $2"
    IntOp $2 $2 - 1
    IntCmp $2 0 delete_service_program_for_xp delete_service_program_for_xp update_reg_component_count

	; Case 1. Component is equal to 0. It means that there is no other MELS component.
	; Therefore, delete MELS Control registry and remove MELS service.
	delete_service_program_for_xp:
		${LogPrint} "nsSCM Stop ${VENDOR} ${SERVICE_NAME}"
    	nsSCM::Stop "${VENDOR} ${SERVICE_NAME}"
		Pop $3
    	${LogPrint} "Stop ${VENDOR} ${SERVICE_NAME} return $3"

        ${LogPrint} "nsSCM Remove ${VENDOR} ${SERVICE_NAME}"
    	nsSCM::Remove "${VENDOR} ${SERVICE_NAME}"
		Pop $3
    	${LogPrint} "Remove ${VENDOR} ${SERVICE_NAME} return $3"

		StrCpy $1 "ServicePath"
    	ReadRegStr $2 HKLM "${LOG_REG_MELS_CTRL_PATH}" $1
   		${LogPrint} "Delete File: $2\${SERVICE_EXE_FILE}"
   		Delete "$2\${SERVICE_EXE_FILE}"
   		${LogPrint} "Remove $2"
   		RmDir /r "$2"

    ;delete_reg_component_count_for_xp:
        DeleteRegKey HKLM "${LOG_REG_MELS_CTRL_PATH}\"

    ;remove_etl_dir:
        ${LogPrint} "Delete File: $sys_root_g\LogFiles\WMI\$vendor_g\$loggerName_g.etl.*"
        Delete $sys_root_g\LogFiles\WMI\$vendor_g\$loggerName_g.etl.*
        ${LogPrint} "RmDir $sys_root_g\LogFiles\WMI\$vendor_g"
        RmDir /r $sys_root_g\LogFiles\WMI\$vendor_g

        goto delete_reg_autolog_session

	; Case 2. Component is larger than 0. It means that there is other MELS component.
	; Therefore, only update MELS Control registry.
    update_reg_component_count:
        ${LogPrint} "Update KEY ${LOG_REG_MELS_CTRL_PATH}\$1 to $2"
        WriteRegDWORD HKLM "${LOG_REG_MELS_CTRL_PATH}" $1 $2

	;remove_etl_dir:
        ${LogPrint} "Delete File: $sys_root_g\LogFiles\WMI\$vendor_g\$loggerName_g.etl.*"
        Delete $sys_root_g\LogFiles\WMI\$vendor_g\$loggerName_g.etl.*

        goto delete_reg_autolog_session

delete_reg_autolog_session:
    ${LogPrint} "Delete KEY: ${LOG_REG_MELS_COMP_PATH}\$vendor_g_$loggerName_g"
    DeleteRegKey HKLM "${LOG_REG_MELS_COMP_PATH}\$vendor_g_$loggerName_g\"

;remove_install_package:
    ; remove the install package   
    ${LogPrint} "Removing the installed package" 
    Push ${UNINSTALLER}
    Push $loggerName_g
    Push $vendor_g
    Push ${VERSION}
    Push $updt_uninst
    call Un.UninstallPackage

uninst_sec_exit:
    goto complete

read_param_fail:
;    MessageBox MB_OK "Please input the parameter $1"
	
	;find_logger_name:
		StrCpy $1 $InstDir
    	IntOp $tmp_g 0 + 0
		
	loop:
		Push $1
		Push "\"
		Call un.StrStr
		Pop $2

        ${LogPrint} "1=$1, 2=$2"
		StrCmp $2 "" loop2
		StrCpy $1 $2 "" 1

    	IntOp $tmp_g $tmp_g + 1
		StrCmp $tmp_g 100 complete

		goto loop

	loop2:
        ${LogPrint} "Find package name=$1"

		; Remove postfix "_LOG"
		StrCpy $2 $1 -4

        ${LogPrint} "Please click ..\$2_Driver\UninstallDriver.exe to uninstall"
	    MessageBox MB_OK "Please click ..\$2_Driver\UninstallDriver.exe to uninstall"
	
    	goto complete

complete:
SectionEnd

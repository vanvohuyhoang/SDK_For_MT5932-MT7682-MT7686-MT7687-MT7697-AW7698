!ifndef MTK_ETW_LOG_NSH
!define MTK_ETW_LOG_NSH

!addincludedir ..\nsis_utils
!include "strstr.nsh"
!include "param.nsh"

;+------------------------------------------------------------------------------
; Version definition
;
!define MAJOR_VER               1
!define MINOR_VER               12
!define MICRO_VER               44
!define BUILD_VER               1
!define VERSION                 "${MAJOR_VER}.${MINOR_VER}.${MICRO_VER}.${BUILD_VER}"


;+------------------------------------------------------------------------------
; ID definition
;
!define VENDOR                  "MediaTek"
!define SERVICE_NAME			"Modem Driver Logger"
!define SERVICE_EXE_FILE		"modem_logger_srv.exe"

;+------------------------------------------------------------------------------
; INF file name definition
;

;+------------------------------------------------------------------------------
; General
;
!define UNINSTALLER             "UninstallLog.exe"
!define PACKAGE                 "Log"


;+------------------------------------------------------------------------------
; Function
;
Function GetParameterValue_F
  ${GetParameterValue}
FunctionEnd

Function un.GetParameterValue_F
  ${GetParameterValue}
FunctionEnd

Function un.StrStr
  ${StrStr}
FunctionEnd

!endif

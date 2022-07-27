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
 *   dev_inst.c
 *
 ****************************************************************************/

#include <stdio.h> 
#include <string.h>
#include <windows.h>  
#include <setupapi.h> 
#include <newdev.h>
#include <shellapi.h>

int InstallDriver(CHAR *pDevId, CHAR *pInfPath);
 
void DbgPrint(const char *format, ...)
{
    char buf[512] = "[dev_inst] ";
    va_list ap;

    va_start(ap, format);

    _vsnprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), format, ap);

    OutputDebugString(buf);

    va_end(ap);
}

/*
* Convert lpCmdLine of WinMain to argc/argv like arguments
*/
PCHAR* CommandLineToArgvA(PCHAR CmdLine, int* _argc)
{
	PCHAR* argv;
	PCHAR  _argv;
	ULONG   len;
	ULONG   argc;
	CHAR   a;
	ULONG   i, j;

    BOOLEAN  in_QM;
	BOOLEAN  in_TEXT;
	BOOLEAN  in_SPACE;
	
	len = strlen(CmdLine);
	i = ((len + 2) / 2) * sizeof(PVOID) + sizeof(PVOID);
	
	argv = (PCHAR *)GlobalAlloc(GMEM_FIXED, i + (len+2) * sizeof(CHAR));
	
	_argv = (PCHAR)(((PUCHAR)argv)+i);
	
	argc = 0;
	argv[argc] = _argv;
	in_QM = FALSE;
	in_TEXT = FALSE;
	in_SPACE = TRUE;
	i = 0;
	j = 0;
	
	while( a = CmdLine[i] ) {
		if(in_QM) {
			if(a == '\"') {
				in_QM = FALSE;
			} else {
				_argv[j] = a;
				j++;
			}
		} else {
			switch(a) {
				case '\"':
					in_QM = TRUE;
					in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    in_SPACE = FALSE;
                    break;
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    if(in_TEXT) {
                        _argv[j] = '\0';
                        j++;
                    }
                    in_TEXT = FALSE;
                    in_SPACE = TRUE;
                    break;
                default:
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    _argv[j] = a;
                    j++;
                    in_SPACE = FALSE;
                    break;
            }
        }
        i++;
    }
    _argv[j] = '\0';
    argv[argc] = NULL;

    (*_argc) = argc;
    return argv;
}


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
/*

Usage:
  dev_inst.exe <HWID> <infPath>

Example:
  dev_inst.exe USB\Vid_0x8d&Pid_7112 // use "^&" instead of "&" in console 

Return Value:
    Standard Console ERRORLEVEL values:

    0 - Remove Successfull
    2 - Remove Failure.
    
--*/
{
	HDEVINFO DeviceInfoSet;
    SP_DEVINFO_DATA DeviceInfoData;
    DWORD i,err;
	LPSTR *szArglist;
	int nArgs;

    DbgPrint("%s", lpCmdLine);

    if (strlen(lpCmdLine) == 0)
    {
        DbgPrint("usage: dev_inst <Hardware_ID> <fullInfPath>\n");
        return 1; // Remove Failure
    }
	
	szArglist = CommandLineToArgvA(lpCmdLine, &nArgs);
	if (NULL == szArglist) {
		DbgPrint("Cannot change");
		return -1;
	}
	
	for (i = 0; i < nArgs; i++) {
		DbgPrint("argv[%d] = %s\n", i, szArglist[i]);
	}
	
	InstallDriver(szArglist[0], szArglist[1]);
	
	LocalFree(szArglist);

    return 0;
}
	
int InstallDriver(CHAR *pDevId, CHAR *pInfPath)
{
	DbgPrint("Installing: %s at %s\n", pDevId, pInfPath);

	if(!UpdateDriverForPlugAndPlayDevices(NULL, pDevId, pInfPath, 0, NULL ))
	{
		switch (GetLastError())
		{
			case ERROR_FILE_NOT_FOUND:
				DbgPrint("INF file %s not found\n", pInfPath);
				return -1;
			case NO_ERROR:
				DbgPrint("UpdateDriverForPlugAndPlayDevices: NO_ERROR\n");
				break;
			case ERROR_IN_WOW64:
				DbgPrint("UpdateDriverForPlugAndPlayDevices: ERROR_IN_WOW64");
				SetupCopyOEMInf(pInfPath, NULL, SPOST_PATH, 0, NULL, 0, NULL, NULL);
				DbgPrint("SetupCopyOEMInf: done\n");
				break;
			case ERROR_NO_SUCH_DEVINST:
				DbgPrint("UpdateDriverForPlugAndPlayDevices: device %s is not plugin\n", pDevId);
			default:
			//we need to copy and (re-install the driver if needed)
				SetupCopyOEMInf(pInfPath, NULL, SPOST_PATH, 0, NULL, 0, NULL, NULL);
				DbgPrint("SetupCopyOEMInf: done\n");
			break;
		}
	}
	else
	{
		DbgPrint("UpdateDriverForPlugAndPlayDevices: OK\n");
	}
	return 0;
}

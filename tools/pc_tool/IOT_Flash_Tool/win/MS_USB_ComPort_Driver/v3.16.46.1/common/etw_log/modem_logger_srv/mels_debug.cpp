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

#include "mels_precomp.h"

#ifdef _WIN32
#if !defined MELS_RELEASE
    #include <crtdbg.h>
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
	#define new			new(_NORMAL_BLOCK, __FILE__, __LINE__)
	#define malloc(s)	_malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif

#define MAX_DBG_MSG_SIZE 1024


#if defined ENABLE_LOG_TO_FILE
static FILE *fpCurrLog = NULL;
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

void mels_log_deinit()
{
	if (fpCurrLog) {
		fclose(fpCurrLog);
	}

	return ;
}

void mels_log_to_file(const PINT8 lpOutText, ...)
{
	fpos_t pos;
	int errno;
	SYSTEMTIME stNow;
	char szPlainMsg[1024];
	int nSize;
	TCHAR dllPath[MAX_PATH];
	TCHAR ExecNameCur[MAX_PATH];
	TCHAR ExecNameOld[MAX_PATH];

    GetLocalTime(&stNow);

	/*
     * Open current log file to append new message.
	 */
	if (NULL == fpCurrLog) {
		GetModuleFileName((HINSTANCE)&__ImageBase, dllPath, MAX_PATH);
    	*(strrchr(dllPath, '\\')) = '\0';
    	sprintf(ExecNameCur, "%s\\%s", dllPath, MELS_CUR_LOG_FILE);

		fpCurrLog = fopen(ExecNameCur, "ab+");
	}
	
	if(NULL != fpCurrLog) {	
		fseek(fpCurrLog, 0, SEEK_END);
		errno = fgetpos(fpCurrLog, &pos);
		if(0 == errno) {		
			/*
			 * Back up current log file and age out older one.
			 */
			if(pos > MELS_MAX_LOG_SIZE) {
				fclose(fpCurrLog);
				fpCurrLog = NULL;

				GetModuleFileName((HINSTANCE)&__ImageBase, dllPath, MAX_PATH);
    			*(strrchr(dllPath, '\\')) = '\0';

    			sprintf(ExecNameOld, "%s\\%s", dllPath, MELS_OLD_LOG_FILE);
    			sprintf(ExecNameCur, "%s\\%s", dllPath, MELS_CUR_LOG_FILE);
				DeleteFile(ExecNameOld);
				MoveFile(ExecNameCur, ExecNameOld);

				fpCurrLog = fopen(ExecNameCur, "wb");
				if(NULL == fpCurrLog) {
					//DbgPrint(("mels_log_to_file: failed to create the log file %s after backup!\n", MELS_CUR_LOG_FILE));
					return;
				}
			}

			/*
			 * Append the message to current log file.
			 */
			_snprintf(szPlainMsg, 1023, "[%02d:%02d:%02d] %s", stNow.wHour, stNow.wMinute, stNow.wSecond, lpOutText);
			szPlainMsg[1023] = '\0';
			nSize = strlen(szPlainMsg);
			if (nSize > 0) {
				fwrite(szPlainMsg, 1, nSize, fpCurrLog);
				fflush(fpCurrLog);
			}
		} else {
			//DbgPrint(("mels_log_to_file: fgetpos(%s), errno: %d!\n", MELS_CUR_LOG_FILE, errno));
		}

		// fclose(fpCurrLog);
		// fpCurrLog = NULL;
	} else {
		//DbgPrint(("mels_log_to_file: failed to open the log file %s!\n", MELS_CUR_LOG_FILE));
	}
	
}
#endif /* ENABLE_LOG_TO_FILE */

void mels_dbg_print(const PINT8 format, ...)
{
	DWORD	dwErrCode;
	CHAR	aBuf[MAX_DBG_MSG_SIZE];
	va_list	ap;

	dwErrCode = GetLastError();

	va_start(ap, format);
	_vsnprintf(aBuf, MAX_DBG_MSG_SIZE, format, ap);
	va_end(ap);

//#if !defined MELS_RELEASE
    OutputDebugStringA(aBuf);
//#endif

#if defined ENABLE_LOG_TO_FILE
	mels_log_to_file(aBuf);
#endif

	SetLastError(dwErrCode);
}

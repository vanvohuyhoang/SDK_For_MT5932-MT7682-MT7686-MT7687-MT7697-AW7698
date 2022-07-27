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

#ifndef _MELS_DEFS_H
#define _MELS_DEFS_H


/*----------------------------------------------------------------------
 * MELS Option Definition
 *--------------------------------------------------------------------*/
//#define OUTPUT_LOG_TO_CONSOLE
#define ENABLE_LOG_TO_FILE
#define MELS_RELEASE

/*----------------------------------------------------------------------
 * MELS Definition
 *--------------------------------------------------------------------*/
#define VENDOR_NAME										"MediaTek"

#define MELS_REG_KEY									"Software\\" VENDOR_NAME "\\Modem Driver Logger"
#define MELS_REG_KEY_COMPONENT_COUNT					"ComponentCount"
#define MELS_REG_KEY_COMPONENT_ITEM_NAME_PREFIX			"ComponentItem_"
#define MELS_REG_KEY_EXTENDED_COMMAND					"ExtCmd"
#define MELS_REG_KEY_EXTENDED_COMMAND_BATCH_FILE_NAME	"mls_ext_cmd.bat"
#define MELS_REG_KEY_EXTENDED_COMMAND_ENABLE_CODE		0x24547208

#define MELS_REG_COMP_VALUE_NEW_INSTALLED				0x99

#define MELS_COMP_REG_KEY								"System\\CurrentControlSet\\Control\\WMI\\Autologger\\"

#define MELS_SERVICE_NAME								VENDOR_NAME " Modem Driver Logger"

#define MELS_CUR_LOG_FILE								"mels_cur.log"
#define MELS_OLD_LOG_FILE								"mels_old.log"
#define MELS_MAX_LOG_SIZE								0x500000


/*----------------------------------------------------------------------
 * Definition
 *--------------------------------------------------------------------*/
#define MAX_BUF_SIZE									512
#define MAX_LOG_PATH_SIZE								512
#define MAX_LOG_VENDOR_SIZE								256
#define MAX_LOG_NAME_SIZE								256
#define MAX_LOG_GUID_SIZE								39
#define MAX_VENDOR_NAME_SIZE							256

#define MAX_LOG_COMP_COUNT								4096

/*----------------------------------------------------------------------
 * Test Configuration
 *--------------------------------------------------------------------*/
//#define TEST_CONF


#endif /* _MELS_DEFS_H */

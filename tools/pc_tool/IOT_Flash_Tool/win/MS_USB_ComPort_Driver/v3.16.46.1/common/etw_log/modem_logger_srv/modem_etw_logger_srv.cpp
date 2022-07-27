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
#include <sys/stat.h>
#include <shlobj.h>
#include <winreg.h>

#pragma warning( disable : 4127 )

/*----------------------------------------------------------------------
 * Helper macro definition.
 *--------------------------------------------------------------------*/
#define UPDATE_MELS_STATUS(_code)  \
		mels_status_g = _code;  \
		mels_write_registry(MELS_REG_KEY, "Status", &(mels_status_g), sizeof(UINT32), REG_DWORD);

// Note: Use this MACRO if and only if comp_key is setting
#define UPDATE_MELS_COMP_STATUS(_inst, _code)  \
		_inst->status = _code;  \
		if (strlen(_inst->comp_key)) {  \
			mels_write_registry(_inst->comp_key, "Status", &(_inst->status), sizeof(UINT32), REG_DWORD);  \
		}

#define LOG_CMD_EXEC						"logman.exe"
#define LOG_TRACE_FILE_MODE_CIRCULAR		0x00000002
#define LOG_TRACE_USE_LOCAL_SEQUENCE		0x00008000

/*----------------------------------------------------------------------
 * Structure definition.
 *--------------------------------------------------------------------*/
typedef enum {
	RET_CODE_SUCCESS,
	RET_CODE_ERROR,
	RET_CODE_INVALID_PARAM,
	RET_CODE_NO_RESOURCE,
	RET_CODE_INVALID_STATE,
	RET_CODE_CREATE_PROCESS_FAILED,
	RET_CODE_CREATE_FILE_FAILED,
	RET_CODE_FILE_NOT_EXIST,
	RET_CODE_OPEN_KEY_FAILED,
	RET_CODE_READ_KEY_FAILED,
	RET_CODE_WRITE_KEY_FAILED,
	RET_CODE_LOG_COMP_NUM_IS_EXCEEDING,
	RET_CODE_EXT_CMD_KEY_IS_INVALID,
} mels_ret_code_t;

typedef struct {
	INT8		comp_key[MAX_BUF_SIZE];
	INT8		log_path[MAX_LOG_PATH_SIZE];
	INT8		vendor_name[MAX_LOG_VENDOR_SIZE];
	INT8		log_name[MAX_LOG_NAME_SIZE];
	INT8		log_guid[MAX_LOG_GUID_SIZE];
	UINT32		max_file_size;
	UINT32		file_max;
	UINT32		log_level;
	UINT32		log_flags;
	UINT32		file_count;
	UINT32		log_file_mode;
	UINT32		flush_timer;
	UINT32		start;
	UINT32		status;
} etw_log_inst_t;

typedef struct {
	INT8	value_name[MAX_BUF_SIZE];
	INT8	value_data[MAX_BUF_SIZE];
} enum_string_reg_t;

typedef struct {
	INT8		name[MAX_BUF_SIZE];
	INT32		type;
	PVOID		value;
} mels_comp_reg_entry_t;

typedef enum {
	MELS_REG_ST_READ_REG_ENTRY						= 0x91,
	MELS_REG_ST_CHECK_ETW_FILE_ENTRY				= 0x92,
	MELS_REG_ST_START_LOG_SESSION_ENTRY				= 0x93,
	MELS_REG_ST_RUN_BATCH_ENTRY						= 0x99,

	/* Success Case */
	MELS_REG_ST_DONE								= 0x0,
	MELS_REG_ST_RUN_BATCH_OK						= 0x1,

	/* Failed Case */
	MELS_REG_ST_READ_REG_COMP_COUNT_FAILED		    = 0x31,
	MELS_REG_ST_COMP_NUM_IS_EXCEEDING_FAILED		= 0x32,
	MELS_REG_ST_RUN_BATCH_FAILED					= 0x39,

} mels_reg_st_t;

typedef enum {
	/* Success Case */
	MELS_REG_COMP_ST_DONE							= 0x0,
	MELS_REG_COMP_ST_INIT_OK						= 0x1,
	MELS_REG_COMP_ST_READ_REG_OK					= 0x2,
	MELS_REG_COMP_ST_DIR_IS_EXIST_OK				= 0x3,
	MELS_REG_COMP_ST_CREATE_ETW_DIR_OK				= 0x4,
	MELS_REG_COMP_ST_START_SESSION_OK				= 0x5,
	MELS_REG_COMP_ST_STOP_SESSION_OK				= 0x6,

	/* Failed Case */
	MELS_REG_COMP_ST_READ_REG_FAILED				= 0x31,
	MELS_REG_COMP_ST_LOG_PATH_IS_NOT_DIR_FAILED		= 0x41,
	MELS_REG_COMP_ST_CREATE_ETW_DIR_FAILED			= 0x42,
	MELS_REG_COMP_ST_START_SESSION_FAILED			= 0x51,
	MELS_REG_COMP_ST_STOP_SESSION_FAILED			= 0x62,
} mels_reg_comp_st_t;

/*----------------------------------------------------------------------
 * Global variables.
 *--------------------------------------------------------------------*/
etw_log_inst_t				*p_inst_g = NULL;
UINT32						inst_num_g = 0;
UINT32						mels_status_g;
SERVICE_STATUS				ServiceStatus_g;
SERVICE_STATUS_HANDLE		hServiceStatus_g;
HANDLE						mels_event_g = NULL;



/*----------------------------------------------------------------------
 * Private implementation.
 *   - Registry function.
 *--------------------------------------------------------------------*/
mels_ret_code_t mels_write_registry(
		PINT8 key_name,
		PINT8 value_name,
		PVOID in_buf,
		UINT32 in_buf_size,
		DWORD type)
{
	mels_ret_code_t				ret;
	HKEY						reg_key;
	LONG						result;

	/*
	 * Init variable.
	 */
	ret = RET_CODE_SUCCESS;

	/*
	 * 1. Open key.
	 */
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,				// hKey
							key_name,						// lpSubKey
							0,								// ulOptions
							KEY_WRITE,						// samDesired
							&reg_key);						// phkResult

	/*
	 * 2. Query key.
	 */
	if (ERROR_SUCCESS == result) {
		// OK. Successfully open registry key.

		result = RegSetValueEx(reg_key,						// hKey
								value_name,					// lpValueName
								NULL,						// Reserved
								type,						// dwType
								(LPBYTE)in_buf,				// lpData
								in_buf_size);				// cbData

		RegCloseKey(reg_key);

	} else {
		// NG. Failed to open registry key.
		LOG(LOG_ERROR, ("Failed to open MELS registry key(%s) for write!\n", key_name));
		ret = RET_CODE_OPEN_KEY_FAILED;
		goto _failed;
	}

	/*
	 * 3. Check value.
	 */
	if (ERROR_SUCCESS == result) {
		// OK. 
		LOG(LOG_INFO, ("Successfully write key(%s) value(%s), size(%d), type(%d).\n", key_name, value_name, in_buf_size, type))

	} else {
		// NG. Invalid type is acquired.
		LOG(LOG_ERROR, ("Failed to write registry, key(%s) value(%s), size(%d), type(%d)!\n", key_name, value_name, in_buf_size, type));
		ret = RET_CODE_WRITE_KEY_FAILED;
		goto _failed;
	}

_failed:
	return ret;
}


mels_ret_code_t mels_read_registry(
		PINT8 key_name,
		PINT8 value_name,
		PVOID out_buf,
		UINT32 out_buf_size,
		UINT32 *out_size,
		DWORD expected_type)
{
	mels_ret_code_t				ret;
	HKEY						reg_key;
	DWORD						size, type;
	LONG						result;
	

	/*
	 * Init variable.
	 */
	ret = RET_CODE_SUCCESS;


	/*
	 * 1. Open key.
	 */
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,				// hKey
							key_name,						// lpSubKey
							0,								// ulOptions
							KEY_READ,						// samDesired
							&reg_key);						// phkResult

	/*
	 * 2. Query key.
	 */
	if (ERROR_SUCCESS == result) {
		// OK. Successfully open registry key.

		size = out_buf_size;
		result = RegQueryValueEx(reg_key,					// hKey
								value_name,					// lpValueName
								NULL,						// lpReserved
								&type,						// lpType
								(LPBYTE)out_buf,			// lpData
								&size);						// lpcbData
		
		RegCloseKey(reg_key);

	} else {
		// NG. Failed to open registry key.
		LOG(LOG_ERROR, ("Failed to open MELS registry key(%s) for read!\n", key_name));
		ret = RET_CODE_OPEN_KEY_FAILED;
		goto _failed;
	}

	if (out_size) {
		*out_size = size;
	}

	/*
	 * 3. Check value.
	 */
	if (ERROR_SUCCESS == result) {
		if (type == expected_type) {
			// OK. 
			LOG(LOG_INFO, ("Successfully read key(%s) value(%s), size(%d).\n", key_name, value_name, size));

		} else {
			// NG. Invalid type is acquired.
			LOG(LOG_ERROR, ("Invalid registry type is acquired, key(%s) value(%s), type(%d), expected_type(%d)!\n", key_name, value_name, type, expected_type));
			ret = RET_CODE_READ_KEY_FAILED;
			goto _failed;
		}

	} else if (ERROR_FILE_NOT_FOUND == result) {
		// NG. Value does not exist!
		LOG(LOG_ERROR, ("Failed to read key(%s) value(%s)!\n", key_name, value_name));
		ret = RET_CODE_READ_KEY_FAILED;
		goto _failed;
	} else if (ERROR_MORE_DATA == result) {
		// NG. Need more buffer.
		LOG(LOG_ERROR, ("Need more buffer, key(%s) value(%s), given buf_size(%d), needed buf_size(%d)!\n", key_name, value_name, out_buf_size, size));
		ret = RET_CODE_READ_KEY_FAILED;
		goto _failed;
	} else {
		// NG. Unknown error.
		LOG(LOG_ERROR, ("Unknown error(%d), key(%s) value(%s)!\n", result, key_name, value_name));
		ret = RET_CODE_READ_KEY_FAILED;
		goto _failed;
	}

_failed:
	return ret;
}

mels_ret_code_t mels_enum_registry_value(
		PINT8 key_name,
		PVOID enum_buf,
		PUINT32 enum_buf_size,
		DWORD expected_type)
{
	mels_ret_code_t				ret;
	HKEY						reg_key;
	LONG						result;
	UINT32						idx;
	PINT8						p_name, p_data;
	UINT32						name_size, data_size;
	DWORD						type;
	UINT32						enum_count;
	enum_string_reg_t			*p_enum_buf_str;

	// Until now, this function only handles REG_SZ type.
	if (REG_SZ != expected_type) {
		LOG(LOG_ERROR, ("Unsupported type(%d)!\n", expected_type));
		ret = RET_CODE_INVALID_PARAM;
		goto _failed;
	}

	/*
	 * Init variable.
	 */
	ret = RET_CODE_SUCCESS;
	idx = 0;
	type = expected_type;
	enum_count = 0;
	p_enum_buf_str = (enum_string_reg_t *)enum_buf;
	p_name = p_enum_buf_str->value_name;
	p_data = p_enum_buf_str->value_data;

	/*
	 * 1. Open key.
	 */
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,				// hKey
							key_name,						// lpSubKey
							0,								// ulOptions
							KEY_READ,						// samDesired
							&reg_key);						// phkResult

	/*
	 * 2. Enumerate key.
	 */
	if (ERROR_SUCCESS == result) {
		// OK. Successfully open registry key.

		for (idx = 0; enum_count<*enum_buf_size; idx++) {
			name_size = MAX_BUF_SIZE;
			data_size = MAX_BUF_SIZE;
			result = RegEnumValue(reg_key,					// hKey
									idx,					// dwIndex
									p_name,					// lpValueName
									&name_size,				// lpcchValueName
									NULL,					// lpReserved
									&type,					// lpType
									(LPBYTE)p_data,			// lpData
									&data_size);			// lpcbData


			if (ERROR_NO_MORE_ITEMS == result) {
				// Scan complete and there is no more data item.
				break;
			}

			if (type == expected_type) {
				// OK.
				p_enum_buf_str++;
				p_name = p_enum_buf_str->value_name;
				p_data = p_enum_buf_str->value_data;
				enum_count++;
			} else {
				// Don't care. Unexpected_type.
				continue;
			}

		} /* for (idx=0; idx<; idx++) */

		*enum_buf_size = enum_count;
		
		RegCloseKey(reg_key);

	} else {
		// NG. Failed to open registry key.
		LOG(LOG_ERROR, ("Failed to open MELS registry key(%s)!\n", key_name));
		ret = RET_CODE_OPEN_KEY_FAILED;
		goto _failed;
	}


_failed:
	return ret;
}


/*----------------------------------------------------------------------
 * Private implementation.
 *--------------------------------------------------------------------*/
mels_ret_code_t mels_win_exec_cmd(PINT8 command)
{
	mels_ret_code_t				ret;
	STARTUPINFO					si;
	PROCESS_INFORMATION			pi;

	/*
	 * Init variable.
	 */
	ret = RET_CODE_SUCCESS;
	kal_zero_memory(&si, sizeof(si));
	si.cb = sizeof(si);
	kal_zero_memory(&pi, sizeof(pi));

	if ( CreateProcess( NULL,		// lpApplicationName
						command,	// Command line
						NULL,		// Process handle not inheritable
						NULL,		// Thread handle not inheritable
						FALSE,		// Set handle inheritance to FALSE
						NULL ,			// No creation flags, CREATE_NO_WINDOW
						NULL,		// Use parent's environment block
						NULL,		// Use parent's starting directory
						&si,		// Pointer to STARTUPINFO structure
						&pi )		// Pointer to PROCESS_INFORMATION structure
	) {
		// OK. Successfully create child process to handle command.

		// Wait until child process exits.
		WaitForSingleObject(pi.hProcess, INFINITE);
	} else {
		// NG. Failed to create child process to handle command.

		LOG(LOG_ERROR, ("Failed to CreateProcess() with error %d!\n", GetLastError()));
		ret = RET_CODE_CREATE_PROCESS_FAILED;
	}


	return ret;
}

mels_ret_code_t mels_set_default_value(etw_log_inst_t *p_inst)
{
	mels_ret_code_t				ret;

	etw_log_inst_t default_conf_s = {
		"",																	// comp_key
		"C:\\WINDOWS\\system32\\LogFiles\\WMI\\" VENDOR_NAME,				// log_path
		"",																	// vendor_name
		"",																	// log_name
		"",																	// log_guid
		5,																	// max_file_size (MB)
		4,																	// file_max
		4,																	// log_level
		0xffffff,															// log_flags
		0,																	// file_count
		(LOG_TRACE_FILE_MODE_CIRCULAR | LOG_TRACE_USE_LOCAL_SEQUENCE),		// log_file_mode
		1,																	// flush_timer
		0,																	// start
		MELS_REG_COMP_ST_INIT_OK											// status
	};

	/*
	 * Init variable.
	 */
	ret = RET_CODE_SUCCESS;

	kal_zero_memory(p_inst, sizeof(etw_log_inst_t));
	kal_copy_memory(p_inst->comp_key, default_conf_s.comp_key, strlen(default_conf_s.comp_key));
	kal_copy_memory(p_inst->log_path, default_conf_s.log_path, strlen(default_conf_s.log_path));
	kal_copy_memory(p_inst->vendor_name, default_conf_s.vendor_name, strlen(default_conf_s.vendor_name));
	kal_copy_memory(p_inst->log_name, default_conf_s.log_name, strlen(default_conf_s.log_name));
	kal_copy_memory(p_inst->log_guid, default_conf_s.log_guid, strlen(default_conf_s.log_guid));
	p_inst->max_file_size = default_conf_s.max_file_size;
	p_inst->file_max = default_conf_s.file_max;
	p_inst->log_level = default_conf_s.log_level;
	p_inst->log_flags = default_conf_s.log_flags;
	p_inst->file_count = default_conf_s.file_count;
	p_inst->log_file_mode = default_conf_s.log_file_mode;
	p_inst->flush_timer = default_conf_s.flush_timer;
	p_inst->start = default_conf_s.start;
	p_inst->status = default_conf_s.status;

	return ret;
}

#define CHECK_REG_DATA(_cmd)  \
		ret = _cmd;  \
		if (RET_CODE_SUCCESS != ret) {  \
			UPDATE_MELS_COMP_STATUS(p_inst_tmp, MELS_REG_COMP_ST_READ_REG_FAILED);  \
			continue;  \
		}

mels_ret_code_t mels_read_conf(etw_log_inst_t **pp_inst, UINT32 *p_inst_num)
{
	mels_ret_code_t				ret;
	UINT32						comp_count;
	UINT32						idx;
	enum_string_reg_t			*p_enum_buf, *p_enum_tmp;
	UINT32						enum_buf_sz;
	DWORD						type;
	etw_log_inst_t				*p_inst_tmp;

	/*
	 * Init variable.
	 */
	ret = RET_CODE_SUCCESS;
	comp_count = 0;
	idx = 0;
	p_enum_buf = NULL;
	p_inst_tmp = *pp_inst;
	*p_inst_num = 0;

	/*
	 * Read configuration from registry.
	 */	
	if (RET_CODE_SUCCESS == (ret = mels_read_registry(MELS_REG_KEY, MELS_REG_KEY_COMPONENT_COUNT, &comp_count, sizeof(UINT32), NULL, REG_DWORD)) &&
		comp_count > 0) {
		// OK. Successfully read MELS_REG_KEY_COMPONENT_COUNT.
		type = REG_SZ;
		enum_buf_sz = comp_count;
		*pp_inst = (etw_log_inst_t *)kal_malloc_memory(sizeof(etw_log_inst_t) * comp_count);

		// Enumerate registry to find out all of LOG component setting.
		// Grep all REG_SZ type to p_enum_buf, and it includes the setting of LOG component and other setting which type is REG_SZ.
		do {
			enum_buf_sz = comp_count << idx;
			p_enum_buf = (enum_string_reg_t *) kal_malloc_memory (sizeof(enum_string_reg_t) * enum_buf_sz);
			mels_enum_registry_value(MELS_REG_KEY, p_enum_buf, &enum_buf_sz, type);

			if (enum_buf_sz < (comp_count << idx)) {
				// OK. The p_enum_buf is enough for all entry which type is REG_SZ.
				break;
			}

			// NG. The p_enum_buf is not enough for all REG_SZ type entry. Allocate larger p_enum_buf and enumerate registry value again.
			kal_free_memory(p_enum_buf);
			idx++;
		} while (enum_buf_sz < MAX_LOG_COMP_COUNT);


		if (enum_buf_sz < MAX_LOG_COMP_COUNT) {
			// OK.
			LOG(LOG_INFO, ("Looply find out COMPONENT item, comp_count=%d, MAX_LOG_COMP_COUNT=%d", comp_count, MAX_LOG_COMP_COUNT));

			for (idx=0;
				 idx < enum_buf_sz && (*p_inst_num) < comp_count;
				 idx++) {
				p_enum_tmp = p_enum_buf + idx;
				p_inst_tmp = (*pp_inst) + (*p_inst_num);

				if ( p_enum_tmp->value_name == strstr(p_enum_tmp->value_name, MELS_REG_KEY_COMPONENT_ITEM_NAME_PREFIX) ) {
					// This data is the component item.
					LOG(LOG_INFO, ("Found new COMPONENT item, name(%s) data(%s).\n", p_enum_tmp->value_name, p_enum_tmp->value_data));

					// Set default value.
					mels_set_default_value(p_inst_tmp);

					sprintf(p_inst_tmp->comp_key, "%s%s", MELS_COMP_REG_KEY, p_enum_tmp->value_data);
					p_inst_tmp->comp_key[MAX_BUF_SIZE-1] = '\0';

					// ==================================
					// Update COMP registry status to INIT.
					// ==================================
					UPDATE_MELS_COMP_STATUS(p_inst_tmp, MELS_REG_COMP_ST_INIT_OK);

					CHECK_REG_DATA(mels_read_registry(p_inst_tmp->comp_key, "LogPath", p_inst_tmp->log_path, MAX_LOG_PATH_SIZE, NULL, REG_SZ));
					CHECK_REG_DATA(mels_read_registry(p_inst_tmp->comp_key, "VendorName", p_inst_tmp->vendor_name, MAX_VENDOR_NAME_SIZE, NULL, REG_SZ));
					CHECK_REG_DATA(mels_read_registry(p_inst_tmp->comp_key, "LoggerName", p_inst_tmp->log_name, MAX_LOG_NAME_SIZE, NULL, REG_SZ));
					CHECK_REG_DATA(mels_read_registry(p_inst_tmp->comp_key, "Guid", p_inst_tmp->log_guid, MAX_LOG_GUID_SIZE, NULL, REG_SZ));

					// Don't check the return value because the default setting is used if read the following setting failed.
					mels_read_registry(p_inst_tmp->comp_key, "MaxFileSize", &(p_inst_tmp->max_file_size), sizeof(UINT32), NULL, REG_DWORD);
					mels_read_registry(p_inst_tmp->comp_key, "FileMax", &(p_inst_tmp->file_max), sizeof(UINT32), NULL, REG_DWORD);
					mels_read_registry(p_inst_tmp->comp_key, "FileCounter", &(p_inst_tmp->file_count), sizeof(UINT32), NULL, REG_DWORD);
					mels_read_registry(p_inst_tmp->comp_key, "EnableLevel", &(p_inst_tmp->log_level), sizeof(UINT32), NULL, REG_DWORD);
					mels_read_registry(p_inst_tmp->comp_key, "EnableFlags", &(p_inst_tmp->log_flags), sizeof(UINT32), NULL, REG_DWORD);
					mels_read_registry(p_inst_tmp->comp_key, "LogFileMode", &(p_inst_tmp->log_file_mode), sizeof(UINT32), NULL, REG_DWORD);
					mels_read_registry(p_inst_tmp->comp_key, "FlushTimer", &(p_inst_tmp->flush_timer), sizeof(UINT32), NULL, REG_DWORD);
					mels_read_registry(p_inst_tmp->comp_key, "Start", &(p_inst_tmp->start), sizeof(UINT32), NULL, REG_DWORD);

					// ==================================
					// Update COMP registry status to READ_REG_OK.
					// ==================================
					UPDATE_MELS_COMP_STATUS(p_inst_tmp, MELS_REG_COMP_ST_READ_REG_OK);

					if (p_inst_tmp->start) {
						(*p_inst_num) += 1;
					}
				}
			} /* for (idx=0; idx < comp_count; idx++) { */

		} else {
			// NG. The number of log component exceeds to the MAX_LOG_COMP_COUNT.
			LOG(LOG_ERROR, ("The number of log component exceeds to MAX_LOG_COMP_COUNT(%d)!\n", MAX_LOG_COMP_COUNT));

			// ==================================
			// Update registry status to COMP_NUM_IS_EXCEEDING_FAILED!
			// ==================================
			UPDATE_MELS_STATUS(MELS_REG_ST_COMP_NUM_IS_EXCEEDING_FAILED);
			
			ret = RET_CODE_LOG_COMP_NUM_IS_EXCEEDING;
			goto _failed;
		} /* if (enum_buf_sz < MAX_LOG_COMP_COUNT) */
	} else {
		// NG. Failed to read MELS_REG_KEY_COMPONENT_COUNT.
		LOG(LOG_ERROR, ("Failed to read MELS_REG_KEY_COMPONENT_COUNT(%d)!\n", comp_count));

		// ==================================
		// Update registry status to READ_REG_COMP_COUNT_FAILED!
		// ==================================
		UPDATE_MELS_STATUS(MELS_REG_ST_READ_REG_COMP_COUNT_FAILED);
		
		ret = RET_CODE_READ_KEY_FAILED;
		goto _failed;
	} /* Read configuration from registry. */

_failed:
	if (p_enum_buf) {
		kal_free_memory(p_enum_buf);
	}

	return ret;
}

mels_ret_code_t mels_check_etw_file(etw_log_inst_t *p_inst)
{
	mels_ret_code_t				ret;
	struct						_stat st;
	int							ret_create_dir;

	/*
	 * Init variable.
	 */
	ret = RET_CODE_SUCCESS;

	LOG(LOG_INFO, ("%s\n", p_inst->log_path));

	if (_stat(p_inst->log_path, &st) == 0) {
		if (S_IFDIR == (S_IFDIR & st.st_mode)) {
			// The directory of log_path is exist.
			LOG(LOG_INFO, ("Directory(%s) is exist.\n", p_inst->log_path));

			// ==================================
			// Update COMP registry status to DIR_IS_EXIST_OK
			// ==================================
			UPDATE_MELS_COMP_STATUS(p_inst, MELS_REG_COMP_ST_DIR_IS_EXIST_OK);
			
		} else {
			// NG. The directory of log_path is not a directory. RETURN.
			LOG(LOG_ERROR, ("%s is not directory!\n", p_inst->log_path));

			// ==================================
			// Update COMP registry status to LOG_PATH_IS_NOT_DIR_FAILED!
			// ==================================
			UPDATE_MELS_COMP_STATUS(p_inst, MELS_REG_COMP_ST_LOG_PATH_IS_NOT_DIR_FAILED);

			ret = RET_CODE_FILE_NOT_EXIST;
			goto _failed;
		}
	} else {
		LOG(LOG_INFO, ("%s is not exist.\n", p_inst->log_path));

		// Create directory of log_path.
		ret_create_dir = SHCreateDirectoryEx(NULL, p_inst->log_path, NULL);

		if (ERROR_SUCCESS == ret_create_dir) {
			// OK. Successfully create directory.
			LOG(LOG_INFO, ("Successfully create directory(%s).\n", p_inst->log_path));

			// ==================================
			// Update COMP registry status to CREATE_ETW_DIR_OK.
			// ==================================
			UPDATE_MELS_COMP_STATUS(p_inst, MELS_REG_COMP_ST_CREATE_ETW_DIR_OK);

		} else {
			// NG. Failed to create directory.
			LOG(LOG_ERROR, ("Failed to create directory(%s)!\n", p_inst->log_path));

			// ==================================
			// Update COMP registry status to CREATE_ETW_DIR_FAILED!
			// ==================================
			UPDATE_MELS_COMP_STATUS(p_inst, MELS_REG_COMP_ST_CREATE_ETW_DIR_FAILED);

			ret = RET_CODE_CREATE_FILE_FAILED;
			goto _failed;
		}
	}

	// <TJ_TODO> check the max file len is 248 ?
	
_failed:
	return ret;
}

mels_ret_code_t mels_start_log_session(etw_log_inst_t *p_inst)
{
	mels_ret_code_t			ret;
	INT8					exec_cmd[1024];

	/*
	 * Init variable.
	 */
	ret = RET_CODE_SUCCESS;

	/*
	 * Start log session.
	 */
	sprintf(exec_cmd, "%s \
						create trace %s_%s \
						-mode %d \
						-ln %s \
						-max %d \
						-ft %d \
						-o %s\\%s.etl.%03d \
						-p %s %x %d -f bincirc -ets",
						LOG_CMD_EXEC,
						p_inst->vendor_name, p_inst->log_name,								// {vendor_name}_{logger_name}
						p_inst->log_file_mode,												// -mode
						p_inst->log_name,													// -ln
						p_inst->max_file_size,												// -max
						p_inst->flush_timer,												// -ft
						p_inst->log_path, p_inst->log_name, p_inst->file_count,				// -o
						p_inst->log_guid, p_inst->log_flags, p_inst->log_level);			// -p

	LOG(LOG_INFO, ("mels_start_log_session: Start command(%s).\n", exec_cmd));
	ret = kal_exec_command(exec_cmd);

	if (RET_CODE_SUCCESS == ret) {
		// OK.
		LOG(LOG_INFO, ("Successfully to start session(%s).\n", exec_cmd));

		p_inst->file_count = (p_inst->file_count % p_inst->file_max) + 1;
		mels_write_registry(p_inst->comp_key, "FileCounter", &(p_inst->file_count), sizeof(UINT32), REG_DWORD);

		// ==================================
		// Update COMP registry status to START_SESSION_OK.
		// ==================================
		UPDATE_MELS_COMP_STATUS(p_inst, MELS_REG_COMP_ST_START_SESSION_OK);
	} else {
		// NG. Failed to start session!
		LOG(LOG_ERROR, ("Failed to start session(%s)!\n", exec_cmd));

		// ==================================
		// Update COMP registry status to START_SESSION_FAILED!
		// ==================================
		UPDATE_MELS_COMP_STATUS(p_inst, MELS_REG_COMP_ST_START_SESSION_FAILED);
	}
	return ret;
}

mels_ret_code_t mels_stop_log_session(etw_log_inst_t *p_inst)
{
	mels_ret_code_t			ret;
	INT8					exec_cmd[1024];

	/*
	 * Init variable.
	 */
	ret = RET_CODE_SUCCESS;

	/*
	 * Stop log session.
	 */
	sprintf(exec_cmd, "%s stop %s_%s -ets",
						LOG_CMD_EXEC,
						p_inst->vendor_name, p_inst->log_name);								// {vendor_name}_{logger_name}

	LOG(LOG_INFO, ("mels_stop_log_session: Stop command(%s).\n", exec_cmd));
	ret = kal_exec_command(exec_cmd);

	if (RET_CODE_SUCCESS == ret) {
		// OK. Successfully to stop session.
		LOG(LOG_INFO, ("Successfully to stop session(%s).\n", exec_cmd));

		// ==================================
		// Update COMP registry status to STOP_SESSION_OK.
		// ==================================
		UPDATE_MELS_COMP_STATUS(p_inst, MELS_REG_COMP_ST_STOP_SESSION_OK);
	} else {
		// NG. Failed to stop session!
		LOG(LOG_ERROR, ("Failed to stop session(%s)!\n", exec_cmd));

		// ==================================
		// Update COMP registry status to STOP_SESSION_FAILED!
		// ==================================
		UPDATE_MELS_COMP_STATUS(p_inst, MELS_REG_COMP_ST_STOP_SESSION_FAILED);
	}
	return ret;
}

mels_ret_code_t mels_exec_extended_command()
{
	mels_ret_code_t			ret;
	UINT32					ext_cmd;

	/*
	 * Init variable.
	 */
	ret = RET_CODE_SUCCESS;
	ext_cmd = 0;
	
	/*
	 * Read extended batch command configuration
	 */
	mels_read_registry(MELS_REG_KEY, MELS_REG_KEY_EXTENDED_COMMAND, &ext_cmd, sizeof(UINT32), NULL, REG_DWORD);

	if (MELS_REG_KEY_EXTENDED_COMMAND_ENABLE_CODE == ext_cmd) {
		// OK. The enable code is correct. Execute batch command
		ret = kal_exec_command(MELS_REG_KEY_EXTENDED_COMMAND_BATCH_FILE_NAME);
		LOG(LOG_INFO, ("The enable code is correct. Execute extended batch and return %d\n", ret));
	} else {
		// NG. The enable code is not correct!
		LOG(LOG_ERROR, ("The enable code(%d) is not correct!\n", ext_cmd));
		ret = RET_CODE_EXT_CMD_KEY_IS_INVALID;
	}

	return ret;
}

mels_ret_code_t mels_run_server()
{
	mels_ret_code_t		ret;
	UINT32				idx;
	etw_log_inst_t		*p_tmp;

	/*
	 * Init variable.
	 */
	ret = RET_CODE_SUCCESS;
	idx = 0;
	p_tmp = NULL;

	/*
	 * Read configuration.
	 */
	// ==================================
	// Update registry status to READ_REG_ENTRY.
	// ==================================
	UPDATE_MELS_STATUS(MELS_REG_ST_READ_REG_ENTRY);

	ret = mels_read_conf(&p_inst_g, &inst_num_g);
	MELS_ASSERT(RET_CODE_SUCCESS == ret);

	/*
	 * Check the ETW log files
	 */
	// ==================================
	// Update registry status to CHECK_ETW_FILE_ENTRY.
	// ==================================
	UPDATE_MELS_STATUS(MELS_REG_ST_CHECK_ETW_FILE_ENTRY);

	for (idx=0; idx<inst_num_g; idx++) {
		p_tmp = p_inst_g + idx;

		if (MELS_REG_COMP_ST_READ_REG_OK == p_tmp->status) {
			ret = mels_check_etw_file(p_tmp);
			MELS_ASSERT(RET_CODE_SUCCESS == ret);
		}
	}

	/*
	 * Start log session.
	 */
	// ==================================
	// Update registry status to START_LOG_SESSION_ENTRY.
	// ==================================
	UPDATE_MELS_STATUS(MELS_REG_ST_START_LOG_SESSION_ENTRY);

	for (idx=0; idx<inst_num_g; idx++) {
		p_tmp = p_inst_g + idx;

		if (MELS_REG_COMP_ST_DIR_IS_EXIST_OK == p_tmp->status ||
			MELS_REG_COMP_ST_CREATE_ETW_DIR_OK == p_tmp->status) {

			ret = mels_start_log_session(p_tmp);
			MELS_ASSERT(RET_CODE_SUCCESS == ret);

			if (RET_CODE_SUCCESS == ret) {
				// ==================================
				// Update COMP registry status to DONE.
				// ==================================
				UPDATE_MELS_COMP_STATUS(p_tmp, MELS_REG_COMP_ST_DONE);
			}

			p_tmp->start = 1;
			mels_write_registry(p_tmp->comp_key, "Start", &(p_tmp->start), sizeof(UINT32), REG_DWORD);
		}
	}

#if 0				// Don't Execute extended batch command.

	/*
	 * Run batch script.
	 */
	// ==================================
	// Update registry status to RUN_BATCH_ENTRY.
	// ==================================
	UPDATE_MELS_STATUS(MELS_REG_ST_RUN_BATCH_ENTRY);

	ret = mels_exec_extended_command();
	MELS_ASSERT(RET_CODE_SUCCESS == ret);

	if (RET_CODE_SUCCESS == ret) {
		// OK.
		UPDATE_MELS_STATUS(MELS_REG_ST_RUN_BATCH_OK);
	} else {
		// NG. Failed to execute batch file.
		UPDATE_MELS_STATUS(MELS_REG_ST_RUN_BATCH_FAILED);
	}

	/*
	 * Done.
	 */
	// ==================================
	// Update registry status to DONE.
	// ==================================
	if (RET_CODE_SUCCESS == ret) {
		UPDATE_MELS_STATUS(MELS_REG_ST_DONE);
	}

#else

	/*
	 * Done.
	 */
	// ==================================
	// Update registry status to DONE.
	// ==================================
	UPDATE_MELS_STATUS(MELS_REG_ST_DONE);

#endif

	return ret;

}

mels_ret_code_t mels_continue_server()
{
	mels_ret_code_t		ret;
	UINT32				idx;
	etw_log_inst_t		*p_tmp;

	/*
	 * Init variable.
	 */
	ret = RET_CODE_SUCCESS;
	idx = 0;
	p_tmp = NULL;

	/*
	 * Read configuration.
	 */
	// ==================================
	// Update registry status to READ_REG_ENTRY.
	// ==================================
	UPDATE_MELS_STATUS(MELS_REG_ST_READ_REG_ENTRY);

	ret = mels_read_conf(&p_inst_g, &inst_num_g);
	MELS_ASSERT(RET_CODE_SUCCESS == ret);

	/*
	 * Check the ETW log files of new component.
	 */
	// ==================================
	// Update registry status to CHECK_ETW_FILE_ENTRY.
	// ==================================
	UPDATE_MELS_STATUS(MELS_REG_ST_CHECK_ETW_FILE_ENTRY);

	for (idx=0; idx<inst_num_g; idx++) {
		p_tmp = p_inst_g + idx;

		if (MELS_REG_COMP_VALUE_NEW_INSTALLED == p_tmp->start) {
			// The new installed logger component. MELS needs to check ETW file of this new installed logger.
			if (MELS_REG_COMP_ST_READ_REG_OK == p_tmp->status) {
				ret = mels_check_etw_file(p_tmp);
				MELS_ASSERT(RET_CODE_SUCCESS == ret);
			}
		} else {
			// This logger component is started before. Do nothing.

			if (MELS_REG_COMP_ST_READ_REG_OK == p_tmp->status) {
				ret = RET_CODE_SUCCESS;

				// ==================================
				// Update COMP registry status to DIR_IS_EXIST_OK
				// ==================================
				UPDATE_MELS_COMP_STATUS(p_tmp, MELS_REG_COMP_ST_DIR_IS_EXIST_OK);
			}
		}
	}

	/*
	 * Start log session of new component.
	 */
	// ==================================
	// Update registry status to START_LOG_SESSION_ENTRY.
	// ==================================
	UPDATE_MELS_STATUS(MELS_REG_ST_START_LOG_SESSION_ENTRY);

	for (idx=0; idx<inst_num_g; idx++) {
		p_tmp = p_inst_g + idx;

		if (MELS_REG_COMP_VALUE_NEW_INSTALLED == p_tmp->start) {
			if (MELS_REG_COMP_ST_DIR_IS_EXIST_OK == p_tmp->status ||
				MELS_REG_COMP_ST_CREATE_ETW_DIR_OK == p_tmp->status) {

				ret = mels_start_log_session(p_tmp);
				MELS_ASSERT(RET_CODE_SUCCESS == ret);

				if (RET_CODE_SUCCESS == ret) {
					// ==================================
					// Update COMP registry status to DONE.
					// ==================================
					UPDATE_MELS_COMP_STATUS(p_tmp, MELS_REG_COMP_ST_DONE);
				}

				p_tmp->start = 1;
				mels_write_registry(p_tmp->comp_key, "Start", &(p_tmp->start), sizeof(UINT32), REG_DWORD);
			}
		} else {
			// This logger component is started before. Do nothing.
		
			if (MELS_REG_COMP_ST_DIR_IS_EXIST_OK == p_tmp->status ||
				MELS_REG_COMP_ST_CREATE_ETW_DIR_OK == p_tmp->status) {

				ret = RET_CODE_SUCCESS;

				// ==================================
				// Update COMP registry status to DONE.
				// ==================================
				UPDATE_MELS_COMP_STATUS(p_tmp, MELS_REG_COMP_ST_DONE);
			}
		}
	}

	/*
	 * Set Service Status to RUNNING.
	 */
	ServiceStatus_g.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus (hServiceStatus_g, &ServiceStatus_g);

	/*
	 * Done.
	 */
	// ==================================
	// Update registry status to DONE.
	// ==================================
	if (RET_CODE_SUCCESS == ret) {
		UPDATE_MELS_STATUS(MELS_REG_ST_DONE);
	}

	return ret;

}

mels_ret_code_t mels_stop_server()
{
	mels_ret_code_t		ret;
	UINT32				idx;
	etw_log_inst_t		*p_tmp;

	LOG(LOG_INFO, ("mels_stop_server: inst_num=%d\n", inst_num_g));

	for (idx=0; idx<inst_num_g; idx++) {
		p_tmp = p_inst_g + idx;

		ret = mels_stop_log_session(p_tmp);
		MELS_ASSERT(RET_CODE_SUCCESS == ret);
	}
	
	if (p_inst_g) {
		kal_free_memory(p_inst_g);
		inst_num_g = 0;
	}

	return RET_CODE_SUCCESS;
}


/*----------------------------------------------------------------------
 * Public implementation.
 *--------------------------------------------------------------------*/
DWORD WINAPI ServiceCtrlHandler(
        DWORD Opcode,
        DWORD evtype,
        PVOID evdata,
        PVOID Context)
{

    UNREFERENCED_PARAMETER(evtype);
    UNREFERENCED_PARAMETER(evdata);
    UNREFERENCED_PARAMETER(Context);

    LOG(LOG_FUNC, ("===> ServiceCtrlHandler\n"));

    switch(Opcode)
    {
        case SERVICE_CONTROL_PAUSE:
			LOG(LOG_INFO, ("ServiceCtrlHandler: SERVICE_CONTROL_PAUSE\n"));
            ServiceStatus_g.dwCurrentState = SERVICE_PAUSED;
			SetServiceStatus (hServiceStatus_g, &ServiceStatus_g);
            break;

        case SERVICE_CONTROL_CONTINUE:
			LOG(LOG_INFO, ("ServiceCtrlHandler: SERVICE_CONTROL_CONTINUE\n"));
            ServiceStatus_g.dwCurrentState = SERVICE_CONTINUE_PENDING;
			SetServiceStatus (hServiceStatus_g, &ServiceStatus_g);
            break;

        case SERVICE_CONTROL_STOP:
			LOG(LOG_INFO, ("ServiceCtrlHandler: SERVICE_CONTROL_STOP\n"));
            ServiceStatus_g.dwCurrentState = SERVICE_STOPPED;
            SetServiceStatus (hServiceStatus_g, &ServiceStatus_g);
            break;

		default:
			LOG(LOG_INFO, ("ServiceCtrlHandler: Opcode(%d)\n", Opcode));
			break;
    }

	SetEvent(mels_event_g);

	LOG(LOG_FUNC, ("<=== ServiceCtrlHandler\n"));
    return 0;
}

void WINAPI mels_service_main(DWORD argc, LPTSTR *argv)
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

	LOG(LOG_FUNC, ("===> mels_service_main\n"));

	/*
	 * Init variable.
	 */
    ServiceStatus_g.dwServiceType = SERVICE_WIN32;
    ServiceStatus_g.dwCurrentState = SERVICE_RUNNING;
    ServiceStatus_g.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
    ServiceStatus_g.dwWin32ExitCode = 0;
    ServiceStatus_g.dwServiceSpecificExitCode = 0;
    ServiceStatus_g.dwCheckPoint = 0;
    ServiceStatus_g.dwWaitHint = 0;

    hServiceStatus_g = RegisterServiceCtrlHandlerEx(MELS_SERVICE_NAME,
													ServiceCtrlHandler,
													NULL);
    if (!hServiceStatus_g) {
		LOG(LOG_ERROR, ("mels_service_main: Register service control handler failed with %d.\n", GetLastError()));
		goto _exit;
    }

	mels_event_g = CreateEvent(NULL,				// lpEventAttributes
								FALSE,				// bManualReset
								FALSE,				// bInitialState
								NULL);				// lpName
	if (!mels_event_g) {
		LOG(LOG_ERROR, ("mels_service_main: CreateEvent() failed with %d.\n", GetLastError()));
		goto _exit;
	}

    SetServiceStatus(hServiceStatus_g, &ServiceStatus_g);

	LOG(LOG_INFO, ("mels_service_main: Starting server\n"));
	mels_run_server();

	while (1) {
		LOG(LOG_INFO, ("mels_service_main: Wait for new service event.\n"));
		WaitForSingleObject(mels_event_g, INFINITE);

		if (SERVICE_PAUSED == ServiceStatus_g.dwCurrentState) {
			LOG(LOG_INFO, ("mels_service_main: Service is paused, exit.\n"));
			break;
		} else if (SERVICE_CONTINUE_PENDING == ServiceStatus_g.dwCurrentState) {
			LOG(LOG_INFO, ("mels_service_main: Service is contined, only start new component.\n"));
			mels_continue_server();
		} else if (SERVICE_STOPPED == ServiceStatus_g.dwCurrentState) {
			LOG(LOG_INFO, ("mels_service_main: Service is stopped, stop all logger session and exit.\n"));
			mels_stop_server();
			LOG(LOG_INFO, ("mels_service_main: Service is stopped2, stop all logger session and exit.\n"));
			break;
		} else {
			LOG(LOG_INFO, ("mels_service_main: Service status is %d, continue.\n", ServiceStatus_g.dwCurrentState));
		}
	}

    ServiceStatus_g.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(hServiceStatus_g, &ServiceStatus_g);

#if defined ENABLE_LOG_TO_FILE
	mels_log_deinit();
#endif

	LOG(LOG_FUNC, ("<=== mels_service_main\n"));

_exit:
	return ;
}

void showUsage()
{
    mels_dbg_print("Available command-line options are:\n");
    mels_dbg_print("  -help                  - Show this message\n");
	mels_dbg_print("  -service               - Register service to Service Control Manager\n");
    mels_dbg_print("  -server                - Standalone server mode\n");
    mels_dbg_print("\n");
}

SERVICE_TABLE_ENTRY srv_dispatch_table_s[] = {
    {MELS_SERVICE_NAME, mels_service_main},
    {0, 0}};

int __cdecl
main(int argc, const char* argv[]) 
{
    if (argc < 2) {
        showUsage();

	} else if (_stricmp(argv[1], "-service") == 0) {
		StartServiceCtrlDispatcher(srv_dispatch_table_s);

    } else if (_stricmp(argv[1], "-server") == 0) {
        mels_run_server();

#if defined ENABLE_LOG_TO_FILE
		mels_log_deinit();
#endif

    } else {
        showUsage();

    }
}

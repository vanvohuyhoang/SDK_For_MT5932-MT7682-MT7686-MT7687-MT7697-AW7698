/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
 * Without the prior written permission of Airoha and/or its licensors,
 * any reproduction, modification, use or disclosure of Airoha Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
 * if you have agreed to and been bound by the applicable license agreement with
 * Airoha ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of Airoha Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
 */


#ifndef __FOTA_FLASH_FOR_GSOUND_H__
#define __FOTA_FLASH_FOR_GSOUND_H__

#include "fota_flash.h"
#include "fota_platform.h"
#include "fota_util.h"


#ifdef __FOTA_FOR_BISTO__
/**
 * The api.h is used for gsound.
 *
 * @section fota api_usage How to use this module
 * - Query current device fota sector firstly.
 *  - Step 1. Mandatory,call #fota_flash_alloc_partition_info(&type, &addr, size) to query current fota sector start address and current flash type.
 *     If returned value is FOTA_ERRCODE_SUCCESS, the next steps will be meaning.If returned value is FOTA_ERRCODE_FAIL, please not do any other steps.
 * - Sample code:
 *    @code
 *			uint32_t addr;
 *			FotaStorageType type;
 *			FOTA_ERRCODE ret = fota_flash_alloc_partition_info(&type, &addr, 3*1024*1024);
 *		if (ret == FOTA_ERRCODE_SUCCESS) {
 *				FOTA_LOG_I("query_success, type : %d, addr : %x", type, addr);
 *			} else {
 *			    FOTA_LOG_I("query_fail, type : %d, addr : %x", type, addr);
 *			}
 *     @endcode
 * - Download new fota package bin from SP.
 *  - Step 2. After get the partition info successfully, it will call the function #fota_start_transaction() to mark start download new fota package bin flag, the whole download, it only call once.
 *  - Step 3. After successfully, it start erase fota sector, it will call the function #fota_erase(addr, type),it should be called many time, because it only erase 4k once.
 *  - Step 4. start write data to fota sector,it will call the function #fota_download(addr, &data, length, type).
 *   - Sample code:
 *     @code
 *		FOTA_ERRCODE ret = fota_start_transaction();
 *		if (ret == FOTA_ERRCODE_SUCCESS) {
 *          uint32_t addr = 0x0022b000;
 *          FotaStorageType type = InternalFlash;
 *	 		ret = fota_erase(addr, type);
 *		    if (ret == FOTA_ERRCODE_SUCCESS) {
 *				uint8_t data[32] = {1,2,3,4,5,6,0};
 *              ret = fota_download(addr, &data, length, type);
 *              if (ret == FOTA_ERRCODE_SUCCESS) {
 *              	FOTA_LOG_I("Download success");
 *              }
 *          }
 *		}
 *    @endcode
 *  -Step 5. after Download completed, Call the function #fota_notify_download_done() to notify device to do integrity check and set upgrade flag.
 *   - Sample code:
 *    @code
 *      FOTA_ERRCODE ret = fota_notify_download_done();
 *      if (ret == FOTA_ERRCODE_SUCCESS) {
 *         FOTA_LOG_I("notify sucecss");
 *      }
 *    @endcode
 * - Get version.
 *  - Step 1.Call the function #fota_get_fw_version(&verion, type) to get current version or new version in fota package bin.
 *   - Sample code:
 *    @code
 *      uint8_t ver[32] = {0};
 *      Imageversiontype type = newversion;
 *      FOTA_ERRCODE ret = fota_get_fw_version(&ver, type);
 *      if (ret == FOTA_ERRCODE_SUCCESS) {
 *	        char buf[32];
 *	        for(int i=0;i < 32;i++) {
 *		 		FOTA_LOG_I("GET_VERSION, i = %d, data(%x)", i, ver[i]);
 *	 			if (ver[i] != NULL)
 *			 		buf[i]=ver[i];
 *		 	else {
 *			 	break;
 *		 	}
 *	  	}
 *	 		FOTA_LOG_I("GET_VERSION(%s)", buf);
 *      } else {
 *	 		FOTA_LOG_I("get version fail");
 *   }
 *    @endcode
 * - Do upgrade.
 *  - After download fota package bin completely, it should do upgrade.
 *  - Step 1. Call the function #fota_reboot() to reboot device, device will auto run upgrade flow.
 *   - Sample code:
 *    @code
 *        fota_reboot();
 *    @endcode
 */

typedef enum
{
    currversion = 0,
    newversion = 1,
} Imageversiontype;

/**
 * @brief                 	This function get partition info.
 *                        	with the connection request result.
 * @param[out] address     	is the start address of fota.
 * @param[out] storage_type current support storage type internal flash or external flash.
 * @param[in]  size        	is the wanted alloc size.
 * @return                	#FOTA_ERRCODE_SUCCESS, the allco request successfully.
 *                        	#FOTA_ERRCODE_FAILD, if size > support size or current snoy fota is doing.
 */
FOTA_ERRCODE fota_flash_alloc_partition_info(FotaStorageType *storage_type, uint32_t *address,uint32_t size);

/**
 * @brief                 	This function get firmware version.
 * @param[out] version     	is the version of new image, version length is 28 bytes, so user need alloc buffer.
 * @param[in] version_type  is current run bin version or new version in fota package.
 * @return                	#FOTA_ERRCODE_SUCCESS, get version successfully.
 * 							#FOTA_ERRCODE_FAILD, get version failed.
 */
FOTA_ERRCODE fota_get_fw_version(uint8_t *version, Imageversiontype version_type);

/**
 * @brief                	This function is called before start download fota, it used to set flag & init .
 * @return                	#FOTA_ERRCODE_SUCCESS, get version successfully.
 * 							#FOTA_ERRCODE_FAILD, get version failed.
 */
FOTA_ERRCODE fota_start_transaction(void);

/**
 * @brief                	This function earase the spec address, erase size is once 4k .
 * @param[in] address    	erase start address.
 * @param[in] storage_type  internal flash or external flash.
 * @return                	#FOTA_ERRCODE_SUCCESS, fota erase successfully.
 * 							#FOTA_ERRCODE_FAILD, fota erase failed.
 */
FOTA_ERRCODE fota_erase(uint32_t address, FotaStorageType storage_type);
/**
 * @brief                	This function write data to the spec address.
 * @param[in] address    	the download address.
 * @param[in] data       	the data.
 * @param[in] length     	the length of data.
 * @param[in] storage_type 	internal flash or external flash.
 * @return                #FOTA_ERRCODE_SUCCESS, fota download successfully.
 *                        #FOTA_ERRCODE_FAILD, fota download fail.
 */
FOTA_ERRCODE fota_download(uint32_t address, const uint8_t *data, uint32_t length, FotaStorageType storage_type);
/**
* @brief                 This function is used to do integrity check & flag set after download done.
* @return                #FOTA_ERRCODE_SUCCESS, the request successfully.
*                        #FOTA_ERRCODE_FAILD, the integrity fail, or flag set fail.
*/
FOTA_ERRCODE fota_notify_download_done(void);
/**
 * @brief                This function reboot device.
 */
void fota_reboot();

/**
* @brief                 This function query result upgrade of new firmware.
* @param[out] result	 The value has two case, 0x101: means upgrade success, 0xffff: means init case .
* @return                #FOTA_ERRCODE_SUCCESS, the request successfully.
*                        #FOTA_ERRCODE_FAILD, query result fail.
*/
FOTA_ERRCODE fota_query_upgrade_result(uint16_t *result);


/**
* @brief                 This function sets Bisto FOTA's runinng status.
* @param[in] is_running    TRUE: Bisto FOTA is running; FALSE: Bisto fota is not running.
*/
void fota_set_bisto_fota_running(bool is_running);


/**
* @brief                 This function returns if the running status of Bisto FOTA.
* @return               TRUE: Bisto FOTA is running;
*                            FALSE: Bisto fota is not running.
*/
bool fota_is_bisto_fota_running(void);
#endif
#endif /* __FOTA_FLASH_FOR_GSOUND_H__ */


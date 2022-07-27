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
 
#ifndef __HAL_SW_DMA_H__
#define __HAL_SW_DMA_H__
#include "hal_platform.h"

#ifdef HAL_SW_DMA_MODULE_ENABLED
#include "stdio.h"
/**
 * @addtogroup HAL
 * @{
 * @addtogroup SW_DMA
 * @{
 * This section introduces the Software General Direct Memeory Access(SW_DMA) APIs including terms and acronyms,
 * supported features, software architecture, details on how to use this driver, SW_DMA function groups, enums, structures and functions.
 *
 * @section HAL_SW_DMA_Terms_Chapter Terms and acronyms
 *
 * |Terms                   |Details                                                                 |
 * |------------------------------|------------------------------------------------------------------------|
 * |\b GDMA                | General Direct Memeory Access. GDMA is an operation in which data is copied (transported) from one resource to another in a computer system without the involvement of the CPU.
 *  For more information, please refer to <a href="https://en.wikipedia.org/wiki/Direct_memory_access"> GDMA in Wikipedia </a>.
 * @section SW_DMA_Features_Chapter Supported features
 *
 * - \b Support \b interrupt \b mode
 *   GDMA can copy data from source memory to destination memory, instead of from memory to I/O-device or I/O-device to memory. SW_DMA allows users to
 *   start up to #SW_DMA_USER_NUM GDMA transactions even when none of them have completed the data transfer. When each GDMA transaction is complete, the related
 *   user's callback will be called to inform the specific user.\n
 *  - \b Interrupt \b mode: In this mode, the GDMA hardware generates an interrupt when the transaction is complete. Then the SW_DMA driver will
 *       check if the user has any other need to use the GDMA hardware and call the callback function of the completed transaction user.\n
 *   \n
 * @section SW_DMA_Driver_Usage_Chapter How to use this driver
 *
 * - The user only needs to prepare the related parameters. For more information about the parameters, please see #hal_sw_dma_config_info_t, then call #hal_sw_gdma_start().
 *  - Sample code:
 *    @code
 *    Note 1. To use DMA:
 *           - Memory should be non-cacheable.
 *           - Memory address and length should match below align requirement.
 *      |---------------------------------------------------------------------------|
 *      |                         Address and length alignment requirement          |
 *      |---------------------------------------------------------------------------|
 *      |h_size/burst                   |single                 |4-beat burst       |
 *      |---------------------------------------------------------------------------|
 *      |byte (h_size = 0)              |1 byte align           |4 bytes align      |
 *      |half-word (h_size = 1)         |2 bytes align          |8 bytes align      |
 *      |word (h_size = 2)              |4 bytes align          |16 bytes align     |
 *      |---------------------------------------------------------------------------|
 *    Note 2. To get non-cacheable memory:
 *           - Define a global data array and add a memory attribute.
 *                      ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN, if the array has no initial value.
 *                      ATTR_RWDATA_IN_NONCACHED_RAM_4BYTE_ALIGN, if the array has an initial value.
 *             Example: ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint8_t  source_address[data_length];
 *                      ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint8_t  destination_address[data_length];
 *            - Apply memory dynamically with an API void* PvPortMallocNC(size_t xWantedSize).
 *             Example:  char *source_address= PvPortMallocNC(data_length);
 *                       char *destination_address= PvPortMallocNC(data_length);
 *
 *       static uint32_t dma_length = 64;
 *       ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint8_t  source_address[64];
 *       ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint8_t  destination_address[64];
 *       hal_sw_dma_config_info_t temp_dma_cfg;
 *       hal_sw_gdma_status_t status;
 *       temp_dma_cfg.source_address = source_address;
 *       temp_dma_cfg.destination_address = destination_address;
 *       temp_dma_cfg.length = dma_length;
 *       temp_dma_cfg.func = dma_irq_handle;
 *       temp_dma_cfg.argument = NULL;
 *       temp_dma_cfg.h_size = HAL_SW_DMA_WORD;
 *       temp_dma_cfg.dma_type = HAL_SW_DMA_NORMAL_MODE;
 *
 *       status = hal_sw_gdma_start(&temp_dma_cfg);
 *       if (status != HAL_SW_DMA_STATUS_OK) {
 *           // Error handling.
 *       }
 *
 *    @endcode
 *    @code
 *       // Callback function.
 *         static void dma_irq_handle(hal_sw_dma_event_t event, void  *user_data)
 *         {
 *            //callback will be triggered when gdma transfers finished.
 *            //add your code here
 *               printf("\r\n ---sw_dma_example finished!!!---\r\n");
 *         }
 *
 *    @endcode
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup hal_sw_dma_enum Enum
  * @{
  */
/** @brief SW_DMA status */
typedef enum {
    HAL_SW_DMA_STATUS_BUFFER_FULL         = -4,         /**< SW_DMA buffer full */
    HAL_SW_DMA_STATUS_ERROR               = -3,         /**< SW_DMA function ERROR */
    HAL_SW_DMA_STATUS_ERROR_HANDLE        = -2,         /**< SW_DMA error handle */
    HAL_SW_DMA_STATUS_INVALID_PARAMETER   = -1,         /**< SW_DMA error invalid parameter */
    HAL_SW_DMA_STATUS_OK                  = 0,          /**< SW_DMA function OK*/
} hal_sw_gdma_status_t;

/** @brief SW_DMA transaction status */
typedef enum {
    HAL_SW_DMA_EVENT_TRANSACTION_ERROR    = -1,         /**<SW_DMA transaction error */
    HAL_SW_DMA_EVENT_TRANSACTION_SUCCESS  = 0,          /**<SW_DMA transaction success */
} hal_sw_dma_event_t;

/** @brief SW_DMA transaction type */
typedef enum {
    HAL_SW_DMA_SOURCE_AND_DESTINATION_ADDRESS_FIXED_MODE = 0x0,   /**< source and destination addresses fixed */
    HAL_SW_DMA_DESTINATION_ADDRESS_FIXED_MODE = 0x1,              /**< destination addresses fixed and source addresses increments after each transfer */
    HAL_SW_DMA_SOURCE_ADDRESS_FIXED_MODE = 0x2,                   /**< source addresses fixed and destination addresses increments after each transfer */
    HAL_SW_DMA_NORMAL_MODE = 0x3,                                 /**< source and destination addresses increments after each transfer */
} hal_sw_dma_type_t;

/** @brief SW_DMA transfer size format */
typedef enum {
    HAL_SW_DMA_BYTE      = 0,            /**< SW_DMA transfer size -byte format */
    HAL_SW_DMA_HALF_WORD = 1,            /**< SW_DMA transfer size -half word format */
    HAL_SW_DMA_WORD      = 2             /**< SW_DMA transfer size -word format */
} hal_sw_dma_size_format_t;

/** @brief SW_DMA transfer size format */
typedef enum {
    HAL_SW_DMA_SINGLE_BURST = 0x0,       /**< SW_DMA transfer type - single burst */
    HAL_SW_DMA_4_BEAT_BURST = 0x2        /**< SW_DMA transfer type - 4-beat burst */
} hal_sw_dma_transfer_type_t;

/**
  * @}
  */

/** @defgroup hal_sw_dma_typedef Typedef
   * @{
   */

/** @brief  This defines the callback function prototype.
 *          Register a callback function when in the interrupt mode, this function will be called in interrupt
 *          service routine after a transaction is complete.
 *  @param [in] event is the transaction event for the current transaction, application can get the transaction result from this parameter.
 *              For more details about the event type, please refer to #hal_sw_dma_event_t.
 *  @param [in] user_data is a reserved parameter.
 */
typedef void (*hal_sw_dma_callback_t)(hal_sw_dma_event_t event, void  *user_data);
/**
  * @}
  */

/** @defgroup hal_sw_dma_struct Struct
   * @{
   */
/** @brief SW_DMA configuration setting structure. */
typedef struct {
    uint32_t source_address;                    /**< The source address for the DMA operation. */
    uint32_t destination_address;               /**< The destination address for the DMA operation. */
    uint32_t length;                            /**< The data length for the DMA operation, uint is byte. If h_size is 0, the largest length is 0xFFFF (byte), if h_size is 2, the largest length is 0xFFFF*4 (byte). */
    hal_sw_dma_callback_t func;                 /**< The callback function to notify user that DMA data transaction is finished. */
    hal_sw_dma_type_t dma_type;                 /**< The address operation for source and destination address, details please see #hal_sw_dma_type_t. */
    hal_sw_dma_size_format_t h_size;            /**< The setting for the DMA transfer size, details please see #hal_sw_dma_size_format_t. */
    hal_sw_dma_transfer_type_t transfer_type;   /**< The setting for the DMA transfer type, details please see #hal_sw_dma_transfer_type_t. */
    void *argument;                             /**< The parameter is for user's self-defined usage. */
} hal_sw_dma_config_info_t;
/**
  * @}
  */

/**
 * @brief     This function enables the SW_DMA to operate in the interrupt mode, after data transaction is finished, the registered callback function will be called to notify user.
 * @param[in] info The configuration setting information for the SW_DMA operation, details please see #hal_sw_dma_config_info_t.
 * @return   To indicate whether this function call is successful or not.
 *           #HAL_SW_DMA_STATUS_OK, the operation completed successfully.
 *           #HAL_SW_DMA_STATUS_BUFFER_FULL, the SW_DMA buffer is full, no available space for another user.
 * @par       Example
 * Sample code, please refer to @ref SW_DMA_Driver_Usage_Chapter.
 */
hal_sw_gdma_status_t hal_sw_gdma_start(hal_sw_dma_config_info_t *info);

#ifdef __cplusplus
}
#endif


/**
* @}
* @}
*/

#endif /*HAL_SW_DMA_MODULE_ENABLED*/
#endif /* __HAL_SW_DMA_H__ */


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
 
#ifndef __HAL_CAPTOUCH_H__
#define __HAL_CAPTOUCH_H__

#include "hal_platform.h"


#ifdef HAL_CAPTOUCH_MODULE_ENABLED
#include "hal_keypad_table.h"

/**
 * @addtogroup HAL
 * @{
 * @addtogroup CAPTOUCH
 * @{
 * This section introduces the captouch driver APIs including terms and acronyms, supported features,
 * software architecture, details on how to use this driver, enums, structures and functions.

 * @section HAL_CAPTOUCH_Terms_Chapter Terms and acronyms
 *
 *
 * |Terms                         |Details                                                                 |
 * |------------------------------|------------------------------------------------------------------------|
 * |\b GPIO                       | For an introduction to General Purpose Inputs-Outputs, please refer to the @ref GPIO module in HAL. |
 * |\b NVIC                       | The Nested Vectored Interrupt Controller (NVIC) is the interrupt controller of ARM Cortex-M. For more details, please refer to <a href="http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.100166_0001_00_en/ric1417175922867.html"> NVIC introduction in ARM Cortex-M4 Processor Technical Reference Manual </a>.|
 * |\b Captouch                   | Capacitive touch controler is used for human interfacing. When a human finger touches the sensing part, usually a part of system HW, it forms a capacitance where one plate is the conductive layer and the other being the human finger.|
 * |\b EPT                        | Easy Pinmux Tool is to configure the GPIO source table.|
 * @section HAL_CAPTOUCH_Features_Chapter Supported features
 *
 * This module provides a generic design to get external touch events.
 * The captouch provides two ways to use, the normal usage as below, by the detail description of automatic calibration usage.
 *
 * - \b Normal \b usage.\n
 *  In this usage, an interrupt is triggered whenever a pad is pressed or released.
 *  A callback function can be registered for the interrupt. The callback function is invoked whenever a pad is pressed or released.
 *  #hal_captouch_get_event() can be used in the callback function to get the key event and the key position number.
 *  #hal_captouch_translate_channel_to_symbol() can be used to translate the key position number to the EPT key definition symbol name.\n
 * - \b tunning \b usange.\n
 *  In this usage, the user can tune the coarse cap by #hal_captouch_tune_control().\n
 * @}
 * @}
 */

/**
 * @addtogroup HAL
 * @{
 * @addtogroup CAPTOUCH
 * @{
 * @section HAL_CAPTOUCH_Driver_Usage_Chapter How to use this driver
 *
 * - \b Using \b the \b normal.\n
 *    To use the catpouch driver in normal mode, please refer to @ref GPIO datasheet to determine which GPIOs should be selected to pinmux to captouch pins.
 *    Note that the channel0 is controled by @ref RTC, and if you use the EPT tool to configure the captouch pin settings, there is no need to configure the captouch GPIO pinmux anymore.
 *    Then call #hal_captouch_init() to manually set the channel bit map, register the callback function, the default coarse cap and the threshold value.
 *    After calling #hal_captouch_init(), the driver can work.
 *    If a key is pressed or released on the pad, the pad triggers an interrupt to call the callback function.
 *    The user should use #hal_captouch_get_event() in the callback function to get the key event and the key position number.
 *    To ensure a reliable key scan, do not overload the callback function. Let the callback return as quickly as possible.
 *  - Step1. Call #hal_captouch_init() to initialize the captouch module.
 *  - Step2. Call #hal_captouch_deinit() to de-initialize the captouch module if it is no longer in use.
 *  - Sample code:
 *    @code
 *      hal_captouch_config_t config;
 *      hal_captouch_tune_data_t tune_data;
 *      uint32_t i;
 *
 *      config.callback.callbck  = user_captouch_callback;           // Register callback
 *      config.callback.userdata = NULL;                             // User's callback parameter pointer
 *      config.channel_bit_map   = 0xff;                             // Sets the channel0~7 enable
 *      for(i=0;i<8;i++) {
 *          config.high_thr[i] = 210;                                // Sets the high threshold value
 *          config.low_thr[i]  = 100;                                // Sets the low threshold value
 *          config.coarse_cap[i] = 2;                                // Sets the coarse cap value
 *      }
 *      hal_captouch_init(&config);
 *
 *      ...
 *      hal_captouch_deinit();                                       // De-initialize the captouch module if it is no longer in use.
 *    @endcode
 *    @code
 *       // Callback function. This function should be registered with #hal_captouch_init().
 *       void user_captouch_callback (void *user_data)
 *       {
 *          // Get the key press or release event.
 *          hal_captouch_event_t key_event;
 *          uint32_t key_symbol;
 *          hal_captouch_status_t ret;
 *
 *          while(1) {
 *              ret = hal_captouch_get_event(&key_event);
 *              if (ret == HAL_CAPTOUCH_STATUS_NO_EVENT) {
 *                  break;
 *              }
 *              // Translate the key position to EPT symbol name for application if necessary.
 *              hal_captouch_translate_channel_to_symbol(key_event.key_data,&key_symbol);
 *           }
 *       }
 *
 *    @endcode
 *
 * - \b Using \b the \b tunning \b usage.\n
 *    Then tunning usage is to find the proper coarse cap value by channel.
 *  - Step1. Call #hal_captouch_init() to initialize the captouch module.
 *    Step2. Call #hal_captouch_tune_control() to find the proper coarse cap value.
 *  - Step3. Call #hal_captouch_deinit() to de-initialize the captouch module if it is no longer in use.
 *  - Sample code:
 *    @code
 *      hal_captouch_config_t config;
 *      hal_captouch_tune_data_t tune_data;
 *      uint32_t i;
 *
 *      config.callback.callbck  = user_captouch_callback;           // Register callback
 *      config.callback.userdata = NULL;                             // User's callback parameter pointer
 *      config.channel_bit_map   = 0xff;                             // Sets the channel0~7 enable
 *      for(i=0;i<8;i++) {
 *          config.high_thr[i] = 210;                                // Sets the high threshold value
 *          config.low_thr[i]  = 100;                                // Sets the low threshold value
 *          config.coarse_cap[i] = 2;                                // Sets the coarse cap value
 *      }
 *      hal_captouch_init(&config);
 *
 *      for(i=0;i<8;i++) {
 *          // The tune_data can be stored in memory, and filled by #hal_captouch_init().
 *          hal_captouch_tune_control(i,HAL_CAPTOUCH_TUNE_HW_AUTO,&tune_data);  // If tuning the 8 channels.
 *      }
 *      ...
 *      hal_captouch_deinit();                                       // De-initialize the captouch module if it is no longer in use.
 *    @endcode
 * @}
 * @}
 */


/**
 * @addtogroup HAL
 * @{
 * @addtogroup CAPTOUCH
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup hal_captouch_typedef Typedef
  * @{
  */

/** @brief  This function registers a callback function when in a normal usage.
 *          This function is called after the key is pressed or released in the captouch ISR routine.
 * @param [in] user_data: This variable pointer is defined by the user to record the data.
 */
typedef void (*hal_captouch_callback_t)(void *user_data);


/**
  * @}
  */

/** @defgroup hal_keypad_enum Enum
  * @{
  */



/** @brief This enum defines captouch status */
typedef enum {
    HAL_CAPTOUCH_STATUS_UNEXPECTED_ERROR  = -7,      /**< An unexpected error. */
    HAL_CAPTOUCH_STATUS_CHANNEL_ERROR     = -6,      /**< The channel number error. */
    HAL_CAPTOUCH_STATUS_NO_EVENT          = -5,      /**< No event in buffer. */
    HAL_CAPTOUCH_STATUS_UNINITIALIZED     = -4,      /**< The driver is uninitialized. */
    HAL_CAPTOUCH_STATUS_INITIALIZED       = -3,      /**< The driver is initialized. */
    HAL_CAPTOUCH_STATUS_ERROR             = -2,      /**< A function error. */
    HAL_CAPTOUCH_STATUS_INVALID_PARAMETER = -1,      /**< An invalid parameter. */
    HAL_CAPTOUCH_STATUS_OK   = 0                     /**< No error occurred during the function call. */
} hal_captouch_status_t;

/** @brief This enum defines key status */
typedef enum {
    HAL_CAP_TOUCH_KEY_RELEASE             = 0,        /**< A key is released. */
    HAL_CAP_TOUCH_KEY_PRESS               = 1,        /**< A key is pressed. */
    HAL_CAP_TOUCH_KEY_OPERATION_MAX
} hal_captouch_key_state_t;
/**
  * @}
  */

/** @defgroup hal_captouch_struct Struct
  * @{
  */


/** @brief This structure defines callback parameter */
typedef struct {
    hal_captouch_callback_t callback;                       /**< The callback function.*/
    void *user_data;                                        /**< The callback function parameter pointer.*/
} hal_captouch_callback_context_t;

/** @brief This structure defines configuration parameter */
typedef struct {
    hal_captouch_callback_context_t callback;               /**< The callback parameter.*/
    uint8_t  channel_bit_map;                               /**< The channel bit map; one bit specifies one channel. */
    uint8_t  mavg_r;                                        /**< Number of samples to be averaged when calibrating channel capacitance. The actual number 2^mavg_r. 
                                                                 This value range is 0~15. If this value is filled with 0xff. It means use the default value 10.*/
    uint8_t  avg_s;                                         /**< Weight of new sample when computing ADC output moving average. 
                                                                 This value range is 0~5. If this value is filled with 0xff. It means use the default value 5.\n 
                                                                 0: new sample has 100% weighting and old values has 0% weighting.\n 
                                                                 1: new sample has 50% weighting and old values has 50% weighting.\n
                                                                 2: new sample has 25% weighting and old values has 75% weighting.\n
                                                                 3: new sample has 12.5% weighting and old values has 87.5% weighting.\n
                                                                 4: new sample has 6.25% weighting and old values has 93/75% weighting.\n
                                                                 5: new sample has 3.125% weighting and old values has 96.875% weighting.*/
    int32_t  high_thr[8];                                   /**< The channel high threshold. */
    int32_t  low_thr[8];                                    /**< The channel low threshold. */
    uint32_t coarse_cap[8];                                 /**< The channel coarse cap value. */
} hal_captouch_config_t;


/** @brief This structure defines tune type */
typedef enum {
    HAL_CAPTOUCH_TUNE_MAN  = 0,                             /**< Specify the manual tune mode. */
    HAL_CAPTOUCH_TUNE_HW_AUTO = 1,                          /**< Specify the hardware automatic tune mode. */
    HAL_CAPTOUCH_TUNE_SW_AUTO = 2                           /**< Specify the software automatic tune mode. */
} hal_captouch_tune_type_t;


/** @brief This structure defines power mode type */
typedef enum {
    HAL_CAPTOUCH_MODE_NORMAL  = 0,                          /**< Specify the normal mode */
    HAL_CAPTOUCH_MODE_LOWPOWER = 1                          /**< Specify the low-power mode */
} hal_captouch_lowpower_type_t;

/** @brief This structure defines tune data */
typedef struct {
    uint8_t  coarse_cap;                                    /**< Specify the coarese cap value. */
    int16_t  fine_cap;                                      /**< Specify the current fine cap value. */
    int16_t  avg_adc;                                       /**< Specify the current average adc value. */
    int16_t  vadc;                                          /**< Specify the current adc value. */
    int16_t  man;                                           /**< Specify the current fine cap vavlue of the manual mode. */
} hal_captouch_tune_data_t;

/** @brief This structure defines key event data */
typedef struct {
    hal_captouch_key_state_t state;                         /**< Specifies the key status, such as released or pressed etc. */
    uint32_t   key_data;                                    /**< Specifies the key position number on the channel.*/
    uint32_t   time_stamp;                                  /**< Specifies the time stamp of the event on the channel.*/
} hal_captouch_event_t;


/** @brief This structure defines channel */
typedef enum {
    HAL_CAPTOUCH_CHANNEL_0 = 0,                             /**< Specifies the channel0. */
    HAL_CAPTOUCH_CHANNEL_1 = 1,                             /**< Specifies the channel1. */
    HAL_CAPTOUCH_CHANNEL_2 = 2,                             /**< Specifies the channel2. */
    HAL_CAPTOUCH_CHANNEL_3 = 3,                             /**< Specifies the channel3. */
    HAL_CAPTOUCH_CHANNEL_4 = 4,                             /**< Specifies the channel4. */
    HAL_CAPTOUCH_CHANNEL_5 = 5,                             /**< Specifies the channel5. */
    HAL_CAPTOUCH_CHANNEL_6 = 6,                             /**< Specifies the channel6. */
    HAL_CAPTOUCH_CHANNEL_7 = 7,                             /**< Specifies the channel7. */
    HAL_CAPTOUCH_CHANNEL_MAX = 8                            /**< Specifies the maximum channel number. */
} hal_captouch_channel_t;

/**
  * @}
  */

/**
 * @brief     This function initializes the captouch module.
 *            Call this function if the captouch is required.
 * @param[in] config is the pointer to configuration. For more details, please refer to #hal_captouch_config_t.
 * @return    #HAL_CAPTOUCH_STATUS_OK, if operation is successful.\n
 * @sa        #hal_captouch_deinit()
 */
hal_captouch_status_t hal_captouch_init(hal_captouch_config_t *config);

/**
 * @brief     This function deinitializes the captouch module.
 *            Call this function if the captouch is no longer in use.
 * @return    #HAL_CAPTOUCH_STATUS_OK, if operation is successful.\n
 * @sa        #hal_captouch_init()
 */
hal_captouch_status_t hal_captouch_deinit(void);

/**
 * @brief     This function enables the channel. It will enable the sensing of the channel interrupt, and the channel wakeup interrupt.
 *            This API and the #hal_captouch_channel_disable() are codependent. It must be called after #hal_captouch_init() if necessary.
 * @param[in] channel is the channel number. For more details, please refer to #hal_captouch_channel_t.
 * @return    #HAL_CAPTOUCH_STATUS_OK, if the operation is successful.\n
 * @sa        #hal_captouch_channel_disable()
 */
hal_captouch_status_t hal_captouch_channel_enable(hal_captouch_channel_t channel);


/**
 * @brief     This function disables the channel. It will disable the sensing, the channel interrupt, and the channel wakeup interrupt.
 *            This API and the #hal_captouch_channel_enable() are the independ, it must be called after #hal_captouch_init() if necessary.
 * @param[in] channel is the channel number. For more details, please refer to #hal_captouch_channel_t.
 * @return    #HAL_CAPTOUCH_STATUS_OK, if the operation is successful.\n
 * @sa        #hal_captouch_channel_enable()
 */
hal_captouch_status_t hal_captouch_channel_disable(hal_captouch_channel_t channel);

/**
 * @brief     This function gets the key event data.
 *            This key data is the channel position number. You can call #hal_captouch_translate_channel_to_symbol() to translate the key symbol name for application.
 * @param[out] event is the pointer to key event data. For more details, please refer to #hal_captouch_event_t.
 * @return    #HAL_CAPTOUCH_STATUS_OK, if the operation is successful.\n
 * @sa        #hal_captouch_translate_channel_to_symbol()
 */
hal_captouch_status_t hal_captouch_get_event(hal_captouch_event_t *event);

/**
 * @brief     This function translates the key data position value to key symbol name.
 *            The key symbol name is defined by the EPT tool. It can be use for the application to recognize the meaning of the key.
 * @param[in] channel is the channel number. For more details, please refer to #hal_captouch_channel_t.
 * @param[out] symbol is the pointer to the symbol name data. please refer to the file in hal_keypad_table.h.
 * @return    #HAL_CAPTOUCH_STATUS_OK, if the operation is successful.\n
 * @sa        #hal_captouch_get_event()
 */
hal_captouch_status_t hal_captouch_translate_channel_to_symbol(hal_captouch_channel_t channel, uint8_t *symbol);


/**
 * @brief     This function controls when the captouch goes into normal or low-power mode. It can be used after #hal_captouch_init().
 * @param[in] lowpower_type is the type value. For more details, please refer to #hal_captouch_lowpower_type_t.
 * @return    #HAL_CAPTOUCH_STATUS_OK, if the operation is successful.\n
 */
hal_captouch_status_t hal_captouch_lowpower_control(hal_captouch_lowpower_type_t lowpower_type);

/**
 * @brief     This function changes the high threshold and the low threshold by channel.
 * @param[in] channel is the channel number. For more details, please refer to #hal_captouch_channel_t.
 * @param[in] high_thr is the high threshold value. When the average adc value is greater than this value, it will trigger the press event.
 *            The range of the value should be -256<=high_thr<=255.
 * @param[in] low_thr is the low threshold value. When the average adc value is less than this value, it will trigger the release event.
 *            The range of the value should be -256<=high_thr<=255.
 * @return    #HAL_CAPTOUCH_STATUS_OK, if the operation is successful.\n
 */
hal_captouch_status_t hal_captouch_set_threshold(hal_captouch_channel_t channel,int32_t high_thr, int32_t low_thr);

/**
 * @brief     This function changes the fine cap value. This API can be used only in manual mode.
 * @param[in] channel is the channel number. For more details, please refer to #hal_captouch_channel_t.
 * @param[in] fine is the fine cap value. If in the same coarse cap value, the fine cap value is bigger, the average and the vadc will be smaller.
 *            The range of the value should be -64<=fine<=63.
 * @return    #HAL_CAPTOUCH_STATUS_OK, if the operation is successful.\n
 */
hal_captouch_status_t hal_captouch_set_fine_cap(hal_captouch_channel_t channel,int32_t fine);

/**
 * @brief     This function change the coarese cap value.
 * @param[in] channel is the channel number. For more details, please refer to #hal_captouch_channel_t.
 * @param[in] coarse is the corase cap value. If in the same fine cap value, the coarse cap value is bigger, the average and the vadc will be smaller.
 *            The range of the value should be 0<=coarse<=7.
 * @return    #HAL_CAPTOUCH_STATUS_OK, if the operation is successful.\n
 */
hal_captouch_status_t hal_captouch_set_coarse_cap(hal_captouch_channel_t channel, uint32_t coarse);

/**
 * @brief     This function tunes the calibration value by channel.
 * @param[in] channel is the channel number. For more details, please refer to #hal_captouch_channel_t.
 * @param[in] tune_type is the tune type mode. For more details, please refer to #hal_captouch_tune_type_t.
 *            when use the #HAL_CAPTOUCH_TUNE_MAN, please make sure that you have called the #hal_captouch_set_fine_cap() to set fine cap.
 * @param[out] data is the pointer tune result.
 * @return    #HAL_CAPTOUCH_STATUS_OK, if the operation is successful.\n
 */
hal_captouch_status_t hal_captouch_tune_control(hal_captouch_channel_t channel,hal_captouch_tune_type_t tune_type, hal_captouch_tune_data_t* data);







#ifdef __cplusplus
}
#endif
/**
* @}
* @}
*/

#endif /* HAL_CAPTOUCH_MODULE_ENABLED */
#endif /* __HAL_CAPTOUCH_H__ */


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
#ifndef __CCCI_H__
#define __CCCI_H__
#include "hal_ccni.h"
#include "ccci_configure.h"

#include "hal_core_status.h"

/**
 * @addtogroup kernel_service
 * @{
 * @addtogroup CCCI
 * @{
 * This section describes the programming interface of the CCCI driver.
 *
 * @section Ccci_Terms_Chapter Terms and acronyms
 *
 * The following provides descriptions of the terms commonly used in the CCCI driver and how to use its various functions.
 *
 * |Terms                   |Details                                                                 |
 * |------------------------------|------------------------------------------------------------------------|
 * |\b CCCI                       | Cross cores communication interface.|
 * |\b CCNI                       | Cross Core notification interface.  |
 * @section Ccci_Features_Chapter Supported features
 * - \b Cross-Cores \b Communication.\n
 *  CCCI is CCNI upper layer. CCNI is based on HW of IRQGEN to implement a simpleness communication mechanism.
 *   One CCNI event is based on one HW channel, the details information can reference the HAL module of CCNI.
 *   CCCI is based on one CCNI event to implement a multi events communication mechanism. Use CCCI can transmit one 32bit event and one 32bit data.
 * - \b Static \b Register.\n
 *  Like CCNI, because CCCI is a communication mechanism cross multi cores, multi core should do some pre-define for the event. 
 *    So CCCI should need static register. \n
 *    For example, Core_A need send message to Core_B, user should static define event type on both Core_A and Core_B,
 *    and static define the event handle callback function on Core_B. The content which need static defined should on the project file of ccci_configure.h.\n
 *    Please reference the sample of dsp\project\ab1558_evk\templates\dsp0_freertos_create_thread\inc\ccci_configure.h and dsp\project\ab1558_evk\templates\dsp1_create_message_queue\inc\ccci_configure.h .
 * - \b Notice: The content of DSP0 and DSP1 ccci_configure.h must be in pairs.\n
 *    DSP0 project ccci_configure.h sample code as below:
 *  @code
 * //sample code:
 * //if user want to add DSP1 to DSP0 event,please add to here!!!
 * //this event is a enum of type of ccci_event_t, the event name define by user
 * //better follow the naming rule of CCCI_EVENT_DSP1_TO_DSP0_{user name}
 * //please strict follow the format, don't assignment value by user
 * //like:"CCCI_EVENT_DSP1_TO_DSP0_111 = 1," is forbidden!!!
 * #define DSP1_TO_DSP0_CCCI_EVENT_LIST    \
 *                                         CCCI_EVENT_DSP1_TO_DSP0_111,\
 *                                         CCCI_EVENT_DSP1_TO_DSP0_222,
 *
 * //if user want to add DSP0 to DSP1 event,please add to here!!!
 * //this event is a enum of type of ccci_event_t, the event name define by user
 * //better follow the naming rule of CCCI_EVENT_DSP0_TO_DSP1_{user name}
 * //please strict follow the format, don't assignment value by user
 * //like:"CCCI_EVENT_DSP0_TO_DSP1_333 = 3," is forbidden!!!
 * #define DSP0_TO_DSP1_CCCI_EVENT_LIST    CCCI_EVENT_DSP0_TO_DSP1_333,\
 *                                         CCCI_EVENT_DSP0_TO_DSP1_444,
 *
 * //if user had add event to "DSP1_TO_DSP0_CCCI_EVENT_LIST". DSP0 as receiver, must add the event callback to here!!!
 * //user must follow the format, the event sequence must be follow the event of "DSP1_TO_DSP0_CCCI_EVENT_LIST"!!!
 * #define DSP0_RECEIVE_CCCI_FROM_DSP1_USER_ARRAY  \
 *         {CCCI_EVENT_DSP1_TO_DSP0_111, ccci_dsp1_to_dsp0_111_callback},\
 *         {CCCI_EVENT_DSP1_TO_DSP0_222, ccci_dsp1_to_dsp0_222_callback},
 *  @endcode
 *    DSP1 project ccci_configure.h sample code as below:
 *  @code
 * //sample code:
 * //if user want to add DSP1 to DSP0 event,please add to here!!!
 * //this event is a enum of type of ccci_event_t, the event name define by user
 * //better follow the naming rule of CCCI_EVENT_DSP1_TO_DSP0_{user name}
 * //please strict follow the format, don't assignment value by user
 * //like:"CCCI_EVENT_DSP1_TO_DSP0_111 = 1," is forbidden!!!
 * #define DSP1_TO_DSP0_CCCI_EVENT_LIST    \
 *                                         CCCI_EVENT_DSP1_TO_DSP0_111,\
 *                                         CCCI_EVENT_DSP1_TO_DSP0_222,
 *
 * //if user want to add DSP0 to DSP1 event,please add to here!!!
 * //this event is a enum of type of ccci_event_t, the event name define by user
 * //better follow the naming rule of CCCI_EVENT_DSP0_TO_DSP1_{user name}
 * //please strict follow the format, don't assignment value by user
 * //like:"CCCI_EVENT_DSP0_TO_DSP1_333 = 3," is forbidden!!!
 * #define DSP0_TO_DSP1_CCCI_EVENT_LIST    \
 *                                         CCCI_EVENT_DSP0_TO_DSP1_333,\
 *                                         CCCI_EVENT_DSP0_TO_DSP1_444,
 *
 * //if user had add event to "DSP0_TO_DSP1_CCCI_EVENT_LIST". DSP1 as receiver, must add the event callback to here!!!
 * //user must follow the format, the event sequence must be follow the event of "DSP0_TO_DSP1_CCCI_EVENT_LIST"!!!
 * #define DSP1_RECEIVE_CCCI_FROM_DSP0_USER_ARRAY  \
 *         {CCCI_EVENT_DSP0_TO_DSP1_333, ccci_dsp0_to_dsp1_333_callback},\
 *         {CCCI_EVENT_DSP0_TO_DSP1_444, ccci_dsp0_to_dsp1_444_callback},
 *  @endcode
 * Please reference the CCCI schematic diagram as below:
 * @image html ccci_schematic_diagram.png
 *
 * @section Ccci_How_To_Use_Chapter How to use CCCI
 *  Use DSP0 send a message of CCCI_DSP0_TO_DSP1_EVENT_XXX to DSP1 as a example:
 * - \b Step 1:Add this EVENT to DSP0(sender) project file of ccci_configure.h, and add ccci/module.mk to project Makefile.
 *  @code
 *  //file:dsp\project\ab1558_evk\templates\dsp0_freertos_create_thread\inc\ccci_configure.h
 *  #define DSP0_TO_DSP1_CCCI_EVENT_LIST    CCCI_EVENT_DSP0_TO_DSP1_PIC_DEMO_LIBRARY_LOAD_DONE,\
 *                                          CCCI_EVENT_DSP0_TO_DSP1_PIC_DEMO_LIBRARY_UNLOAD_DONE,\
 *                                          CCCI_DSP0_TO_DSP1_EVENT_XXX,
 *  //file:dsp\project\ab1558_evk\templates\dsp0_freertos_create_thread\XT-XCC\Makefile
 *   ################################################################################
 *   # Include Module Configuration
 *   ################################################################################
 *   include $(ROOTDIR)/kernel/service/ccci/module.mk
 *  @endcode
 * - \b Step 2: Same with Step 1, also need add this EVENT to DSP1(receiver) project file of ccci_configure.h, and add ccci/module.mk to project Makefile.
 *  @code
 *  //file:dsp\project\ab1558_evk\templates\dsp1_create_message_queue\inc\ccci_configure.h
 *  #define DSP0_TO_DSP1_CCCI_EVENT_LIST    CCCI_EVENT_DSP0_TO_DSP1_PIC_DEMO_LIBRARY_LOAD_DONE,\
 *                                          CCCI_EVENT_DSP0_TO_DSP1_PIC_DEMO_LIBRARY_UNLOAD_DONE,\
 *                                          CCCI_DSP0_TO_DSP1_EVENT_XXX,
 *  //file:dsp\project\ab1558_evk\templates\dsp1_create_message_queue\XT-XCC\Makefile
 *   ################################################################################
 *   # Include Module Configuration
 *   ################################################################################
 *   include $(ROOTDIR)/kernel/service/ccci/module.mk
 *  @endcode
 * - \b Step 3: Add user receive callback function to DSP1(receiver) project file of ccci_configure.h
 *      Notice: User must be guarantee the event with a same sequence with the callback.
 *              Means CCCI_DSP0_TO_DSP1_EVENT_XXX in the third place of DSP0_TO_DSP1_CCCI_EVENT_LIST, so the User_callback_XXX must be in the third place of DSP1_RECEIVE_CCCI_FROM_DSP0_USER_ARRAY.
 *  @code
 *  #define DSP1_RECEIVE_CCCI_FROM_DSP0_USER_ARRAY  {\
 *           {CCCI_EVENT_DSP0_TO_DSP1_PIC_DEMO_LIBRARY_LOAD_DONE, dsp1_pisplit_lib_demo_load_done},\
 *           {CCCI_EVENT_DSP0_TO_DSP1_PIC_DEMO_LIBRARY_UNLOAD_DONE, dsp1_pisplit_lib_demo_unload_done},\
 *           {CCCI_DSP0_TO_DSP1_EVENT_XXX,User_callback_XXX}
 *         }
 *  @endcode
 * - \b Step 4: Defined user User_callback_XXX() on DSP1(receiver) side, the function type is #p_ccci_receive_callback
 *  @code
 * void User_callback_XXX(ccci_msg_t ccci_msg)
 * {
 *     uint32_t user_data1,user_data2;
 *     if(ccci_msg.event != CCCI_DSP0_TO_DSP1_EVENT_XXX)
 *     {
 *         assert(0);
 *     }
 *     user_data1 = ccci_msg.data;
 *     user_data2 = ccci_msg.data2;
 *          //Example of use CCCI to transfer buffer address and length.
 *          //user_data1 = hal_memview_infrasys_to_dsp1(ccci_msg.data);
 *          //user_data2 = ccci_msg.data2;
 *     //add your code to here!!!
 * }
 *  @endcode
 * - \b Step 5: Send the message on DSP0 side.
 *  @code
 *   #include "ccci.h"
 *   ccci_msg_t ccci_msg;
 *   ccci_msg.event = CCCI_DSP0_TO_DSP1_EVENT_XXX;
 *   ccci_msg.data = ???;
 *   ccci_msg.data2 = ???;
 *          //Example of use CCCI to transfer buffer address and length.Because of :
 *          //Multi cores have same view to see the physics(infra view) address for one device.
 *          //Multi cores have difference view to see a virtual(cacheable/non-cacheable view) address for one memory device.
 *          //So for the user code common and robustness, must do address transition when send/receive the address information.
 *          // Example code:
 *          // ccci_msg.data = hal_memview_dsp0_to_infrasys(address);
 *          // ccci_msg.data2 = len;
 *   if(CCCI_STATUS_OK != ccci_send_msg(HAL_CORE_DSP0,ccci_msg,CCCI_SEND_MSG_NO_WAIT))
 *   {
 *       assert(0);
 *   }
 *  @endcode
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DSP1_TO_DSP0_CCCI_EVENT_LIST
#define DSP1_TO_DSP0_CCCI_EVENT_LIST
#endif
#ifndef DSP0_TO_DSP1_CCCI_EVENT_LIST
#define DSP0_TO_DSP1_CCCI_EVENT_LIST
#endif

#ifndef CM4_TO_DSP0_CCCI_EVENT_LIST
#define CM4_TO_DSP0_CCCI_EVENT_LIST
#endif

#ifndef DSP0_TO_CM4_CCCI_EVENT_LIST
#define DSP0_TO_CM4_CCCI_EVENT_LIST
#endif

#ifndef CM4_TO_DSP1_CCCI_EVENT_LIST
#define CM4_TO_DSP1_CCCI_EVENT_LIST
#endif

#ifndef DSP1_TO_CM4_CCCI_EVENT_LIST
#define DSP1_TO_CM4_CCCI_EVENT_LIST
#endif



/*****************************************************************************
 * Enums
 *****************************************************************************/

/** @defgroup ccci_enum Enum
 *  @{
 */
/** @brief This enum defines the CCCI event. */
typedef enum {
    //DSP1 send to DSP0
    CCCI_EVENT_DSP1_TO_DSP0_start= CCNI_EVENT_SRC_DSP1|CCNI_EVENT_DST_DSP0,    /**< Defines DSP1 to DSP0 CCCI event start, 
    DSP1_TO_DSP0_CCCI_EVENT_LIST defined in by project file of ccci_configure.h by users. Notice: Both sender and receiver need defined all events.*/
    DSP1_TO_DSP0_CCCI_EVENT_LIST
    CCCI_EVENT_DSP1_TO_DSP0_end,           /**< Defines DSP1 to DSP0 CCCI event end */

    //DSP0 send to DSP1
    CCCI_EVENT_DSP0_TO_DSP1_start= CCNI_EVENT_SRC_DSP0|CCNI_EVENT_DST_DSP1,    /**< Defines DSP0 to DSP1 CCCI event start, 
    DSP0_TO_DSP1_CCCI_EVENT_LIST defined in by project file of ccci_configure.h by users.Notice: Both sender and receiver need defined all events.*/
    DSP0_TO_DSP1_CCCI_EVENT_LIST
    CCCI_EVENT_DSP0_TO_DSP1_end,           /**< Defines DSP0 to DSP1 CCCI event end */

    //CM4 send to DSP0
    CCCI_EVENT_CM4_TO_DSP0_start= CCNI_EVENT_SRC_CM4|CCNI_EVENT_DST_DSP0,      /**< Defines CM4 to DSP0 CCCI event start, 
    CM4_TO_DSP0_CCCI_EVENT_LIST defined in by project file of ccci_configure.h by users. Notice: Both sender and receiver need defined all events.*/
    CM4_TO_DSP0_CCCI_EVENT_LIST
    CCCI_EVENT_CM4_TO_DSP0_end,            /**< Defines CM4 to DSP0 CCCI event end */

    //DSP0 send to CM4
    CCCI_EVENT_DSP0_TO_CM4_start= CCNI_EVENT_SRC_DSP0|CCNI_EVENT_DST_CM4,      /**< Defines DSP0 to CM4 CCCI event start,
    DSP0_TO_CM4_CCCI_EVENT_LIST defined in by project file of ccci_configure.h by users. Notice: Both sender and receiver need defined all events.*/
    DSP0_TO_CM4_CCCI_EVENT_LIST
    CCCI_EVENT_DSP0_TO_CM4_end,            /**< Defines DSP0 to CM4 CCCI event end */

    //CM4 send to DSP1
    CCCI_EVENT_CM4_TO_DSP1_start= CCNI_EVENT_SRC_CM4|CCNI_EVENT_DST_DSP1,      /**< Defines CM4 to DSP1 CCCI event start,
    CM4_TO_DSP1_CCCI_EVENT_LIST defined in by project file of ccci_configure.h by users. Notice: Both sender and receiver need defined all events.*/
    CM4_TO_DSP1_CCCI_EVENT_LIST
    CCCI_EVENT_CM4_TO_DSP1_end,            /**< Defines CM4 to DSP1 CCCI event end */

    //DSP1 send to CM4
    CCCI_EVENT_DSP1_TO_CM4_start= CCNI_EVENT_SRC_DSP1|CCNI_EVENT_DST_CM4,  /**< Defines DSP1 to CM4 CCCI event start,
    DSP1_TO_CM4_CCCI_EVENT_LIST defined in by project file of ccci_configure.h by users. Notice: Both sender and receiver need defined all events.*/
    DSP1_TO_CM4_CCCI_EVENT_LIST
    CCCI_EVENT_DSP1_TO_CM4_end,            /**< Defines DSP1 to CM4 CCCI event end */

    CCCI_EVENT_DSP0_TO_N9_start = 0xffffffff,   /**< N9 not support CCCI */
    CCCI_EVENT_DSP0_TO_N9_end =   0xffffffff,   /**< N9 not support CCCI */
    CCCI_EVENT_DSP1_TO_N9_start = 0xffffffff,   /**< N9 not support CCCI */
    CCCI_EVENT_DSP1_TO_N9_end =   0xffffffff,   /**< N9 not support CCCI */
    CCCI_EVENT_CM4_TO_N9_start =  0xffffffff,   /**< N9 not support CCCI */
    CCCI_EVENT_CM4_TO_N9_end =    0xffffffff,   /**< N9 not support CCCI */
    CCCI_EVENT_N9_TO_CM4_start =  0xffffffff,   /**< N9 not support CCCI */
    CCCI_EVENT_N9_TO_CM4_end =    0xffffffff,   /**< N9 not support CCCI */
    CCCI_EVENT_N9_TO_DSP0_start = 0xffffffff,   /**< N9 not support CCCI */
    CCCI_EVENT_N9_TO_DSP0_end =   0xffffffff,   /**< N9 not support CCCI */
    CCCI_EVENT_N9_TO_DSP1_start = 0xffffffff,   /**< N9 not support CCCI */
    CCCI_EVENT_N9_TO_DSP1_end =   0xffffffff,   /**< N9 not support CCCI */

    CCCI_EVENT_max= 0xffffffff,
} ccci_event_t;

/** @brief This enum defines the CCCI status. */
typedef enum {
    CCCI_STATUS_PARAMETER_ERROR = -3,   /**< Defines status of parameter error */
    CCCI_STATUS_SW_FIFO_FULL = -2,      /**< Defines status of SW FIFO full */
    CCCI_STATUS_FAIL = -1,              /**< Defines status of CCCI fail */
    CCCI_STATUS_OK   = 0,               /**< Defines status of CCCI ok */
    CCCI_STATUS_BUSY = 1,               /**< Defines status of CCCI busy, user should try again */
} ccci_status_t;

/** @brief This enum defines the CCCI send wait type. */
typedef enum {
    CCCI_SEND_MSG_NO_WAIT = 0,                      /**< Defines the wait type, means non-blocking to send. The event will push to SW FIFO firstly, then automatic send the event when CCCI is idle */
    CCCI_SEND_MSG_WAIT_FOR_SEND_RECEIVE_DONE = 1,   /**< Defines the wait type, means will wait until other Core receive done and execute user callback done */
} ccci_send_wait_type_t;


/**
 * @}
 */

/** @defgroup ccci_struct Struct
 *  @{
 */

/** @brief ccni msg structure */
typedef struct {
    ccci_event_t event;  /**< Defines ccci event */
    uint32_t data;       /**< Defines ccci user data1 */
    uint32_t data2;      /**< Defines ccci user data2 */
} ccci_msg_t;
/**
 * @}
 */

/** @defgroup preloader_typedef Typedef
 *  @{
 */
/** @brief This typedef defines user's CCCI reveive callback function prototype.
 */
typedef void (*p_ccci_receive_callback)(ccci_msg_t ccci_msg);
/**
 * @}
 */


/**
 * @brief     This function for ccci send message by sender.
 * @return
 *                #CCCI_STATUS_OK if OK. \n
 *                #CCCI_STATUS_FAIL if fail. \n
 * @note
 *  @code
 *   #include "ccci.h"
 *   ccci_msg_t ccci_msg;
 *   ccci_msg.event = CCCI_DSP0_TO_DSP1_EVENT_XXX;
 *   ccci_msg.data = ???;
 *   ccci_msg.data2 = ???;
 *   if(CCCI_STATUS_OK != ccci_send_msg(HAL_CORE_DSP0,&ccci_msg,CCCI_SEND_MSG_NO_WAIT))
 *   {
 *       assert(0);
 *   }
 *  @endcode
 */
ccci_status_t ccci_send_msg(hal_core_id_t dst_core_id,ccci_msg_t ccci_msg,ccci_send_wait_type_t wait_type);

#ifdef __cplusplus
}
#endif
/**
 * @}
 * @}
*/


#endif /* __CCCI_H__ */


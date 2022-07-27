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

#ifndef __SYSLOG_H__
#define __SYSLOG_H__

#include <stdarg.h>
#include <stdint.h>
#include "hal_uart.h"
#include "memory_attribute.h"

/**
 * @addtogroup kernel_service
 * @{
 * @addtogroup SYSLOG
 * @{
 *
 * @brief This section introduces the SYSLOG APIs including terms and acronyms, supported features, details on how to use the system log, function groups, enums, structures, and functions.
 *
 * SYSLOG facilitates debugging during software development and provides a convenient logging system that supports log filtering and multitasking.
 * Log filtering allows the developers to focus on specific parts of the logs.
 * Multitasking support makes sure that the logging is properly managed when more than one task calls the log API at the same time.
 *
 * @section SYSLOG_Terms_Chapter Terms and acronyms
 *
 * |        Terms         |           Details                              |
 * |----------------------|------------------------------------------------|
 * |\b      NVDM          |        Non-volatile Data Management            |
 * |\b      IRQ           |            Interrupt Request                   |
 * |\b      UART          |  Universal Asynchronous Receiver/Transmitter   |
 * |\b      USB           |           Universal Serial Bus                 |
 * |\b      GPT           |           General Purpose Timer                |
 * |\b      OS            |             Operation System                   |
 *
 * @}
 * @}
 */

#ifdef MTK_SYSLOG_VERSION_2
/**
 * @addtogroup kernel_service
 * @{
 * @addtogroup SYSLOG
 * @{
 * @section SYSLOG_Features_Chapter Supported features
 * - \b Unified \b format \b log \b output
 *   - The SYSLOG module manages every user's log and shows it in the same format.
 *
 * - \b Support \b log \b over \b different \b type \b of \b log \b port
 *   - The SYSLOG module can support logging through a UART or USB port.
 *
 * - \b Multi-Level \b debug \b level \b for \b log \b filter
 *   - The SYSLOG module provides support for logging at more than one level so that the user can control which log is used at buildtime and runtime.
 *
 * - \b Support \b multitasking \b and \b IRQ \b log \b out
 *   - The SYSLOG module enables the user to print the log in task context and IRQ context.
 *
 * - \b Compatible \b with \b log \b over \b printf()
 *   - The SYSLOG module redirects printf() and re-implements this function so that the log format is compatible with a standard log.
 *
 * - \b Support \b high \b efficiency \b log \b out \b with \b MSGID \b enable
 *   - The SYSLOG module provides several different APIs to allow the user to send only its log position to the PC side for display.
 *
 * - \b Support \b integrate \b with \b customized \b log \b data
 *   - The user can use the SYSLOG module to output customized logging data, such as BT or audio data.
 *
 * @section SYSLOG_Architecture_Chapter System Log architecture
 *   SYSLOG is dependent on the USB/UART driver, GPT, NVDM, and HW semaphore services.
 *     @image html syslog_architecture.png
 *
 * @section SYSLOG_HOWTO How to use SYSLOG Module
 *
 * - \b How \b to \b initialize \b SYSLOG \b module
 *  - The user must call the following APIs to initialize the SYSLOG module in sequence.
 *    Note that it is only necessary for the CM4 side to initialize the SYSLOG module.
 *   - Step 1: Call #log_uart_init() to do pre-initialization for the SYSLOG module during the early initialization phase.
 *             This API causes the logs to be sent with the polling mode according to the UART port.
 *             The user must set the port number and the baudrate to this API to control the UART port number and speed for early logging.
 *
 *   - Step 2: Call #log_init() to do a complete initialization for the SYSLOG module.
 *             This API causes the logs to be sent with the DMA mode according to the UART or USB port.
 *             The SYSLOG module tries to read the port number, the port baudrate, and the port type from NVDM.
 *             If the setting is found, the SYSLOG module uses the setting from NVDM.
 *             If not, the SYSLOG uses the setting passed on by the user and writes the setting to NVDM.
 *             Note that only the UART port must be initialized with the baudrate parameter.
 *
 *  - The user must complete the following procedure to configure the SYSLOG over the USB port.
 *   - Set MTK_USB_DEMO_ENABLED = y in feature.mk of the project to enable the USB middleware module.
 *   - Define HAL_USB_MODULE_ENABLED in hal_feature_config.h of the project to enable the USB driver module,
 *     and make sure that this applies to both CM4 side and other CPU sides.
 *   - Modify the port type and port number when call #log_init() in main.c
 *    - log_init(SYSLOG_PORT_TYPE_USB, 0)  USB modem port for syslog
 *    - log_init(SYSLOG_PORT_TYPE_USB, 1)  USB debug port for syslog
 *   - Make sure to select the USB modem port or the USB debugging port when opening the PC logging tool.
 *   - Make sure to perform a complete format when you change the USB port for the syslog.
 *
 * - \b How \b to \b print \b log
 *  - The SYSLOG module makes the log APIs with a group so that it supports different types of logs.
 *   - #LOG_D() / #LOG_I() / #LOG_W() / #LOG_E()
 *     - These APIs allow the user to print the log to UART/USB in the usual method.
 *   - #LOG_MSGID_D() / #LOG_MSGID_I() / #LOG_MSGID_W() / #LOG_MSGID_E()
 *     - These APIs allow the user to print the log to UART/USB using an efficient method.
 *       The build system collects the user's log strings in a different log file
 *       so the SYSLOG module only needs to transmit the offset information to the PC side.
 *       The PC logging tool finds and parses the specific string from this separate log file
 *       based on the offset information.
 *     - You must make sure that the user passes the number of parameters to these APIs.
 *       There is only support for \%d, \%i, \%u, \%x, \%X, and \%c.
 *   - #LOG_TLVDUMP_D() / #LOG_TLVDUMP_I() / #LOG_TLVDUMP_W() / #LOG_TLVDUMP_E()
 *     - These APIs allow the user to send the customized log with a specific format
 *       so the PC logging tool can filter those logs and do specific operations on them.
 *       The user must add a log type in #log_type_t to create a new type of log,
 *       and add the post process to the PC logging tool.
 *   - printf()
 *     - This API is the function of C library. The user must not call this API to print the log.
 *       This API is only supported because some third-party software uses printf() to print the log.
 *
 * - \b How \b to \b control \b filter \b of \b log
 *  - log level
 *   - The SYSLOG defines five types of log level, as shown below:
 *    |   log level  |        LOG_D      |   LOG_I           |   LOG_W           |   LOG_E           |
 *    |--------------|-------------------|-------------------|-------------------|-------------------|
 *    |    debug     |        Y          |    Y              |    Y              |    Y              |
 *    |    info      |        N          |    Y              |    Y              |    Y              |
 *    |   warning    |        N          |    N              |    Y              |    Y              |
 *    |    error     |        N          |    N              |    N              |    Y              |
 *    |    none      |        N          |    N              |    N              |    N              |
 *
 *    |  log level   |  LOG_MSGID_D      |  LOG_MSGID_I      |  LOG_MSGID_W      |  LOG_MSGID_E      |
 *    |--------------|-------------------|-------------------|-------------------|-------------------|
 *    |    debug     |        Y          |    Y              |    Y              |    Y              |
 *    |    info      |        N          |    Y              |    Y              |    Y              |
 *    |   warning    |        N          |    N              |    Y              |    Y              |
 *    |    error     |        N          |    N              |    N              |    Y              |
 *    |    none      |        N          |    N              |    N              |    N              |
 *
 *    |  log level   |  LOG_TLVDUMP_D    |  LOG_TLVDUMP_I    |  LOG_TLVDUMP_W    |  LOG_TLVDUMP_E    |
 *    |--------------|-------------------|-------------------|-------------------|-------------------|
 *    |    debug     |        Y          |    Y              |    Y              |    Y              |
 *    |    info      |        N          |    Y              |    Y              |    Y              |
 *    |   warning    |        N          |    N              |    Y              |    Y              |
 *    |    error     |        N          |    N              |    N              |    Y              |
 *    |    none      |        N          |    N              |    N              |    N              |
 *
 *    |  log level   |  LOG_HEXDUMP_D    |  LOG_HEXDUMP_I    |  LOG_HEXDUMP_W    |  LOG_HEXDUMP_E    |
 *    |--------------|-------------------|-------------------|-------------------|-------------------|
 *    |    debug     |        Y          |    Y              |    Y              |    Y              |
 *    |    info      |        N          |    Y              |    Y              |    Y              |
 *    |   warning    |        N          |    N              |    Y              |    Y              |
 *    |    error     |        N          |    N              |    N              |    Y              |
 *    |    none      |        N          |    N              |    N              |    N              |
 *
 *     The level of urgency increases from the debug log level to the error log level.
 *     If we define MTK_DEBUG_LEVEL as info, the debugging log cannot be output.
 *     If we define MTK_DEBUG_LEVEL as error, only the error log can be output.
 *     The log level control can have an effect during both the buildtime and runtime.
 *     MTK_DEBUG_LEVEL provides the option to decide on the log level during the buildtime.
 *     Debugging logs with a log level less than MTK_DEBUG_LEVEL are bypassed and are not built in the final binary,
 *     so they cannot output any log.
 *     Debugging logs with a log level less than the log level setting of the current CPU or current module are not output during runtime.
 *  - Log Module
 *   - The SYSLOG module supports a log filter with different groups which we call a "log module".
 *     The log modules can be filtered separately.
 *     The user must complete the following procedure to create a customized log module:
 *    - Step 1: Call #log_create_module() to create the log module.
 *    - Step 2: Call #LOG_CONTROL_BLOCK_DECLARE() to declare the log module when user reference log module in different file.
 *    - Step 3: Call #log_set_filter() to make the filters available to the system log module.
 *      @code
 *
 *      #include "syslog.h"
 *
 *      log_create_module(main, PRINT_LEVEL_INFO);
 *
 *      void system_init(void)
 *      {
 *          log_set_filter();
 *      }
 *
 *      @endcode
 *
 *  - How to turn on/off the log
 *   - The user can turn the log on or off during the buildtime and runtime.
 *    - For buildtime:
 *     - a. Each CPU can separately turn off all logs by defining MTK_DEBUG_LEVEL as none.
 *     - b. Each CPU can also turn some levels of log on or off by defining MTK_DEBUG_LEVEL as info/warning/error.
 *     - c. Each CPU can also make all logs available by defining MTK_DEBUG_LEVEL as debug.
 *    - For runtime:
 *     - a. Each CPU can separately turn on/off all logs by sending a request according to the PC logging tool.
 *     - b. Each module's log can be turned on or off separately by sending a request according to the PC logging tool.
 *
 * - \b Watch \b the \b log \b content \b on \b PC \b side
 *  - There is a special PC logging tool for showing the log.
 *    The user must first open the PC logging tool. They can then properly view the log from the target.
 *    Please refer to the related document for more information about using the logging tool.
 *
 * - \b How \b to \b Port \b SYSLOG \b in \b new \b project
 *  - The user must complete the following procedure to use the SYSLOG service:
 *   - Step 1: Define the MTK_DEBUG_LEVEL in feature.mk
 *   - Step 2: Make any necessary changes to the project's makefile.
 *    - a. Include the syslog/exception's module.mk
 *      @code
 *         include $(ROOTDIR)/kernel/service/exception_handler/module.mk
 *         include $(ROOTDIR)/kernel/service/syslog/module.mk
 *      @endcode
 *    - b. Add a rule for generating the MSG ID bin.
 *      @code
 *         @$(OBJCOPY) -O binary --remove-section=.log_str $@ $(BINARY)
 *         @$(OBJCOPY) -O binary --only-section=.log_str $@ $(OUTDIR)/dsp0_log_str.bin
 *      @endcode
 *    - c. The MSG ID bin is currently named cpuname_log_str.bin, the same as CM4 is named cm4_log_str.bin.
 *    - d. The MSG ID bin should be generated under the project's out folder,
 *        such as mcu\out\ab155x_evk\freertos_create_thread\cm4_log_str.bin for freertos_create_thread project.
 *   - Step 3: Add a log region to the project's Linker file.
 *    - a. Linker's MEMORY
 *      @code
 *         ROM_LOG_STRING(rx)  : ORIGIN = 0x06000000, LENGTH = 32M
 *      @endcode
 *    - b. Linker's SECTIONS
 *      @code
 *         .log_str :
 *         {
 *             *(.log_string)
 *         } > ROM_LOG_STRING
 *      @endcode
 * @}
 * @}
 */

#else

/**
 * @addtogroup kernel_service
 * @{
 * @addtogroup SYSLOG
 * @{
 * @section SYSLOG_Features_Chapter Supported features
 * - \b Unified \b format \b log \b output
 *   - The SYSLOG module manages every user's log and shows it in the same format.
 *
 * - \b Support \b log \b over \b different \b types \b of \b log \b ports
 *   - The SYSLOG module supports logging over a UART or USB port.
 *
 * - \b Multi-Level \b debug \b level \b for \b log \b filter
 *   - The SYSLOG module supports a multi-level log level so that the user can control which log is output during buildtime and runtime.
 *
 * - \b Support \b multitasking \b and \b IRQ \b log \b out
 *   - The SYSLOG module allows the user to print the log in task context and IRQ context.
 *
 * - \b Compatible \b with \b log \b over \b printf()
 *   - The SYSLOG module supports the use of printf() to output the log as SYSLOG APIs.
 *
 * @section SYSLOG_Architecture_Chapter System Log architecture
 *   SYSLOG is dependent on the FreeRTOS, USB/UART driver, GPT, and NVDM services.
 *   The SYSLOG module uses the PORT SERVICE if it is enabled.
 *     @image html syslog_architecture.png
 *
 * @section SYSLOG_HOWTO How to use SYSLOG Module
 *
 * - \b How \b to \b initialize \b SYSLOG \b module
 *  - The user must call the following APIs in sequence to initialize the SYSLOG module.
 *   - Step 1: Call #log_uart_init() to do pre-initialization for the SYSLOG module during the early initialization phase.
 *             This API causes the logs to be sent with the polling mode determined by the UART port.
 *             The user must set the port number to this API to control which UART port is used for early logging.
 *
 *   - Step 2: Call #log_init() to complete the initialization for the SYSLOG module.
 *             This API causes the logs to be sent with the DMA mode determined by the UART or USB port when the OS starts running.
 *             The SYSLOG module tries to read the port number and the port type from NVDM.
 *             The SYSLOG module uses the setting from NVDM if it finds the setting.
 *             If not, the SYSLOG uses the setting passed by the user and writes the setting to NVDM.
 *
 * - \b How \b to \b print \b log
 *  - The SYSLOG module makes log APIs with a group to support different types of log.
 *   - #LOG_D() / #LOG_I() / #LOG_W() / #LOG_E()
 *     - These APIs allow the user to print the ASCII log string to UART/USB.
 *   - #LOG_HEXDUMP_D() / #LOG_HEXDUMP_I() / #LOG_HEXDUMP_W() / #LOG_HEXDUMP_E()
 *     - These APIs allow the user to send the ASCII log string with binary data.
 *       The SYSLOG module formats the binary data as an ASCII log string before sending it out.
 *   - printf()
 *     - This API is the function of C library. The user must not call this API to print the log.
 *       This API is only supported because some third-party software uses printf() to print the log.
 *
 * - \b How \b to \b control \b filter \b of \b log
 *  - Log Level
 *   - The SYSLOG defines five types of log level, as shown below:
 *    |   log level  |        LOG_D      |   LOG_I           |   LOG_W           |   LOG_E           |
 *    |--------------|-------------------|-------------------|-------------------|-------------------|
 *    |    debug     |        Y          |    Y              |    Y              |    Y              |
 *    |    info      |        N          |    Y              |    Y              |    Y              |
 *    |   warning    |        N          |    N              |    Y              |    Y              |
 *    |    error     |        N          |    N              |    N              |    Y              |
 *    |    none      |        N          |    N              |    N              |    N              |
 *
 *    |  log level   |  LOG_HEXDUMP_D    |  LOG_HEXDUMP_I    |  LOG_HEXDUMP_W    |  LOG_HEXDUMP_E    |
 *    |--------------|-------------------|-------------------|-------------------|-------------------|
 *    |    debug     |        Y          |    Y              |    Y              |    Y              |
 *    |    info      |        N          |    Y              |    Y              |    Y              |
 *    |   warning    |        N          |    N              |    Y              |    Y              |
 *    |    error     |        N          |    N              |    N              |    Y              |
 *    |    none      |        N          |    N              |    N              |    N              |
 *
 *     The level of urgency increases from debug to error. The debugging log cannot be output if MTK_DEBUG_LEVEL is defined as information.
 *     Only the error log is output if MTK_DEBUG_LEVEL is defined as error.
 *     The log level control can affect the buildtime and runtime.
 *     There is a feature option with MTK_DEBUG_LEVEL that allows you to determine the log level at buildtime.
 *     Debugging logs with a log level less than MTK_DEBUG_LEVEL are bypassed and are not built in the final binary,
 *     so there is no opportunity to output a log.
 *     Debugging logs with a log level that is less than the log level setting of the current CPU or current module are not output during runtime.
 *  - Log Module
 *   - The SYSLOG module supports log filters with different groups which we call log modules.
 *     These log modules can be filtered separately.
 *     The user must complete the following procedure to create a customized log module:
 *    - Step 1: Call #log_create_module() to create the log module.
 *    - Step 2: Call #LOG_CONTROL_BLOCK_DECLARE() to declare the log module in main.c file of the project.
 *    - Step 3: Define an array in main.c file of the project to collect all log modules and initialize the array with #LOG_CONTROL_BLOCK_SYMBOL().
 *    - Step 4: Call #log_init() and pass in this array to make the filters available to the SYSLOG module.
 *      @code
 *
 *      #include "syslog.h"
 *
 *      log_create_module(main, PRINT_LEVEL_INFO);
 *
 *      LOG_CONTROL_BLOCK_DECLARE(common);
 *      LOG_CONTROL_BLOCK_DECLARE(hal);
 *
 *      log_control_block_t *syslog_control_blocks[] = {
 *          &LOG_CONTROL_BLOCK_SYMBOL(common),
 *          &LOG_CONTROL_BLOCK_SYMBOL(hal),
 *          &LOG_CONTROL_BLOCK_SYMBOL(main),
 *          NULL
 *      };
 *
 *      void system_init(void)
 *      {
 *          log_init(syslog_config_save, syslog_config_load, syslog_control_blocks);
 *      }
 *
 *      @endcode
 *
 *  - How to turn on/off the module's log
 *   - The user can turn the module's log on or off during buildtime and runtime.
 *    - For buildtime:
 *     - a. Define MTK_DEBUG_LEVEL as none to turn off all logging by the module.
 *     - b. Define MTK_DEBUG_LEVEL as info/warning/error to turn on/off separate levels of the module's log.
 *     - c. Define MTK_DEBUG_LEVEL as debug to make all of the module's logs available.
 *    - For runtime:
 *     - If the ATCI module is enabled, AT+SYSLOG can be used to separately turn on/off the module's log.
 *      - AT+SYSLOG=?
 *       - Query for the usage.
 *      - AT+SYSLOG?
 *       - Query all available SYSLOG settings.
 *      - AT+SYSLOG=<module>,<log_switch>,<print_level>
 *       - <module>: "module name"
 *       - <log_swith>: 0, 1 (0 means on, 1 means off)
 *       - <print_level>: 0, 1, 2 (0 means info, 1 means warning, 2 means error)
 *       - For example, "AT+SYSLOG="hal",0,0" means turn on the log module with name of "hal" and set the log level to info.
 *
 *     - If the CLI module is enabled, "log set" can be used to separately turn on/off the module's log.
 *      - log set
 *       - Query for the usage
 *      - log
 *       - Query for the current log level setting
 *      - log set <module> <log_switch> <print_level>
 *       - <module>: module name
 *       - <log_swith>: on, off
 *       - <print_level>: info, warning, error
 *       - For example, "log set hal on info" means turn on the log module with name of "hal" and set the log level to info.
 *
 * - \b Watch \b the \b log \b content \b on \b PC \b side
 *  - The user must use the serial tool to watch the log on the PC side.
 *   - The serial tool can be PuTTY, secrutCRT, eagleCom and so on.
 *  - The SYSLOG module adds a log header before the log body before it sends the log.
 *   - The log header contains a timestamp, module name, level, function, and line number.
 *   - For example, [T: 20761 M: hal C: INFO F: vTestTask L: 824]: CM4 Task 0 Hello World, Idx = 10
 *
 * - \b How \b to \b Port \b SYSLOG \b in \b new \b project
 *  - The user must complete the following procedure to use the SYSLOG service:
 *   - Step 1: Define MTK_DEBUG_LEVEL in feature.mk
 *   - Step 2: Make any necessary changes to the project makefile so that it includes the syslog/exception's module.mk
 *      @code
 *         include $(ROOTDIR)/kernel/service/exception_handler/module.mk
 *         include $(ROOTDIR)/kernel/service/syslog/module.mk
 *      @endcode
 *
 *   - Step 3: Porting _write() fucntion
 *      @code
 *         #if defined(MTK_PORT_SERVICE_ENABLE)
 *         extern int log_write(char *buf, int len);
 *
 *         int _write(int file, char *ptr, int len)
 *         {
 *             return log_write(ptr, len);
 *         }
 *
 *         #else
 *
 *         int _write(int file, char *ptr, int len)
 *         {
 *             int DataIdx;
 *
 *             for (DataIdx = 0; DataIdx < len; DataIdx++) {
 *                 __io_putchar(*ptr++);
 *             }
 *
 *             return len;
 *         }
 *      @endcode
 *
 *   - Step 4: Define the log setting save function and load function.
 *   - Step 5: Define the instance of the syslog_control_blocks array.
 *   - Step 6: Call #log_uart_init() and #log_init() to initialize the SYSLOG module.
 *      @code
 *         #include "syslog.h"
 *
 *         log_create_module(main, PRINT_LEVEL_INFO);
 *
 *         LOG_CONTROL_BLOCK_DECLARE(common);
 *         LOG_CONTROL_BLOCK_DECLARE(hal);
 *
 *         log_control_block_t *syslog_control_blocks[] = {
 *             &LOG_CONTROL_BLOCK_SYMBOL(common),
 *             &LOG_CONTROL_BLOCK_SYMBOL(hal),
 *             &LOG_CONTROL_BLOCK_SYMBOL(main),
 *             NULL
 *         };
 *
 *         static void syslog_config_save(const syslog_config_t *config)
 *         {
 *             nvdm_status_t status;
 *             char *syslog_filter_buf;
 *
 *             syslog_filter_buf = (char *)pvPortMalloc(SYSLOG_FILTER_LEN);
 *             configASSERT(syslog_filter_buf != NULL);
 *             syslog_convert_filter_val2str((const log_control_block_t **)config->filters, syslog_filter_buf);
 *             status = nvdm_write_data_item("common",
 *                                           "syslog_filters",
 *                                           NVDM_DATA_ITEM_TYPE_STRING,
 *                                           (const uint8_t *)syslog_filter_buf,
 *                                           strlen(syslog_filter_buf));
 *             vPortFree(syslog_filter_buf);
 *             LOG_I(common, "syslog config save, status=%d", status);
 *         }
 *
 *         static uint32_t syslog_config_load(syslog_config_t *config)
 *         {
 *             uint32_t sz = SYSLOG_FILTER_LEN;
 *             char *syslog_filter_buf;
 *
 *             syslog_filter_buf = (char *)pvPortMalloc(SYSLOG_FILTER_LEN);
 *             configASSERT(syslog_filter_buf != NULL);
 *             if (nvdm_read_data_item("common", "syslog_filters", (uint8_t *)syslog_filter_buf, &sz) == NVDM_STATUS_OK) {
 *                 syslog_convert_filter_str2val(config->filters, syslog_filter_buf);
 *             } else {
 *                     syslog_config_save(config);
 *             }
 *             vPortFree(syslog_filter_buf);
 *
 *             return 0;
 *         }
 *
 *         int main(void)
 *         {
 *             // early clock initialization
 *
 *             log_uart_init(HAL_UART_2);
 *
 *             // do infrastructure initialization
 *
 *             log_init(syslog_config_save, syslog_config_load, syslog_control_blocks);
 *
 *             // rest project initialization
 *         }
 *
 *      @endcode
 * @}
 * @}
 */


#endif

/**
* @addtogroup kernel_service
* @{
* @addtogroup SYSLOG
* @{
*/


#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup SYSLOG_enum Enum
  * @{
  */

/** @brief Print switch definition. */
typedef enum {
    DEBUG_LOG_ON,   /**<  log on */
    DEBUG_LOG_OFF   /**<  log off */
} log_switch_t;

/** @brief Print level definition. */
typedef enum {
    PRINT_LEVEL_DEBUG,   /**<  log level with debugging */
    PRINT_LEVEL_INFO,    /**<  log level with info */
    PRINT_LEVEL_WARNING, /**<  log level with a warning */
    PRINT_LEVEL_ERROR    /**<  log level with an error */
} print_level_t;

#ifdef MTK_SYSLOG_VERSION_2
/** @brief SYSLOG port type. */
typedef enum {
    SYSLOG_PORT_TYPE_UART,       /**<  using the UART port as a SYSLOG port */
    SYSLOG_PORT_TYPE_USB,        /**<  using the USB port as a SYSLOG port */
    SYSLOG_PORT_TYPE_MAX,        /**<  the maximum number of SYSLOG ports */
} syslog_port_type_t;

/**
 * @brief
 *  The PC logging tool can tell the difference between and process different types of logs
 *  by sending the users's log and attaching one of the logs from the following list.
 *  - Logs types less than #LOG_TYPE_INTERNAL_LOG_END are used for internally syncing with the PC logging tool.
 *  - Logs types less than #LOG_TYPE_COMMON_LOG_END are used internally.
 *    The user must never replace or insert a new log type between these log types.
 *   - Log types with #LOG_TYPE_TEXTURE_LOG are used for transmitting a log string using the general method.
 *   - Log types with #LOG_TYPE_MSG_ID_LOG are used for transmitting a log string using the high-efficency method.
 *     Using this method, the log can be transmited with only address information.
 *     The PC logging tool then parses the address information and restores the complete log string content.
 *     Therefore, a lot of the bandwidth can be saved for printing more log strings.
 *     We strongly suggest using this method if the user frequently sends large logs.
 *     Please refer to @ref SYSLOG_HOWTO for more information.
 *   - Log types with #LOG_TYPE_EXCEPTION_STRING_LOG and #LOG_TYPE_EXCEPTION_BINARY_LOG
 *     are used for exception dump logs.
 *
 *  - The user's log type must be defined after #LOG_TYPE_COMMON_LOG_END.
 *    The user must complete the following three steps if they want to add a new log type and make it workable.
 *   - Step 1: Define the new log type in this enum. HCI and audio now add their log type here.
 *   - Step 2: Add parse support to the PC logging tool. This usually means the additional DLL file
 *             must be created and called by the PC logging tool during runtime.
 *   - Step 3: Call #LOG_TLVDUMP_D()/#LOG_TLVDUMP_I()/#LOG_TLVDUMP_W()/#LOG_TLVDUMP_E() to
 *             send the log content with the new log type. Make sure to send the parameter with the new log type.
 */
typedef enum {
    LOG_TYPE_INTERNAL_COMMAND = 0x01,     /**<  Internal data used to communicated with the PC logging tool. */
    LOG_TYPE_INTERNAL_LOG_END = 0x0f,     /**<  The maximum number of log types used for internal usage. */

    LOG_TYPE_TEXTURE_LOG = 0x10,          /**<  Transmit log with its string content. */
    LOG_TYPE_MSG_ID_LOG = 0x11,           /**<  Transmit log with the address only. */
    LOG_TYPE_EXCEPTION_STRING_LOG = 0x12, /**<  Exception string log. */
    LOG_TYPE_EXCEPTION_BINARY_LOG = 0x13, /**<  Exception binary log. */
    LOG_TYPE_COMMON_LOG_END = 0x2f,       /**<  The maximum number of log types used for internal and common logs. */

    LOG_TYPE_HCI_DATA = 0x30,             /**<  HCI specific log data. */
    LOG_TYPE_AUDIO_DATA = 0x31,           /**<  Audio specific log data. */
    LOG_TYPE_SPECIAL_LOG_END = 0x4f,      /**<  The maximum number of log types used for customized data. */
} log_type_t;
#endif

/**
  * @}
  */

#include "syslog_internal.h"

#ifndef MTK_SYSLOG_VERSION_2

/** @defgroup SYSLOG_typedef Typedef
  * @{
  */

/**
 * @brief This typedef declares a structure prototype.
 *        See #syslog_config_s for detail information.
 */
typedef struct syslog_config_s syslog_config_t;

/**
 * @brief SYSLOG saves the config callback function prototype.
 *        Declare a function of this prototype and register it in
 *        log_init so that it can be used when saving the configuration.
 *        This decouples SYSLOG from the direct NVDM module dependency.
 */
typedef void (*syslog_save_fn)(const syslog_config_t *config);

/**
 * @brief SYSLOG loads the configuration callback function prototype.
 *        Declare a function of this prototype and register it in
 *        log_init so that it can be used when loading the configuration.
 *        This decouples SYSLOG from the direct NVDM module dependency.
 */
typedef uint32_t (*syslog_load_fn)(syslog_config_t *config);

/**
  * @}
  */

/** @defgroup SYSLOG_struct Struct
  * @{
  */

/** @brief This structure defines the SYSLOG configuration. */
struct syslog_config_s {
    syslog_save_fn          save_fn;      /**<  The function is used to save the filter setting to NVDM. */
    log_control_block_t     **filters;    /**<  Point to the log filter array that is defined by the user. */
};

/**
  * @}
  */

/** @brief This function changes the switch status and log level of the log control module during runtime.
 *         It also has the option to be saved to NVDM.
 * @param[in] module_name is the name of the log control module.
 * @param[in] log_switch_str is the string which marks the switch status. It should be one string of "on" and "off".
 * @param[in] print_level_str is the string which marks the log level. It should be one string of "debug", "info", "warning", and "error".
 * @param[in] The boolean value "save" controls whether the setting is saved to NVDM.
 * @return
 *         0, if the operation successfully completed. \n
 *         Negative numbers, if the operation failed.
 * @code
 *      ret = syslog_set_filter("common", "on", "warning", true);
 * @endcode
 * @sa #syslog_at_set_filter()
*/
int syslog_set_filter(char *module_name, char *log_switch_str, char *print_level_str, int save);

/** @brief This function changes the switch status and log level of the log control module during runtime.
 *         It also has the option to be saved to NVDM.
 * @param[in] module_name is the name of the log control module.
 * @param[in] log_switch_str is the switch status of the log control module.
 * @param[in] print_level_str is the log level of the log control module.
 * @param[in] The boolean value "save" controls whether the setting is saved to NVDM.
 * @return
 *         0, if the operation successfully completed. \n
 *         Negative numbers, if the operation failed.
 * @code
 *      ret = syslog_at_set_filter("common", DEBUG_LOG_OFF, PRINT_LEVEL_WARNING, true);
 * @endcode
 * @sa #syslog_set_filter()
*/
int syslog_at_set_filter(char *module_name, int log_switch, int print_level, int save);

/** @brief This function is used to query all SYSLOG configs during runtime.
 * @param[in] config is the pointer of the user buffer to store all SYSLOG configuration.
 */
void syslog_get_config(syslog_config_t *config);

/** @brief This function converts the filter specifications from a string to its internal notation.
 * @param[in] filters is the array of filters.
 * @param[in] buff is the filter specifications.
 * @return
 *         0, if the operation successfully completed. \n
 *         Negative numbers, if the operation failed.
 * @sa #syslog_convert_filter_val2str()
*/
int syslog_convert_filter_str2val(log_control_block_t **filters, char *buff);

/** @brief This function converts the filter specifications from internal notation to a string.
 * @param[in] filters is the array of filters.
 * @param[in] buff is the filter specifications.
 * @return
 *         End position of the formated string in the buffer.
 * @sa #syslog_convert_filter_str2val()
*/
int syslog_convert_filter_val2str(const log_control_block_t **filters, char *buff);

#endif


#ifdef MTK_SYSLOG_VERSION_2

/** @brief This function initializes the SYSLOG module to work with polling mode.
 * @param[in] port is the UART port for sending the log data out.
 * @param[in] baudrate is the baudrate of the UART port.
 * @return
 *         true, if the operation successfully completed. \n
 *         false, if the operation failed.
 * @note   It is only necessary for the CM4 side to call this API to do initialization.
 * @par Example
 * @code
 *      ret = log_uart_init(HAL_UART_0, HAL_UART_BAUDRATE_921600);
 * @endcode
 * @sa #log_init() / #log_set_filter()
*/
bool log_uart_init(hal_uart_port_t port, hal_uart_baudrate_t baudrate);

#else

/** @brief This function initializes the SYSLOG module to work with polling mode.
 *         After calling this function, the UART port is initialized with 115200bps.
 * @param[in] port is the UART port for sending out log data.
 * @return
 *         #HAL_UART_STATUS_OK, if the operation completed successfully.
 * @par Example
 * @code
 *      ret = log_uart_init(HAL_UART_0);
 * @endcode
*/
hal_uart_status_t log_uart_init(hal_uart_port_t port);

#endif

#ifdef MTK_SYSLOG_VERSION_2

/** @brief This function initializes the SYSLOG module to work with DMA mode.
 * @param[in] port_type is the type of #syslog_port_type_t.
 * @param[in] port_index is the port number of the log port.
 * @return
 *         true, if the operation completed successfully. \n
 *         false, if the operation failed.
 * @note   It is only necessary for the CM4 side to call this API to do initialization.
 * @par Example
 * @code
 *      ret = log_init(SYSLOG_PORT_TYPE_UART, 0);
 * @endcode
 * @sa #log_uart_init() / #log_set_filter()
*/
bool log_init(syslog_port_type_t port_type, uint8_t port_index);

/** @brief This function initializes the current CPU's log filter setting.
 *         Each CPU must call this API to initialize its log filter setting during the initialization phase.
 * @return
 *         true, if the operation completed successfully. \n
 *         false, if the operation failed.
 * @sa #log_uart_init() / #log_init()
*/
bool log_set_filter(void);

#else

#if defined (MTK_DEBUG_LEVEL_NONE)

#define log_init(save, load, entries)

#else

/** @brief This function initializes the SYSLOG module to work with DMA mode.
 * @param[in] save is the function pointer used to save the log filter setting to NVDM.
 * @param[in] load is the function pointer used to load the log filter setting from NVDM.
 * @param[in] entries is the pointer of the log filter array.
 * @sa #log_uart_init()
*/
void log_init(syslog_save_fn save, syslog_load_fn load, log_control_block_t  **entries);

#endif

#endif

#ifdef MTK_SYSLOG_VERSION_2

/** @brief This macro defines an instance of the log control module.
 * @param[in] module is the name of the log control module.
 * @param[in] level is the log level of the log control module.
 * @par Example
 * @code
 *      log_create_module(common, PRINT_LEVEL_INFO);
 * @endcode
*/
#define log_create_module(module, level) \
ATTR_LOG_FILTER log_control_block_t log_control_block_##module = \
{ \
    #module, \
    (DEBUG_LOG_ON), \
    (level), \
    print_module_log, \
    dump_module_buffer, \
    dump_module_tlv_buffer, \
    print_module_msgid_log \
}

/** @brief This macro defines an instance of the log control module.
 * @param[in] module is the name of the log control module.
 * @param[in] on_off is the log switch of the log control module.
 * @param[in] level is the log level of the log control module.
 * @par Example
 * @code
 *      log_create_module_variant(common, DEBUG_LOG_OFF, PRINT_LEVEL_INFO);
 * @endcode
*/
#define log_create_module_variant(module, on_off, level) \
ATTR_LOG_FILTER log_control_block_t log_control_block_##module = \
{ \
    #module, \
    (on_off), \
    (level), \
    print_module_log, \
    dump_module_buffer, \
    dump_module_tlv_buffer, \
    print_module_msgid_log \
}

#else

/** @brief This macro defines an instance of the log control module.
 * @param[in] module is the name of the log control module.
 * @param[in] level is the log level of the log control module.
 * @par Example
 * @code
 *      log_create_module(common, PRINT_LEVEL_INFO);
 * @endcode
*/
#define log_create_module(module, level) \
log_control_block_t log_control_block_##module = \
{ \
    #module, \
    (DEBUG_LOG_ON), \
    (level), \
    print_module_log, \
    dump_module_buffer \
}

/** @brief This macro defines an instance of the log control module.
 * @param[in] module is the name of the log control module.
 * @param[in] on_off is the log switch of the log control module.
 * @param[in] level is the log level of the log control module.
 * @par Example
 * @code
 *      log_create_module_variant(common, DEBUG_LOG_OFF, PRINT_LEVEL_INFO);
 * @endcode
*/
#define log_create_module_variant(module, on_off, level) \
log_control_block_t log_control_block_##module = \
{ \
    #module, \
    (on_off), \
    (level), \
    print_module_log, \
    dump_module_buffer \
}

#endif

#if defined(MTK_DEBUG_LEVEL_DEBUG)

/** @brief This macro calls the SYSLOG internal function to print the user's log string with a debugging level.
 *         Therefore, the user's log cannot be sent if the module's log level setting is greater than this level.
 * @param[in] module is the name of the log control module.
 * @param[in] message is the user's log string.
 * @param[in] ... is the parameter list corresponding to the message string.
 * @par Example
 * @code
 *      LOG_D(common, "this is test log from LOG_D() at %d", timestamp);
 * @endcode
 * @sa #LOG_I()/#LOG_W()/#LOG_E()
*/
#define LOG_D(module, message, ...) \
do { \
    extern log_control_block_t log_control_block_##module; \
    log_control_block_##module.print_handle(&log_control_block_##module, \
                                             __FUNCTION__, \
                                             __LINE__, \
                                             PRINT_LEVEL_DEBUG, \
                                             (message), \
                                             ##__VA_ARGS__); \
} while (0)

/** @brief This macro calls the SYSLOG internal function to print the user's log string and binary data with a debugging level.
 *         Therefore, the user's log cannot be sent if the module's log level setting is greater than this level.
 *         Additional binary data is formatted by the SYSLOG before it is sent.
 * @param[in] module is the name of the log control module.
 * @param[in] message is the user's log string.
 * @param[in] data is the pointer of the user's binary data content.
 * @param[in] len is the length of the user's binary data content.
 * @param[in] ... is the parameter list corresponding with the message string.
 * @par Example
 * @code
 *      LOG_HEXDUMP_D(common, "this is test log from LOG_HEXDUMP_D() at %d", binary_buffer, 1024, timestamp);
 * @endcode
 * @sa #LOG_HEXDUMP_I()/#LOG_HEXDUMP_W()/#LOG_HEXDUMP_E()
*/
#define LOG_HEXDUMP_D(module, message, data, len, ...) \
do { \
    extern log_control_block_t log_control_block_##module; \
    log_control_block_##module.dump_handle(&log_control_block_##module, \
                                            __FUNCTION__, \
                                            __LINE__, \
                                            PRINT_LEVEL_DEBUG, \
                                            (data), \
                                            (len), \
                                            (message), \
                                            ##__VA_ARGS__); \
} while (0)

#ifdef MTK_SYSLOG_VERSION_2

/** @brief This macro calls the SYSLOG internal function to print the user's log string with only address information.
 *         The parameters are sent at the same time.
 *         This macro defines the user's log with a debugging level. Therefore, the user's log cannot be sent if the module's log level is higher.
 *         The PC logging tool should search and parse the log string using the address information from the target.
 * @param[in] module is the name of the log control module.
 * @param[in] message is the user's log string.
 * @param[in] arg_cnt is the count of the user's parameters.
 * @param[in] ... is the parameter list corresponding with the message string.
 * @note   The user must specify the number of parameters when calling the API.
 * @par Example
 * @code
 *      LOG_MSGID_D(common, "this is test log from LOG_MSGID_D() at %d", 1, timestamp);
 * @endcode
 * @sa #LOG_MSGID_I()/#LOG_MSGID_W()/#LOG_MSGID_E()
*/
#ifdef MTK_SYSLOG_SUB_FEATURE_MSGID_TO_STRING_LOG_SUPPORT
#define LOG_MSGID_D(module, message, arg_cnt, ...) \
    LOG_D(module, message, ##__VA_ARGS__)
#else
#define LOG_MSGID_D(module, message, arg_cnt, ...) \
do { \
     extern log_control_block_t log_control_block_##module; \
     ATTR_LOG_STRING msg_id_string[] = "[M:" #module " C:debug F: L: ]: " message; \
     log_control_block_##module.msg_id_handle(&log_control_block_##module, \
                                            PRINT_LEVEL_DEBUG, \
                                            (msg_id_string), \
                                            (arg_cnt),  \
                                            ##__VA_ARGS__); \
} while (0)
#endif

#else

#ifndef MTK_SYSLOG_GEN_DOCUMENT
#define LOG_MSGID_D(module, message, arg_cnt, ...) \
    LOG_D(module, message, ##__VA_ARGS__)
#endif

#endif

#ifdef MTK_SYSLOG_VERSION_2

/** @brief This macro calls the SYSLOG internal function to print the user's customized data with a debugging level.
 *         Therefore, the user's data cannot be sent if the module's log level is higher.
 *         This data must be managed with a specific method on the PC side.
 *         Therefore, the type must be decided here so that the PC logging tool can split them for specific post processing.
 *         The user must check the return value to see whether all data is sent.
 *         If not, the user must try to send the remaining data by calling this API again.
 * @param[in] module is the name of the log control module.
 * @param[in] type is a uniform value of #log_type_t used to tell the difference between the user's different logs and data.
 * @param[in] p_data is the address of the data buffer array. It must be end with NULL.
 * @param[in] p_len is the array of data buffer length.
 * @param[out] ret_len is the size actually sent.
 * @note   The user must check the return value to see whether all data is sent.
 * @par Example
 * @code
 *      uint8_t *audio_buffer_array[] = {audio_buffer_1, audio_buffer_2, NULL};
 *      uint32_t audio_buffer_length_array[] = {0x10, 0x200};
 *      LOG_TLVDUMP_D(common, LOG_TYPE_HCI_DATA, audio_buffer_array, audio_buffer_length_array, ret_len);
 * @endcode
 * @sa #LOG_TLVDUMP_I()/#LOG_TLVDUMP_W()/#LOG_TLVDUMP_E()
*/
#define LOG_TLVDUMP_D(module, type, p_data, p_len, ret_len) \
do { \
    extern log_control_block_t log_control_block_##module; \
    ret_len = log_control_block_##module.tlv_dump_handle(&log_control_block_##module, \
                                            PRINT_LEVEL_DEBUG, \
                                            (type), \
                                            (const void **)(p_data), \
                                            (p_len)); \
} while (0)

#else

#ifndef MTK_SYSLOG_GEN_DOCUMENT
#define LOG_TLVDUMP_D(module, type, p_data, p_len, ret_len)
#endif

#endif

#else

#define LOG_D(module, message, ...)
#define LOG_HEXDUMP_D(module, message, data, len, ...)
#define LOG_MSGID_D(module, message, arg_cnt, ...)
#define LOG_TLVDUMP_D(module, type, p_data, p_len, ret_len)

#endif /* MTK_DEBUG_LEVEL_DEBUG */

#if defined(MTK_DEBUG_LEVEL_INFO)

/** @brief This macro calls the SYSLOG internal function to print the user's log string with an information level.
 *         Therefore, the user's log cannot be sent if the module's log level setting is higher.
 * @param[in] module is the name of the log control module.
 * @param[in] message is the user's log string.
 * @param[in] ... is the parameter list corresponding with the message string.
 * @par Example
 * @code
 *      LOG_I(common, "this is test log from LOG_I() at %d", timestamp);
 * @endcode
 * @sa #LOG_D()/#LOG_W()/#LOG_E()
*/
#define LOG_I(module, message, ...) \
do { \
    extern log_control_block_t log_control_block_##module; \
    log_control_block_##module.print_handle(&log_control_block_##module, \
                                             __FUNCTION__, \
                                             __LINE__, \
                                             PRINT_LEVEL_INFO, \
                                             (message), \
                                             ##__VA_ARGS__); \
} while (0)

/** @brief This macro calls the SYSLOG internal function to print the users's log string and binary data with an information level.
 *         Therefore, the user's log cannot be sent if the module's log level setting is higher.
 *         Additional binary data is formatted by the SYSLOG module before it is sent.
 * @param[in] module is the name of the log control module.
 * @param[in] message is the user's log string.
 * @param[in] data is the pointer of the user's binary data content.
 * @param[in] len is the length of the user's binary data content.
 * @param[in] ... is the parameter list corresponding to the message string.
 * @par Example
 * @code
 *      LOG_HEXDUMP_I(common, "this is test log from LOG_HEXDUMP_I() at %d", binary_buffer, 1024, timestamp);
 * @endcode
 * @sa #LOG_HEXDUMP_D()/#LOG_HEXDUMP_W()/#LOG_HEXDUMP_E()
*/
#define LOG_HEXDUMP_I(module, message, data, len, ...) \
do { \
    extern log_control_block_t log_control_block_##module; \
    log_control_block_##module.dump_handle(&log_control_block_##module, \
                                            __FUNCTION__, \
                                            __LINE__, \
                                            PRINT_LEVEL_INFO, \
                                            (data), \
                                            (len), \
                                            (message), \
                                            ##__VA_ARGS__); \
} while (0)

#ifdef MTK_SYSLOG_VERSION_2

/** @brief This macro calls the SYSLOG internal function to print the user's log string with only address information.
 *         The parameters are sent at the same time.
 *         This macro defines the user's log with the information level. Therefore, the user's log cannot be sent if the module's log level setting is higher.
 *         The PC logging tool must search and parse the log string using the address information from the target.
 * @param[in] module is the name of the log control module.
 * @param[in] message is the user's log string.
 * @param[in] arg_cnt is the count of the user's parameters.
 * @param[in] ... is the parameter list corresponding with the message string.
 * @note   The user must specify the number of parameters when calling the API.
 * @par Example
 * @code
 *      LOG_MSGID_I(common, "this is test log from LOG_MSGID_I() at %d", 1, timestamp);
 * @endcode
 * @sa #LOG_MSGID_D()/#LOG_MSGID_W()/#LOG_MSGID_E()
*/
#ifdef MTK_SYSLOG_SUB_FEATURE_MSGID_TO_STRING_LOG_SUPPORT
#define LOG_MSGID_I(module, message, arg_cnt, ...) \
    LOG_I(module, message, ##__VA_ARGS__)
#else
#define LOG_MSGID_I(module, message, arg_cnt, ...) \
do { \
     extern log_control_block_t log_control_block_##module; \
     ATTR_LOG_STRING msg_id_string[] = "[M:" #module " C:info F: L: ]: " message; \
     log_control_block_##module.msg_id_handle(&log_control_block_##module, \
                                            PRINT_LEVEL_INFO, \
                                            (msg_id_string), \
                                            (arg_cnt),  \
                                            ##__VA_ARGS__); \
} while (0)
#endif

#else

#ifndef MTK_SYSLOG_GEN_DOCUMENT
#define LOG_MSGID_I(module, message, arg_cnt, ...) \
    LOG_I(module, message, ##__VA_ARGS__)
#endif

#endif

#ifdef MTK_SYSLOG_VERSION_2

/** @brief This macro calls the SYSLOG internal function to print the user's customized data with an information level.
 *         Therefore, the user's data cannot be sent if the module's log level is higher.
 *         This data must be managed with a specific method on the PC side.
 *         Therefore, the type must be set here so that the PC logging tool can split it for specific post processing.
 *         The user must check the return value to see whether all data is sent.
 *         If not, the user must call this API again to send the remaining data.
 * @param[in] module is the name of the log control module.
 * @param[in] type is a uniform value of #log_type_t used to tell the difference between the user's different logs and data.
 * @param[in] p_data is the address of the data buffer array. It must be end with NULL.
 * @param[in] p_len is the array of data buffer length.
 * @param[out] ret_len is the size actually sent.
 * @note   The user must check the return value to see whether all data is sent.
 * @par Example
 * @code
 *      uint8_t *audio_buffer_array[] = {audio_buffer_1, audio_buffer_2, NULL};
 *      uint32_t audio_buffer_length_array[] = {0x10, 0x200};
 *      LOG_TLVDUMP_I(common, LOG_TYPE_HCI_DATA, audio_buffer_array, audio_buffer_length_array, ret_len);
 * @endcode
 * @sa #LOG_TLVDUMP_D()/#LOG_TLVDUMP_W()/#LOG_TLVDUMP_E()
*/
#define LOG_TLVDUMP_I(module, type, p_data, p_len, ret_len) \
do { \
    extern log_control_block_t log_control_block_##module; \
    ret_len = log_control_block_##module.tlv_dump_handle(&log_control_block_##module, \
                                            PRINT_LEVEL_INFO, \
                                            (type), \
                                            (const void **)(p_data), \
                                            (p_len)); \
} while (0)

#else

#ifndef MTK_SYSLOG_GEN_DOCUMENT
#define LOG_TLVDUMP_I(module, type, p_data, p_len, ret_len)
#endif

#endif

#else

#define LOG_I(module, message, ...)
#define LOG_HEXDUMP_I(module, message, data, len, ...)
#define LOG_MSGID_I(module, message, arg_cnt, ...)
#define LOG_TLVDUMP_I(module, type, p_data, p_len, ret_len)

#endif /* MTK_DEBUG_LEVEL_INFO */

#if defined(MTK_DEBUG_LEVEL_WARNING)
/** @brief This macro calls the SYSLOG internal function to print the user's log string with a warning level.
 *         Therefore, the user's log cannot be sent if the module's log level is higher.
 * @param[in] module is the name of the log control module.
 * @param[in] message is the user's log string.
 * @param[in] ... is the parameter list corresponding with the message string.
 * @par Example
 * @code
 *      LOG_W(common, "this is test log from LOG_W() at %d", timestamp);
 * @endcode
 * @sa #LOG_D()/#LOG_I()/#LOG_E()
*/
#define LOG_W(module, message,...) \
do { \
    extern log_control_block_t log_control_block_##module; \
    log_control_block_##module.print_handle(&log_control_block_##module, \
                                             __FUNCTION__, \
                                             __LINE__, \
                                             PRINT_LEVEL_WARNING, \
                                             (message), \
                                             ##__VA_ARGS__); \
} while (0)

/** @brief This macro calls the SYSLOG internal function to print the user's log string and binary data with a warning level.
 *         Therefore, the user's log cannot be sent if the module's log level setting is higher.
 *         Additional binary data is formatted by the SYSLOG module before it is sent.
 * @param[in] module is the name of the log control module.
 * @param[in] message is the users's log string.
 * @param[in] data is the pointer of the users's binary data content.
 * @param[in] len is length of the user's binary data content.
 * @param[in] ... is the parameter list corresponding with the message string.
 * @par Example
 * @code
 *      LOG_HEXDUMP_W(common, "this is test log from LOG_HEXDUMP_W() at %d", binary_buffer, 1024, timestamp);
 * @endcode
 * @sa #LOG_HEXDUMP_D()/#LOG_HEXDUMP_I()/#LOG_HEXDUMP_E()
*/
#define LOG_HEXDUMP_W(module, message, data, len, ...) \
do { \
    extern log_control_block_t log_control_block_##module; \
    log_control_block_##module.dump_handle(&log_control_block_##module, \
                                            __FUNCTION__, \
                                            __LINE__, \
                                            PRINT_LEVEL_WARNING, \
                                            (data), \
                                            (len), \
                                            (message), \
                                            ##__VA_ARGS__); \
} while (0)

#ifdef MTK_SYSLOG_VERSION_2

/** @brief This macro calls the SYSLOG internal function to print the user's log string with only address information.
 *         The parameters are also sent at the same time.
 *         This macro defines the user's log with a warning level. Therefore, the user's log cannot be sent if the module's log level setting is higher.
 *         The PC logging tool must search and parse the log string using the address information from the target.
 * @param[in] module is the name of the log control module.
 * @param[in] message is the user's log string.
 * @param[in] arg_cnt is the count of the user's parameters.
 * @param[in] ... is the parameter list corresponding with the message string.
 * @note   The user must specify the number of parameters when calling the API.
 * @par Example
 * @code
 *      LOG_MSGID_W(common, "this is test log from LOG_MSGID_W() at %d", 1, timestamp);
 * @endcode
 * @sa #LOG_MSGID_D()/#LOG_MSGID_I()/#LOG_MSGID_E()
*/
#ifdef MTK_SYSLOG_SUB_FEATURE_MSGID_TO_STRING_LOG_SUPPORT
#define LOG_MSGID_W(module, message, arg_cnt, ...) \
    LOG_W(module, message, ##__VA_ARGS__)
#else
#define LOG_MSGID_W(module, message, arg_cnt, ...) \
do { \
     extern log_control_block_t log_control_block_##module; \
     ATTR_LOG_STRING msg_id_string[] = "[M:" #module " C:warning F: L: ]: " message; \
     log_control_block_##module.msg_id_handle(&log_control_block_##module, \
                                            PRINT_LEVEL_WARNING, \
                                            (msg_id_string), \
                                            (arg_cnt),  \
                                            ##__VA_ARGS__); \
} while (0)
#endif

#else

#ifndef MTK_SYSLOG_GEN_DOCUMENT
#define LOG_MSGID_W(module, message, arg_cnt, ...) \
    LOG_W(module, message, ##__VA_ARGS__)
#endif

#endif

#ifdef MTK_SYSLOG_VERSION_2

/** @brief This macro calls the SYSLOG internal function to print the user's customized data with a warning level.
 *         Therefore, the user's log cannot be sent if the module's log level setting is higher.
 *         This data must be managed using a specific method on the PC side.
 *         Therefore, the type must be set here so that the PC logging tool can split it for specific post proccessing.
 *         The user must check the return value to see whether all data is sent.
 *         If not, the user must call this API again to send the remaining data.
 * @param[in] module is the name of the log control module.
 * @param[in] type is a uniform value of #log_type_t used to tell the difference between the user's different logs and the data.
 * @param[in] p_data is the address of the data buffer array. It must be end with NULL.
 * @param[in] p_len is the array of data buffer length.
 * @param[out] ret_len is the size actually be sent out.
 * @note   The user must check the return value to see whether all data is sent.
 * @return
 *         The size of data that is actually sent.
 * @par Example
 * @code
 *      uint8_t *audio_buffer_array[] = {audio_buffer_1, audio_buffer_2, NULL};
 *      uint32_t audio_buffer_length_array[] = {0x10, 0x200};
 *      LOG_TLVDUMP_W(common, LOG_TYPE_HCI_DATA, audio_buffer_array, audio_buffer_length_array, ret_len);
 * @endcode
 * @sa #LOG_TLVDUMP_D()/#LOG_TLVDUMP_I()/#LOG_TLVDUMP_E()
*/
#define LOG_TLVDUMP_W(module, type, p_data, p_len, ret_len) \
do { \
    extern log_control_block_t log_control_block_##module; \
    ret_len = log_control_block_##module.tlv_dump_handle(&log_control_block_##module, \
                                            PRINT_LEVEL_WARNING, \
                                            (type), \
                                            (const void **)(p_data), \
                                            (p_len)); \
} while (0)

#else

#ifndef MTK_SYSLOG_GEN_DOCUMENT
#define LOG_TLVDUMP_W(module, type, p_data, p_len, ret_len)
#endif

#endif

#else

#define LOG_W(module, message, ...)
#define LOG_HEXDUMP_W(module, message, data, len, ...)
#define LOG_MSGID_W(module, message, arg_cnt, ...)
#define LOG_TLVDUMP_W(module, type, p_data, p_len, ret_len)

#endif /* MTK_DEBUG_LEVEL_WARNING */

#if  defined(MTK_DEBUG_LEVEL_ERROR)
/** @brief This macro calls the SYSLOG internal function to print the user's log string with an error level.
 *         Therefore, the user's log cannot be sent if the module's log level setting is higher.
 * @param[in] module is the name of the log control module.
 * @param[in] message is the user's log string.
 * @param[in] ... is the parameter list corresponding with the message string.
 * @par Example
 * @code
 *      LOG_E(common, "this is test log from LOG_E() at %d", timestamp);
 * @endcode
 * @sa #LOG_D()/#LOG_I()/#LOG_W()
*/
#define LOG_E(module, message,...) \
do { \
    extern log_control_block_t log_control_block_##module; \
    log_control_block_##module.print_handle(&log_control_block_##module, \
                                             __FUNCTION__, \
                                             __LINE__, \
                                             PRINT_LEVEL_ERROR, \
                                             (message), \
                                             ##__VA_ARGS__); \
} while (0)

/** @brief This macro calls the SYSLOG internal function to print the user's log string and binary data with an error level.
 *         Therefore, the user's log cannot be sent if the module's log level setting is higher.
 *         Additional binary data is formatted by the SYSLOG module before it is sent.
 * @param[in] module is the name of the log control module.
 * @param[in] message is the user's log string.
 * @param[in] data is the pointer of the user's binary data content.
 * @param[in] len is the length of the user's binary data content.
 * @param[in] ... is the parameter list corresponding with the message string.
 * @par Example
 * @code
 *      LOG_HEXDUMP_E(common, "this is test log from LOG_HEXDUMP_E() at %d", binary_buffer, 1024, timestamp);
 * @endcode
 * @sa #LOG_HEXDUMP_D()/#LOG_HEXDUMP_I()/#LOG_HEXDUMP_W()
*/
#define LOG_HEXDUMP_E(module, message, data, len, ...) \
do { \
    extern log_control_block_t log_control_block_##module; \
    log_control_block_##module.dump_handle(&log_control_block_##module, \
                                            __FUNCTION__, \
                                            __LINE__, \
                                            PRINT_LEVEL_ERROR, \
                                            (data), \
                                            (len), \
                                            (message), \
                                            ##__VA_ARGS__); \
} while (0)

#ifdef MTK_SYSLOG_VERSION_2

/** @brief This macro calls the SYSLOG internal function to print the user's log string with only the address information.
 *         The parameters are sent at the same time.
 *         This macro defines the user's log with an error level. Therefore, the user's log cannot be sent if the module's log level setting is higher.
 *         The PC logging tool should search and parse the log string using the address information from the target.
 * @param[in] module is the name of the log control module.
 * @param[in] message is the user's log string.
 * @param[in] arg_cnt is a count of the user's parameters.
 * @param[in] ... is the parameter list corresponding with the message string.
 * @note   The user must specify the number of parameters when calling the API.
 * @par Example
 * @code
 *      LOG_MSGID_E(common, "this is test log from LOG_MSGID_E() at %d", 1, timestamp);
 * @endcode
 * @sa #LOG_MSGID_D()/#LOG_MSGID_I()/#LOG_MSGID_W()
*/
#ifdef MTK_SYSLOG_SUB_FEATURE_MSGID_TO_STRING_LOG_SUPPORT
#define LOG_MSGID_E(module, message, arg_cnt, ...) \
    LOG_E(module, message, ##__VA_ARGS__)
#else
#define LOG_MSGID_E(module, message, arg_cnt, ...) \
do { \
     extern log_control_block_t log_control_block_##module; \
     ATTR_LOG_STRING msg_id_string[] = "[M:" #module " C:error F: L: ]: " message; \
     log_control_block_##module.msg_id_handle(&log_control_block_##module, \
                                            PRINT_LEVEL_ERROR, \
                                            (msg_id_string), \
                                            (arg_cnt),  \
                                            ##__VA_ARGS__); \
} while (0)
#endif

#else

#ifndef MTK_SYSLOG_GEN_DOCUMENT
#define LOG_MSGID_E(module, message, arg_cnt, ...) \
    LOG_E(module, message, ##__VA_ARGS__)
#endif

#endif

#ifdef MTK_SYSLOG_VERSION_2

/** @brief This macro calls the SYSLOG internal function to print the user's customized data with an error level.
 *         Therefore, the user's log cannot be sent if the module's log level setting is higher.
 *         This data must be managed with a specific method on the PC side.
 *         Therefore, the type must be set here so that the PC logging tool can split it for specific post processing.
 *         The user must check the return value to see whether all data is sent.
 *         If not, the user must call this API again to send the remaining data.
 * @param[in] module is the name of the log control module.
 * @param[in] type is a uniform value of #log_type_t used to tell the difference between the user's different logs and data.
 * @param[in] p_data is the address of the data buffer array. It must be end with NULL.
 * @param[in] p_len is the array of data buffer length.
 * @param[out] ret_len is the size actually sent.
 * @note   The user must check the return value to see whether all data is sent.
 * @par Example
 * @code
 *      uint8_t *audio_buffer_array[] = {audio_buffer_1, audio_buffer_2, NULL};
 *      uint32_t audio_buffer_length_array[] = {0x10, 0x200};
 *      LOG_TLVDUMP_E(common, LOG_TYPE_HCI_DATA, audio_buffer_array, audio_buffer_length_array, ret_len);
 * @endcode
 * @sa #LOG_TLVDUMP_D()/#LOG_TLVDUMP_I()/#LOG_TLVDUMP_W()
*/
#define LOG_TLVDUMP_E(module, type, p_data, p_len, ret_len) \
do { \
    extern log_control_block_t log_control_block_##module; \
    ret_len = log_control_block_##module.tlv_dump_handle(&log_control_block_##module, \
                                            PRINT_LEVEL_ERROR, \
                                            (type), \
                                            (const void **)(p_data), \
                                            (p_len)); \
} while (0)

#else

#ifndef MTK_SYSLOG_GEN_DOCUMENT
#define LOG_TLVDUMP_E(module, type, p_data, p_len, ret_len)
#endif

#endif

#else

#define LOG_E(module, message, ...)
#define LOG_HEXDUMP_E(module, message, data, len, ...)
#define LOG_MSGID_E(module, message, arg_cnt, ...)
#define LOG_TLVDUMP_E(module, type, p_data, p_len, ret_len)

#endif /* MTK_DEBUG_LEVEL_ERROR */


#if defined (MTK_DEBUG_LEVEL_NONE)

#define log_config_print_switch(module, log_switch)
#define log_config_print_level(module, level)
#define log_config_print_func(module, print_func)
#define log_config_dump_func(module, dump_func)
#ifdef MTK_SYSLOG_VERSION_2
#define log_config_tlv_dump_func(module, tlv_dump_func)
#define log_config_msg_id_func(module, msg_id_func)
#endif
#define LOG_CONTROL_BLOCK_DECLARE(module)
#define LOG_CONTROL_BLOCK_SYMBOL(module)

#else

/** @brief This macro turns the specific module's log on or off during runtime.
 * @param[in] module is the name of the log control module.
 * @param[in] switch is the status of the log switch.
 * @par Example
 * @code
 *      log_config_print_switch(common, DEBUG_LOG_OFF);
 * @endcode
*/
#ifdef MTK_SYSLOG_VERSION_2
#define log_config_print_switch(module, switch) \
do { \
    extern log_control_block_t log_control_block_##module; \
    filter_config_print_switch(&log_control_block_##module, switch); \
} while (0)
#else
#define log_config_print_switch(module, switch) \
do { \
    extern log_control_block_t log_control_block_##module; \
    log_control_block_##module.log_switch = (switch); \
} while (0)
#endif

/** @brief This macro sets the module's log level to a specific level during runtime.
 * @param[in] module is the name of the log control module.
 * @param[in] level is the new log level of the module.
 * @par Example
 * @code
 *      log_config_print_level(common, PRINT_LEVEL_WARNING);
 * @endcode
*/
#ifdef MTK_SYSLOG_VERSION_2
#define log_config_print_level(module, level) \
do { \
    extern log_control_block_t log_control_block_##module; \
    filter_config_print_level(&log_control_block_##module, level); \
} while (0)
#else
#define log_config_print_level(module, level) \
do { \
    extern log_control_block_t log_control_block_##module; \
    log_control_block_##module.print_level = (level); \
} while (0)
#endif

/** @brief This macro declares the specific log module in the place where it is used.
 * @param[in] module is the name of the log control module.
 * @par Example
 * @code
 *      LOG_CONTROL_BLOCK_DECLARE(common);
 * @endcode
*/
#define LOG_CONTROL_BLOCK_DECLARE(module) extern log_control_block_t log_control_block_##module

/** @brief This macro gets the log_control_block's address of the specific module.
 * @param[in] module is the name of the log control module.
 * @par Example
 * @code
 *      LOG_CONTROL_BLOCK_SYMBOL(common);
 * @endcode
*/
#define LOG_CONTROL_BLOCK_SYMBOL(module)  log_control_block_##module

#endif /* MTK_DEBUG_LEVEL_NONE */

#ifdef __cplusplus
}
#endif

/**
* @}
* @}
*/

#endif//__SYSLOG_H__


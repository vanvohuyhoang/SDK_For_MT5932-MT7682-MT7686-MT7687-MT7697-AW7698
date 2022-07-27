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

/**
 * @addtogroup aw7698_evk aw7698_evk
 * @{
 * @addtogroup aw7698_evk_templates templates
 * @{
 * @addtogroup aw7698_evk_templates_freertos_create_thread freertos_create_thread
 * @{

@par Overview
  - Example description
    - This example describes how to setup FreeRTOS multitasking environment.
      The project creates four tasks. Each task sleeps for a certain amount
      of time measured in OS ticks and prints the logs when it awakes.
  - Process / procedure of the example project
    - Declare and initialize four tasks and schedule at different intervals.
  - Results
    - Log will show when a task wakes up.

@par Hardware and software environment
  - Supported EVK
    - 7698 EVK.
  - EVK switches and pin configuration
    - N/A.
  - Environment configuration
    - The output logs are communicated through a micro USB cable to the PC
      from USB (CON5) connector on the EVK.
      - Install the mbed serial driver according to the instructions at
        https://developer.mbed.org/handbook/Windows-serial-configuration. For
        more information, please refer to section "Installing the 7698
        EVK drivers on Microsoft Windows" on the "7698 EVK User Guide"
        in [sdk_root]/doc folder.
      - Use a type-A to micro-B USB cable to connect type-A USB of the PC and
        MK20 micro-B USB connector on the 7698 EVK. For more
        information about the connector cable, please refer to
        https://en.wikipedia.org/wiki/USB#Mini_and_micro_connectors.
      - Launch a terminal emulator program, such as Tera terminal on your PC
        for data logging through UART. For the installation details, please
        refer to section "Installing Tera terminal on Microsoft Windows" on
        the "Airoha IoT SDK Get Started Guide" in [sdk_root]/doc folder.
      - COM port settings. baudrate: 115200, data bits: 8, stop bit: 1,
        parity: none and flow control: off.
      - Configure a Wi-Fi router with the SSID as SQA_TEST_AP, password as
        77777777, and authentication mode as WPA_PSK_WPA2_PSK. The Wi-Fi
        router should have the internet connection.
      - You can change the default settings in main.c to adapt the device to
        your Wi-Fi Accesss Point by specifying SSID and password.

@par Directory contents
  - Source and header files
    - \b src/main.c:            Main program.
    - \b src/system_aw7698.c:   The configuration file of the Cortex-M4 with
                                floating point core registers and system clock
    - \b src/sys_init.c:        This file initializes the hardware environment
                                for the system, including system clock, UART
                                port for logging, Pinmux, cache and other
                                necessary hardware.
    - \b src/ept_eint_var.c:    The EINT configuration file generated by Easy
                                Pinmux Tool (EPT). Please do not edit the file.
    - \b src/ept_gpio_var.c:    The GPIO configuration file generated by the
                                EPT. Please do not edit the file.
    - \b inc/FreeRTOSConfig.h:  FreeRTOS feature configuration file.
    - \b inc/task_def.h:        Define the task stack size, queue length,
                                project name, and priority for the application
                                to create tasks.
    - \b inc/hal_feature_config.h:
                                AW7698's feature configuration file.
    - \b inc/memory_map.h:      AW7698's memory layout symbol file.
    - \b inc/ept_eint_drv.h:      The EINT configuration file generated by Easy
                                  Pinmux Tool(EPT). Please do not edit the
                                  file.
    - \b inc/ept_gpio_drv.h:      The GPIO configuration file generated by Easy
                                  Pinmux Tool(EPT). Please do not edit the
                                        file.
    - \b inc/sys_init.h:        The header file of sys_init.c.
    - \b GCC/startup_aw7698.s:  AW7698's startup file for GCC.
    - \b GCC/syscalls.c:        The minimal implementation of the system calls.
  - Project configuration files using GCC
    - \b GCC/feature.mk:  Feature configuration.
    - \b GCC/Makefile.:   Makefile.
    - \b GCC/aw7698_flash.ld:   Linker script.

@par Run the demo
  - Build the example project with a command "./build.sh aw7698_evk
    freertos_create_thread" from the SDK root folder and download the binary
    file to 7698 EVK.
  - Connect the EVK to the PC with a type-A to micro-B USB cable and specify
    the port on Tera terminal corresponding to "mbed Serial Port".
  - Run the example. The log will show "hello world x", where x is the task
    ID with the range from 0 to 3.
*/
/**
 * @}
 * @}
 * @}
 */


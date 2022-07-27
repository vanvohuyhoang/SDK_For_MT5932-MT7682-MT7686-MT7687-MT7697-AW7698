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
 * @addtogroup aw7698_evk_templates_no_rtos_initialize_system no_rtos_initialize_system
 * @{

@par Overview
  - Example description
    - Potential users of this project are developers who want to create their
      own project without FreeRTOS and the purpose of this project is to
      demonstrate how to achieve this goal on the basis of this project.
    - This example demonstrates how to create a bare minimum system.
  - How to create your own project based on this project
    - Clone this project to apps folder and rename it to your own project
      name.
    - To add your own application code, please follow comments described in
      the main.c of this project.
  - Features of the example project
    - The project initializes the hardware and demonstrates how to create
      user-defined applications.
  - Result
    - A welcome message will display in the log on the terminal window.

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
    - \b inc/hal_feature_config.h:
                                AW7698's feature configuration file.
    - \b inc/memory_map.h:      AW7698's memory layout symbol file.
    - \b GCC/startup_aw7698.s:  AW7698's startup file for GCC.
    - \b GCC/syscalls.c:        The minimal implementation of the system calls.
  - Project configuration files using GCC
    - \b GCC/feature.mk:  Feature configuration.
    - \b GCC/Makefile.:   Makefile.

@par Run the demo
  - Build the example project with a command "./build.sh aw7698_evk
    no_rtos_initialize_system bl" from the SDK root folder and download the binary
    file to 7698 EVK.
  - Connect the EVK to the PC with a type-A to micro-B USB cable and specify
    the port on Tera terminal corresponding to "mbed Serial Port".
  - Run the example, the terminal window will show "welcome to main()"
    message and this indicates a successful operation.
*/
/**
 * @}
 * @}
 * @}
 */


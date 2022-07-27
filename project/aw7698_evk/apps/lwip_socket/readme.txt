/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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
 * @addtogroup aw7698_evk_apps apps
 * @{
 * @addtogroup aw7698_evk_apps_lwip_socket lwip_socket
 * @{

@par Overview
  - Application description
    - This application shows the usage of socket APIs.
  - Features of the application
    - This application demonstrates how to create a UDP/TCP client/Server and
      how to communicate between client and server.
    - This application explains how to:
    - 1. Connect to an access point.
    - 2. Create TCP/UDP server and wait client to connect.
    - 3. Create TCP client to connect to a server and send data to the
      server. The server echoes the received data back to the client.
    - 4. Create UDP client to connect the server and send data to the server.
      The server echoes the received data back to the client.
  - Results
    - The output is in a log.

@par Hardware and software environment
  - Supported platform
    - 7698 EVK.
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
      - You can change the SSID and password in main.c to match these of your
        Wi-Fi Access Point.

@par Directory contents
  - Source and header files.
    - \b src/main.c:              Main program file.
    - \b src/system_aw7698.c:     The configuration file of the Cortex-M4 with
                                  floating point core registers and system
                                  clock.
    - \b src/sys_init.c:          This file initializes the basic hardware
                                  environment for the system, including system
                                  clock, UART port for logging, Pinmux, cache
                                  and other necessary hardware.
    - \b src/wifi_lwip_helper.c:  Network helper function for Wi-Fi station
                                  mode initialization until IP network ready.
    - \b src/ept_eint_var.c:      The EINT configuration file generated by Easy
                                  Pinmux Tool (EPT). Please do not edit the
                                  file.
    - \b src/ept_gpio_var.c:      The GPIO configuration file generated by the
                                  EPT. Please do not edit the file.
    - \b inc/FreeRTOSConfig.h:    FreeRTOS feature configuration file.
    - \b inc/task_def.h:          Define the task stack size, queue length,
                                  project name, and priority for the
                                  application to create tasks.
    - \b inc/hal_feature_config.h:
                                  AW7698 feature configuration file.
    - \b inc/flash_map.h:         AW7698 memory layout symbol file.
    - \b inc/ept_eint_drv.h:      The EINT configuration file generated by the
                                  EPT. Please do not edit the file.
    - \b inc/ept_gpio_drv.h:      The GPIO configuration file generated by the
                                  EPT. Please do not edit the file.
    - \b inc/sys_init.h:          The header file of sys_init.c.
    - \b inc/wifi_lwip_helper.h:  The header file of wifi_lwip_helper.c.
    - \b inc/lwipopts.h:          The configuration file of tcpip feature.
  - Project configuration files using GCC
    - \b GCC/feature.mk:        Feature configuration.
    - \b GCC/makefile:          Makefile.
    - \b GCC/aw7698_flash.ld:   Linker script.
    - \b GCC/startup_aw7698.s:  AW7698 startup file for GCC.
    - \b GCC/syscalls.c:        The minimal implementation of the system calls.

@par Run the application
  - Build the application with the command, "./build.sh aw7698_evk
    lwip_socket" from the SDK root folder and download the bin file to
    7698 EVK.
  - Connect the EVK to the PC with a micro USB cable.
  - Run the application. The result is displayed in the log. "example project
    test success" printed in the log indicates a success.
*/
/**
 * @}
 * @}
 * @}
 */


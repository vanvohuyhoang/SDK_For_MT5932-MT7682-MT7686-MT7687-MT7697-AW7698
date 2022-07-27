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
 * @addtogroup aw7698_evk_apps_ble_mesh_switch ble_mesh_switch
 * @{
 
@par Overview
  - Application description
    - This is a reference application to demonstrate the Bluetooth mesh switch and Wi-Fi
      gateway features of the AW7698 EVK
      through the following:
      - How to use the Bluetooth mesh and Wi-Fi gateway functions.
  - Application features
    - Act as a Wi-Fi station to connect to a Wi-Fi network.
    - Act as a Bluetooth low-energy (LE) device with mesh features. All Bluetooth Mesh Command Line
      Interface (CLI) commands are supported in this project.

@par Hardware and software environment
  - Supported platform
    - Airoha AW7698 EVK
  - EVK switches and pin configuration
    - J36 provides the pins for GPIOs, PWMs, SPI master chip select 0, SPI
      master, and UART1 RX/TX.
    - J35 provides the pins for GPIOs, PWMs, UART2 RX/TX, UART1 RTS/CTS, SPI
      master chip select 1, IR TX, and IR RX.
    - J34 provides the pins for GPIOs, PWMs, UART2 RTS/CTS, I2S, SPI slave,
      and I2C0.
    - J33 provides the pins for GPIOs, PWMs, I2C0, and ADC0~3.
    - J32 provides the pins for GND, 5V, 3.3V, and the reset pin.
    - J25 sets the EVK to either Flash Normal mode or Flash Recovery mode. To
      update the firmware on the AW7698 EVK:
      - Set the jumper J25 to FLASH Recovery mode. Jumpers J23, J26, J27, 
        and J30 must be on.
      - In this mode, if the power is on, the board loads the ROM code and
        starts the ATE Daemon or Firmware Upgrade Daemon according to the
        MT76x7 Flash Tool's behavior on the PC. To run the project on the
        AW7698 EVK:
      - Set the jumper J25 off to switch to FLASH Normal mode. Jumpers
        J23, J26, J27, and J30 must be on.
      - In this mode, if the power is on, the board loads the firmware from
        the flash and reboots.
    - There are three buttons on the board:
      - RST - Reset
      - EINT - External interrupt
      - RTC_INT - RTC interrupt
  - Environment configuration
    - A serial tool is necessary for UART logging.
    - COM port settings -baudrate: 115200, data bits: 8, stop bit: 1, parity:
      none, and flow control: off.

@par Directory contents
  - This file
    - \b readme.txt.                        Overview of the project
  - Source and header files
    - \b use files in ble_mesh_vendor_device project. All the files are used except the file 
      mesh_app_vendor_device.c, mesh_app_vendor_device_msg_handler.c and mesh_app_vendor_device_pwm.c.
  - Project configuration files using GCC
    - \b GCC/Makefile.:                     GNU Makefile for this project

@par Run the application
  - Build the example project with the command "./build.sh aw7698_evk
    ble_mesh_switch" from the SDK root folder and download the binary file to the
    AW7698 development board.
  - Reboot the EVK. The console shows the "FreeRTOS Running" message to
    indicate that the EVK is booting up.
  - Use '?' and ENTER to query the available command line options. Note that
    the command line options are still under development and subject to
    change without notice.
  - Reference applications run the AW7698 EVK as a Wi-Fi station and acts as a provisioner to 
    add other mesh devices to the mesh network via Bluetooth LE.
  - Example 1. Wi-Fi station mode
    - Find your Wi-Fi access point settings: Before connecting to a Wi-Fi
      access point, the following information must be collected: 1. The
      SSID of your Wi-Fi access point. 2. The authentication mode of your Wi-
      Fi access point. The authentication mode is usualy WPA PSK or WPA2
      PSK. To change the mode, please refer to Table 1 for the list of
      supported authentication modes. 3. The password of your Wi-Fi access
      point. 4. The encryption mode of your Wi-Fi access point (AES or TKIP is usually used). 
      To change the mode, please refer to Table 2 for
      the list of supported encryption modes.
    - When the information is collected, use the following commands to
      configure the AW7698 EVK. The example code in main.c assumes that
      either WPA PSK or WPA2 PSK is used for authentication, TKIP or AES is 
      used for encryption, 'myhome' (length 6) is used for the SSID, and the
      WPA or WPA2 password is '12345678' (length 8).
      \code
      config write STA AuthMode 9
      config write STA EncrypType 8
      config write STA Ssid myhome
      config write STA SsidLen 6
      config write STA WpaPsk 12345678
      config write STA WpaPskLen 8
      config write common OpMode 1
      \endcode press the RESET button on the AW7698 EVK to restart the system.
    - Boot up with the new configuration. If everything is correct, the same
      messages are shown in the console to tell you that the EVK has received
      an IP address.
      \code
      ************************
      DHCP got IP:10.10.10.101
      ************************
      \endcode
    - PING from the AW7698 EVK (SDK v3.1.0) If the IP address is fetched
      and the network is operating, AW7698 can ping other devices on
      the network with the following command in the console.
      \code
      ping 10.10.10.254 3 64
      The ping stops after sending three packets to 10.10.10.254.
      The ping usage is: ping <ip address> <times> <ping packet length>
      \endcode
    - Wi-Fi configuration options for AuthMode and EncrypType.
      \code
      +---+-------------------------------------+
      | 0 | Open(i.e. no security)              |
      +---+-------------------------------------+
      | 4 | WPA PSK                             |
      +---+-------------------------------------+
      | 7 | WPA2 PSK                            |
      +---+-------------------------------------+
      | 9 | Support for both WPA and WPA2 PSK   |
      +---+-------------------------------------+
      Table 1. Supported AuthMode(s)
      +---+---------------------------------+
      | 0 | WEP                             |
      +---+---------------------------------+
      | 1 | No encryption                   |
      +---+---------------------------------+
      | 4 | TKIP                            |
      +---+---------------------------------+
      | 6 | AES                             |
      +---+---------------------------------+
      | 8 | Support for both TKIP and AES   |
      +---+---------------------------------+
      Table 2. Supported EncrypType(s)
      \endcode
  - Example 2. Using the BLE mesh feature
    - This example creates a BLE mesh device with one mesh element in which there are 14 models: 
      - configuration server model; 
      - health server model; 
      - vendor model; 
      - configuration client model; 
      - health client model; 
      - generic on/off client model; 
      - generic level client model; 
      - lightness client model; 
      - CTL client model; 
      - HSL client model;
      - OTA object transfer client model; 
      - OTA firmware distribution server model; 
      - OTA firmware update client model; 
      - OTA firmware distribution client model.
    - The mesh feature is enabled after Bluetooth powers on successfully.
      A similar log is written to the output:
      \code
      [T: xxxx M: mesh_app C: info F: bt_mesh_app_event_callback L: 272]: BLE_MESH_EVT_INIT_DONE
      \endcode
    - It supports PB-ADV and PB-GATT bearers to provision a device to a network.
      - PB-ADV Provision
        - Input the command "mesh config addr [device address]" on the serial
          tool to assign the address for the primary mesh element. For example, 
          "mesh config addr 0001".
        - Input the command "mesh config iv [ivindex] 0" on the serial
          tool to initiate the IV index for the network. For example, 
          "mesh config iv 0 0".
        - Input the command "mesh key add net [netkey] [netkey index]" on the serial
          tool to add the netkey to the mesh stack. This is used when creating a network.
          - netkey, a random number with 16 bytes.
          - netkey index, assign an index for the above netkey, starting at 0.
          - For example, "mesh key add net 12345678901234567890123456789012 0".
        - Input the command "mesh key add app [appkey] [appkey index] [netkey index]" on the serial
          tool to add the app key to the mesh stack. 
          - appkey, random number with 16 bytes.
          - appkey index, is currently 0x123 in this project.
          - netkey index, same as the one above.
          - For example, "mesh key add app 63964771734fbd76e3b40519d1d94a48 0x123 0".
        - Input the command "mesh config bind" on the serial
          tool to bind the vendor model/config model with app key index "0x123".    
        - Create a network
          Input the command "mesh prov run [remote device UUID] [netkey] [netkey index] [ivindex] [address] [flags]" on the serial
          tool to add the specific mesh device to the mesh network.
          - device UUID of vendor device, 16-bytes.
          - netkey, the same netkey when input "mesh key add net".
          - netkey index, the same netkey when input "mesh key add net".
          - ivindex, usually use 0.
          - address, 2 bytes, the address assigned to the specific mesh device when it is successfully added to the network.
          - flags, indicate current states of IV update and key refresh. Usually use 0.
          - For example "mesh prov run 04112233445566778899aabbccddeeff 12345678901234567890123456789012 0 0 0004 0".
      - PB-GATT Provision
        - Input the command "mesh config addr [device address]" on the serial
          tool to assign the address for the primary mesh element. For example, 
          "mesh config addr 0015".
        - Input the command "mesh config iv [ivindex] 0" on the serial
          tool to initiate the IV index for the network. For example, 
          "mesh config iv 0 0".
        - Input the command "mesh key add net [netkey] [netkey index]" on the serial
          tool to add the netkey to the mesh stack. This is used when creating a 
          network.
          - netkey, random number with 16 bytes.
          - netkey index, assign an index for the above netkey, starting at 0.
          - For example, "mesh key add net 12345678901234567890123456789012 0".
        - Input the command "mesh key add app [appkey] [appkey index] [netkey index]" on the serial
          tool to add the app key to the mesh stack. 
          - appkey, random number with 16 bytes.
          - appkey index, is currently 0x123 in this project.
          - netkey index, same as the one above.
          - for example, "mesh key add app 63964771734fbd76e3b40519d1d94a48 0x123 0".
        - Input the command "mesh config bind" in the serial
          tool to bind the vendor model/config model with app key index "0x123".
        - Create a network.
          - Firstly, input the command "mesh prov init [netkey] [netkey index] [ivindex] [address] [flags]" on the serial
            tool to add the specific mesh device to the mesh network.
            - netkey, the same netkey when input "mesh key add net".
            - netkey index, the same netkey when input "mesh key add net".
            - ivindex, usually use 0.
            - address, 2 bytes, the address assigned to the specific mesh device when it is successfully added to the network.
            - flags, indicate current states of IV update and key refresh. Usually use 0.
            - For example "mesh prov init 12345678901234567890123456789012 0 0 0014 0".
          - Then, input the command "mesh gatt connect [addr type] [remote bluetooth addr] [service type]" on the serial
            tool to create a Mesh GATT connection.
            - addr type, the address type of [remote bluetooth addr]. 0 = a public address, 1 = a random address.
            - remote bluetooth addr, the Bluetooth address of the remote device when advertising.
            - service type, 0 means proxy, 1 means provision.
            - For example, "mesh gatt connect 0 C6:29:AB:B7:48:25 1".
      - When the provisioning is complete, the log shows "BLE_MESH_EVT_PROV_DONE SUCCESS" or "BLE_MESH_EVT_PROV_DONE FAILED".
      - Then, some configuration processes will automatically run, incluing Get composition data, Add appkey, and Model Application 
        binding. When the configuration is complete, the log shows "no more model, all model binding is done"
      - When the above procedure is complete, the provision and configuration processes are done, then we can test the detail features by sending messages on the serial
        tool. For example, input the command "mesh msg onoff set [dst_addr] [onoff] [reliable] [ttl]" to use advertising to turn on/off each light with 
	a target address.
	- dst_addr, is the destination address.
	- OnOff, the target value if it is in the Generic OnOff state.
	- reliable, is to send the request as an acknowledged message or not.
	- ttl, is the received TTL value.
	- For example, send "mesh msg onoff set 0014 1 1 5".
      - To delete the mesh data of all nodes and the local device, use the command "mesh delete". It takes effect after rebooting again. 
        It does not delete the mesh data in those nodes which cannot receive the Node Reset message, but the data of the node in the local device will be deleted after retrying five times 
	without any response from the specific node. 
      - For more information about other commands, please refer to the cmds[] structure in the mesh_app_switch.c file.
  - Example 3. Using the BLE mesh/wifi gateway feature
    - This example allows the AW7698 EVK to act as a gateway to send the Wi-Fi data to a BLE mesh device.
    - With the procedure in the Example 1, the AW7698 EVK is connected to the laptop via WI-FI and has an IP address.
    - Connect the laptop to the AW7698 EVK via Telnet.
      Use the Telnet protocol with port 23 to connect to the IP address of the AW7698 EVK.
    - Then Input the commands in Example 2 to complete the network configuration and creation.
  - Example 4. Using the BLE mesh OTA feature.
    - The mesh OTA feature is an Airoha proprietary feature based on Bluetooth specification "Mesh Firware Update proposal v05r05".  
      It only supports initiator and distributor, but does not support updator.
    - This example allows the AW7698 EVK to act as an initiator and distributor of the OTA.
    - As an initiator and distributor, the user must implement the callback function in the bt_mesh_ota_distributor_new_firmware_t.
      For more information, please refer to the example file "mesh_app_switch_firmware_callbacks.c".
    - The network configuration and creation process is complete when you complete the procedure shown in Example 2.
    - Input the command "mesh config subscribe 0 [dst_addr] [element_addr] [group_addr] 0xff00" on the serial 
      tool to configure the subscription list on the peer nodes to subscribe to the OTA model 0xff00 using the specific group address. 
      Input the command "mesh config subscribe 0 [dst_addr] [element_addr] [group_addr] 0xfe00" on the serial 
      tool to configure the subscription list on the peer nodes to subscribe to the OTA model 0xfe00 using the specific group address.
      - 0, is the "add" action.
      - dst_addr, is the address of the primary element in the destination node.
      - element_addr, the address of the element to which the model ID belongs.
      - group_addr, is the group address for the subscription.
      - 0xff00, is the model ID of object transfer server model.
      - 0xfe00, is the model ID of firmware update server model.
      - For example, send "mesh config subscribe 0 0x0010 0x0011 0xc000 0xff00". 
    - Input the command "mesh dist start auto/manual [firmware_id] [distributor_addr] [group_addr] [node1_addr] [node2_addr2]... [noden_addr]" (n is less than 30) on the serial 
      tool to start the OTA.
      - "auto" means that application will start automaticly when the transfer is complete; "manual" means that the user must manually apply when the transfer is complete.
      - firmware_id, is the firmware ID to distinguish between different firmware binaries of the same product. 
      - distributor_addr, is the address of the primary element of the OTA distributor.
      - group_addr, is the group address for the subscription.
      - nodex_addr, is the address of the primary element of the node x to update the firmware.
      - For example, send "mesh dist start auto 0xaabbccdd 0x0001 0xc000 0x0010".
      - This command can be queued in the mesh stack if the command is input repeatedly and the heap memory is enough for the operation.
    - Input the command "mesh dist apply" on the serial 
      tool to apply the firmware if the user uses "mesh dist start manual.." in the previous command.
*/
/**
 * @}
 * @}
 * @}
 */

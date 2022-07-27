Bluetooth Mesh Module Usage Guide

Brief:           This module is the implementation of the Bluetooth Mesh Profile Specification V1.0 (with the fix for the SIG erratum 10395) and Mesh Model Specification V1.0.
                 It supports:
                 - Mesh Relay features
                 - Proxy features
                 - Friend features
                 - Initiator and distributor of OTA features (draft v0.5r05)
                 - Mesh security
                 - Mesh beacons
Usage:           GCC - The following items must be included in your GCC project makefile:
                      1. The mesh module for compiling, which contains libs, source files, and the header file path; include $(SOURCE_DIR)/middleware/MTK/mesh/module.mk
                      2. Set MTK_BT_MESH_ENABLE = y in /GCC/feature.mk
Dependency:      This module has the dependency with Bluetooth Low Energy. Please make sure to also include Bluetooth.
                 - include $(SOURCE_DIR)/middleware/MTK/bluetooth/module.mk
                 Set MTK_BT_ENABLE = y. MTK_BLE_ONLY_ENABLE must be set to 'y' if you only support Bluetooth Low-Energy (BLE) in your project.
Notice:          Mesh features can be configured dynamically by the device itself. Please refer to the API reference in the SDK file "bt_mesh_config.h".
Related doc:     Please refer to the Bluetooth Mesh Developer's Guide on mt7697x under the doc folder for more information.
Example project: Please find the project under [sdk_root]/project/mt7697_hdk/apps/ble_mesh_switch and ble_mesh_vendor_device.


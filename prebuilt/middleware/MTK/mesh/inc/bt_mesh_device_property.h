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

#ifndef __BT_MESH_DEVICE_PROPERTY_H__
#define __BT_MESH_DEVICE_PROPERTY_H__

#include <stdbool.h>
#include <stdint.h>
#include "bt_mesh_common.h"
/**
 *   @addtogroup BluetoothMesh Mesh
 *   @{
 *   @addtogroup BluetoothMeshProperty Property
 *   @{
 *   This section introduces the mesh device property APIs.
*/

/**
 * @defgroup Bluetooth_mesh_property_enum Enum
 * @{
*/

/** @brief Device property type */
typedef enum {
    BT_MESH_DEVICE_PROPERTY_TYPE_USER = 0x01,          /**< User accessible device property. */
    BT_MESH_DEVICE_PROPERTY_TYPE_ADMIN = 0x02,         /**< Administrator accessible device property. */
    BT_MESH_DEVICE_PROPERTY_TYPE_MANUFACTURER = 0x03,  /**< Manufacturer accessible device property. */
} bt_mesh_device_property_type_t;

/** @brief An enumeration indicating whether the device property can be read or written as a Generic User Property. */
typedef enum {
    BT_MESH_DEVICE_PROPERTY_ACCESS_PROHIBITED = 0x00,      /**< Prohibited. */
    BT_MESH_DEVICE_PROPERTY_ACCESS_READ = 0x01,            /**< The device property can be read. */
    BT_MESH_DEVICE_PROPERTY_ACCESS_WRITTEN = 0x02,         /**< The device property can be written. */
    BT_MESH_DEVICE_PROPERTY_ACCESS_READ_WRITTEN = 0x03,    /**< The device property can be read and written. */
} bt_mesh_device_property_user_access_t;

/** @brief An enumeration indicating whether the device property can be read or written as a Generic User Property */
typedef enum {
    BT_MESH_DEVICE_PROPERTY_ACCESS_ADMIN_IS_NOT_A_GENERIC_USER_PROPERTY = 0x00,    /**< The device property is not a Generic User Property. */
    BT_MESH_DEVICE_PROPERTY_ACCESS_ADMIN_READ = 0x01,              /**< The device property is a Generic User Property and can be read. */
    BT_MESH_DEVICE_PROPERTY_ACCESS_ADMIN_WRITTEN = 0x02,           /**< The device property is a Generic User Property and can be written. */
    BT_MESH_DEVICE_PROPERTY_ACCESS_ADMIN_READ_WRITTEN = 0x03,      /**< The device property is a Generic User Property and can be read and written. */
} bt_mesh_device_property_admin_access_t;

/** @brief An enumeration indicating whether or not the device property can be read as a Generic User Property. */
typedef enum {
    BT_MESH_DEVICE_PROPERTY_MANUFACTURER_IS_NOT_A_GENERIC_USER_PROPERTY = 0x00,    /**< The device property is not a Generic User Property. */
    BT_MESH_DEVICE_PROPERTY_MANUFACTURER_IS_A_GENERIC_USER_PROPERTY = 0x01,        /**< The device property is a Generic User Property and can be read. */
} bt_mesh_device_property_manufacturer_access_t;
/**
 * @}
 */
/**
 * @defgroup Bluetooth_mesh_property_struct Struct
 * @{
 */

/** @brief Device property data structure. */
typedef struct {
    uint16_t id;        /**< Property ID */
    uint8_t *value;     /**< Property value */
    uint16_t length;    /**< Property value length */
    bt_mesh_device_property_user_access_t access;  /**< The access field of this property */
    bool is_user_property;  /**< User accessibility */
} __attribute__((packed)) bt_mesh_device_property_t;
/**
 * @}
 */

/**
 * @defgroup Bluetooth_mesh_device_property_define Define
 * @{
*/

/*!
    @name Property identifiers
    @brief The identifiers of all properties.
 */
#define	PROPERTY_ID_PROHIBITED	0x0000
#define	PROPERTY_ID_AVERAGE_AMBIENT_TEMPERATURE_IN_A_PERIOD_OF_DAY	0x0001
#define	PROPERTY_ID_AVERAGE_INPUT_CURRENT	0x0002
#define	PROPERTY_ID_AVERAGE_INPUT_VOLTAGE	0x0003
#define	PROPERTY_ID_AVERAGE_OUTPUT_CURRENT	0x0004
#define	PROPERTY_ID_AVERAGE_OUTPUT_VOLTAGE	0x0005
#define	PROPERTY_ID_CENTER_BEAM_INTENSITY_AT_FULL_POWER	0x0006
#define	PROPERTY_ID_CHROMATICITY_TOLERANCE	0x0007
#define	PROPERTY_ID_COLOR_RENDERING_INDEX_R9	0x0008
#define	PROPERTY_ID_COLOR_RENDERING_INDEX_RA	0x0009
#define	PROPERTY_ID_DEVICE_APPEARANCE	0x000A
#define	PROPERTY_ID_DEVICE_COUNTRY_OF_ORIGIN	0x000B
#define	PROPERTY_ID_DEVICE_DATE_OF_MANUFACTURE	0x000C
#define	PROPERTY_ID_DEVICE_ENERGY_USE_SINCE_TURN_ON	0x000D
#define	PROPERTY_ID_DEVICE_FIRMWARE_REVISION	0x000E
#define	PROPERTY_ID_DEVICE_GLOBAL_TRADE_ITEM_NUMBER	0x000F
#define	PROPERTY_ID_DEVICE_HARDWARE_REVISION	0x0010
#define	PROPERTY_ID_DEVICE_MANUFACTURER_NAME	0x0011
#define	PROPERTY_ID_DEVICE_MODEL_NUMBER	0x0012
#define	PROPERTY_ID_DEVICE_OPERATING_TEMPERATURE_RANGE_SPECIFICATION	0x0013
#define	PROPERTY_ID_DEVICE_OPERATING_TEMPERATURE_STATISTICAL_VALUES	0x0014
#define	PROPERTY_ID_DEVICE_OVER_TEMPERATURE_EVENT_STATISTICS	0x0015
#define	PROPERTY_ID_DEVICE_POWER_RANGE_SPECIFICATION	0x0016
#define	PROPERTY_ID_DEVICE_RUNTIME_SINCE_TURN_ON	0x0017
#define	PROPERTY_ID_DEVICE_RUNTIME_WARRANTY	0x0018
#define	PROPERTY_ID_DEVICE_SERIAL_NUMBER	0x0019
#define	PROPERTY_ID_DEVICE_SOFTWARE_REVISION	0x001A
#define	PROPERTY_ID_DEVICE_UNDER_TEMPERATURE_EVENT_STATISTICS	0x001B
#define	PROPERTY_ID_INDOOR_AMBIENT_TEMPERATURE_STATISTICAL_VALUES	0x001C
#define	PROPERTY_ID_INITIAL_CIE_1931_CHROMATICITY_COORDINATES	0x001D
#define	PROPERTY_ID_INITIAL_CORRELATED_COLOR_TEMPERATURE	0x001E
#define	PROPERTY_ID_INITIAL_LUMINOUS_FLUX	0x001F
#define	PROPERTY_ID_INITIAL_PLANCKIAN_DISTANCE	0x0020
#define	PROPERTY_ID_INPUT_CURRENT_RANGE_SPECIFICATION	0x0021
#define	PROPERTY_ID_INPUT_CURRENT_STATISTICS	0x0022
#define	PROPERTY_ID_INPUT_OVER_CURRENT_EVENT_STATISTICS	0x0023
#define	PROPERTY_ID_INPUT_OVER_RIPPLE_VOLTAGE_EVENT_STATISTICS	0x0024
#define	PROPERTY_ID_INPUT_OVER_VOLTAGE_EVENT_STATISTICS	0x0025
#define	PROPERTY_ID_INPUT_UNDER_CURRENT_EVENT_STATISTICS	0x0026
#define	PROPERTY_ID_INPUT_UNDER_VOLTAGE_EVENT_STATISTICS	0x0027
#define	PROPERTY_ID_INPUT_VOLTAGE_RANGE_SPECIFICATION	0x0028
#define	PROPERTY_ID_INPUT_VOLTAGE_RIPPLE_SPECIFICATION	0x0029
#define	PROPERTY_ID_INPUT_VOLTAGE_STATISTICS	0x002A
#define	PROPERTY_ID_LIGHT_CONTROL_AMBIENT_LUXLEVEL_ON	0x002B
#define	PROPERTY_ID_LIGHT_CONTROL_AMBIENT_LUXLEVEL_PROLONG	0x002C
#define	PROPERTY_ID_LIGHT_CONTROL_AMBIENT_LUXLEVEL_STANDBY	0x002D
#define	PROPERTY_ID_LIGHT_CONTROL_LIGHTNESS_ON	0x002E
#define	PROPERTY_ID_LIGHT_CONTROL_LIGHTNESS_PROLONG	0x002F
#define	PROPERTY_ID_LIGHT_CONTROL_LIGHTNESS_STANDBY	0x0030
#define	PROPERTY_ID_LIGHT_CONTROL_REGULATOR_ACCURACY	0x0031
#define	PROPERTY_ID_LIGHT_CONTROL_REGULATOR_KID	0x0032
#define	PROPERTY_ID_LIGHT_CONTROL_REGULATOR_KIU	0x0033
#define	PROPERTY_ID_LIGHT_CONTROL_REGULATOR_KPD	0x0034
#define	PROPERTY_ID_LIGHT_CONTROL_REGULATOR_KPU	0x0035
#define	PROPERTY_ID_LIGHT_CONTROL_TIME_FADE	0x0036
#define	PROPERTY_ID_LIGHT_CONTROL_TIME_FADE_ON	0x0037
#define	PROPERTY_ID_LIGHT_CONTROL_TIME_FADE_STANDBY_AUTO	0x0038
#define	PROPERTY_ID_LIGHT_CONTROL_TIME_FADE_STANDBY_MANUAL	0x0039
#define	PROPERTY_ID_LIGHT_CONTROL_TIME_OCCUPANCY_DELAY	0x003A
#define	PROPERTY_ID_LIGHT_CONTROL_TIME_PROLONG	0x003B
#define	PROPERTY_ID_LIGHT_CONTROL_TIME_RUN_ON	0x003C
#define	PROPERTY_ID_LUMEN_MAINTENANCE_FACTOR	0x003D
#define	PROPERTY_ID_LUMINOUS_EFFICACY	0x003E
#define	PROPERTY_ID_LUMINOUS_ENERGY_SINCE_TURN_ON	0x003F
#define	PROPERTY_ID_LUMINOUS_EXPOSURE	0x0040
#define	PROPERTY_ID_LUMINOUS_FLUX_RANGE	0x0041
#define	PROPERTY_ID_MOTION_SENSED	0x0042
#define	PROPERTY_ID_MOTION_THRESHOLD	0x0043
#define	PROPERTY_ID_OPEN_CIRCUIT_EVENT_STATISTICS	0x0044
#define	PROPERTY_ID_OUTDOOR_STATISTICAL_VALUES	0x0045
#define	PROPERTY_ID_OUTPUT_CURRENT_RANGE	0x0046
#define	PROPERTY_ID_OUTPUT_CURRENT_STATISTICS	0x0047
#define	PROPERTY_ID_OUTPUT_RIPPLE_VOLTAGE_SPECIFICATION	0x0048
#define	PROPERTY_ID_OUTPUT_VOLTAGE_RANGE	0x0049
#define	PROPERTY_ID_OUTPUT_VOLTAGE_STATISTICS	0x004A
#define	PROPERTY_ID_OVER_OUTPUT_RIPPLE_VOLTAGE_EVENT_STATISTICS	0x004B
#define	PROPERTY_ID_PEOPLE_COUNT	0x004C
#define	PROPERTY_ID_PRESENCE_DETECTED	0x004D
#define	PROPERTY_ID_PRESENT_AMBIENT_LIGHT_LEVEL	0x004E
#define	PROPERTY_ID_PRESENT_AMBIENT_TEMPERATURE	0x004F
#define	PROPERTY_ID_PRESENT_CIE_1931_CHROMATICITY_COORDINATES	0x0050
#define	PROPERTY_ID_PRESENT_CORRELATED_COLOR_TEMPERATURE	0x0051
#define	PROPERTY_ID_PRESENT_DEVICE_INPUT_POWER	0x0052
#define	PROPERTY_ID_PRESENT_DEVICE_OPERATING_EFFICIENCY	0x0053
#define	PROPERTY_ID_PRESENT_DEVICE_OPERATING_TEMPERATURE	0x0054
#define	PROPERTY_ID_PRESENT_ILLUMINANCE	0x0055
#define	PROPERTY_ID_PRESENT_INDOOR_AMBIENT_TEMPERATURE	0x0056
#define	PROPERTY_ID_PRESENT_INPUT_CURRENT	0x0057
#define	PROPERTY_ID_PRESENT_INPUT_RIPPLE_VOLTAGE	0x0058
#define	PROPERTY_ID_PRESENT_INPUT_VOLTAGE	0x0059
#define	PROPERTY_ID_PRESENT_LUMINOUS_FLUX	0x005A
#define	PROPERTY_ID_PRESENT_OUTDOOR_AMBIENT_TEMPERATURE	0x005B
#define	PROPERTY_ID_PRESENT_OUTPUT_CURRENT	0x005C
#define	PROPERTY_ID_PRESENT_OUTPUT_VOLTAGE	0x005D
#define	PROPERTY_ID_PRESENT_PLANCKIAN_DISTANCE	0x005E
#define	PROPERTY_ID_PRESENT_RELATIVE_OUTPUT_RIPPLE_VOLTAGE	0x005F
#define	PROPERTY_ID_RELATIVE_DEVICE_ENERGY_USE_IN_A_PERIOD_OF_DAY	0x0060
#define	PROPERTY_ID_RELATIVE_DEVICE_RUNTIME_IN_A_GENERIC_LEVEL_RANGE	0x0061
#define	PROPERTY_ID_RELATIVE_EXPOSURE_TIME_IN_AN_ILLUMINANCE_RANGE	0x0062
#define	PROPERTY_ID_RELATIVE_RUNTIME_IN_A_CORRELATED_COLOR_TEMPERATURE_RANGE	0x0063
#define	PROPERTY_ID_RELATIVE_RUNTIME_IN_A_DEVICE_OPERATING_TEMPERATURE_RANGE	0x0064
#define	PROPERTY_ID_RELATIVE_RUNTIME_IN_AN_INPUT_CURRENT_RANGE	0x0065
#define	PROPERTY_ID_RELATIVE_RUNTIME_IN_AN_INPUT_VOLTAGE_RANGE	0x0066
#define	PROPERTY_ID_SHORT_CIRCUIT_EVENT_STATISTICS	0x0067
#define	PROPERTY_ID_TIME_SINCE_MOTION_SENSED	0x0068
#define	PROPERTY_ID_TIME_SINCE_PRESENCE_DETECTED	0x0069
#define	PROPERTY_ID_TOTAL_DEVICE_ENERGY_USE	0x006A
#define	PROPERTY_ID_TOTAL_DEVICE_OFF_ON_CYCLES	0x006B
#define	PROPERTY_ID_TOTAL_DEVICE_POWER_ON_CYCLES	0x006C
#define	PROPERTY_ID_TOTAL_DEVICE_POWER_ON_TIME	0x006D
#define	PROPERTY_ID_TOTAL_DEVICE_RUNTIME	0x006E
#define	PROPERTY_ID_TOTAL_LIGHT_EXPOSURE_TIME	0x006F
#define	PROPERTY_ID_TOTAL_LUMINOUS_ENERGY	0x0070
/**
 * @}
 */

/*!
    @brief Add a device property
    @param[in] type is type of this device property.
    @param[in] property is the device property.
    @return
    #BT_MESH_ERROR_NOT_INIT, device property module not initialized.\n
    #BT_MESH_ERROR_OOM, not enough memory for adding the device property. \n
    #BT_MESH_ERROR_NULL, the property is NULL.
*/
bt_mesh_status_t bt_mesh_device_property_add_property(
    bt_mesh_device_property_type_t type, bt_mesh_device_property_t *property);

/*!
    @brief Check property value.
    @param[in] id is the property ID.
    @param[in] value is the property value.
    @param[in] length is the length of property value.
    @return
    @c true means this property value is valid.\n
    @c false means this property value is invalid.
*/
bool bt_mesh_device_property_check_property_value(
    uint16_t id, uint8_t *value, uint16_t length);

/*!
@}
@}
@}
*/

#endif // __BT_MESH_DEVICE_PROPERTY_H__



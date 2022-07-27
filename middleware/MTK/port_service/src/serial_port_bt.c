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
 
#ifdef MTK_PORT_SERVICE_ENABLE
#include "serial_port.h"
#include "serial_port_internal.h"
#include "syslog.h"
#include "hal_log.h"
#ifdef MTK_PORT_SERVICE_BT_ENABLE
#include "serial_port_bt.h"
#include "ble_air_interface.h"
#include "spp_air_interface.h"
#ifdef MTK_AIRUPDATE_ENABLE

#include "airupdate_interface.h"
#endif
#include "syslog.h"


log_create_module(BT_PORT, PRINT_LEVEL_INFO);

typedef enum {
	BLE_AIR_TYPE = 0,
	SPP_AIR_TYPE = 1,
#ifdef MTK_AIRUPDATE_ENABLE
	AIRUPDATE_TYPE = 2,
#endif
} serial_port_bt_type_t;

typedef struct {
    bool           initialized;
	bool           connected;
    uint32_t       conn_handle;        /**connection id*/
    uint16_t       max_packet_length;   /**< The maximum length of a TX/RX packet after a SPP/BLE connection is established. */
	uint8_t        remote_addr[6];
	serial_port_register_callback_t callback; /** serial port user's callback*/
} serial_port_bt_cntx_t;

serial_port_register_callback_t g_serial_port_bt_callback[MAX_BT_PORT_NUM] = {NULL};
serial_port_bt_cntx_t g_serial_port_bt_cntx[MAX_BT_PORT_NUM] = {
	{false, false, 0, 0, {0}, NULL}, 
	{false, false, 0, 0, {0}, NULL}
#ifdef MTK_AIRUPDATE_ENABLE
	, 
	{false, false, 0, 0, {0}, NULL}
#endif
};

static void serial_port_ble_air_event_callback(ble_air_event_t event, void *callback_param);

static void serial_port_ble_air_event_callback(ble_air_event_t event, void *callback_param)
{
    if (NULL != g_serial_port_bt_cntx[BLE_AIR_TYPE].callback) {
		switch (event) {
			case BLE_AIR_EVENT_CONNECT_IND: {
				ble_air_connect_t *conn_ind = (ble_air_connect_t *)callback_param;
				g_serial_port_bt_cntx[BLE_AIR_TYPE].conn_handle = conn_ind->conn_handle; 
				g_serial_port_bt_cntx[BLE_AIR_TYPE].connected = true;
				//memcpy(g_serial_port_bt_cntx[BLE_AIR_TYPE].remote_addr, conn_ind->bdaddr, 6);
				g_serial_port_bt_cntx[BLE_AIR_TYPE].callback(SERIAL_PORT_DEV_BT_LE, SERIAL_PORT_EVENT_BT_CONNECTION, NULL);
			}
			break;

			case BLE_AIR_EVENT_DISCONNECT_IND: {
				ble_air_disconnect_t *disconn_ind = (ble_air_disconnect_t *)callback_param;
				if (disconn_ind->conn_handle == g_serial_port_bt_cntx[BLE_AIR_TYPE].conn_handle) {
					g_serial_port_bt_cntx[BLE_AIR_TYPE].callback(SERIAL_PORT_DEV_BT_LE, SERIAL_PORT_EVENT_BT_DISCONNECTION, NULL);
					g_serial_port_bt_cntx[BLE_AIR_TYPE].connected = false;
					g_serial_port_bt_cntx[BLE_AIR_TYPE].conn_handle = 0;
					//memset(g_serial_port_bt_cntx[BLE_AIR_TYPE].remote_addr, 0x0, 6);
				}
			}
			break;

			case BLE_AIR_EVENT_READY_TO_READ_IND: {
				ble_air_ready_to_read_t *ready_read = (ble_air_ready_to_read_t *)callback_param;
				if (ready_read->conn_handle == g_serial_port_bt_cntx[BLE_AIR_TYPE].conn_handle) {
					g_serial_port_bt_cntx[BLE_AIR_TYPE].callback(SERIAL_PORT_DEV_BT_LE, SERIAL_PORT_EVENT_READY_TO_READ, NULL);	
				}
			}
			break;
			
			case BLE_AIR_EVENT_READY_TO_WRITE_IND: {
				ble_air_ready_to_write_t *ready_write = (ble_air_ready_to_write_t *)callback_param;
				if (ready_write->conn_handle == g_serial_port_bt_cntx[BLE_AIR_TYPE].conn_handle) {
					g_serial_port_bt_cntx[BLE_AIR_TYPE].callback(SERIAL_PORT_DEV_BT_LE, SERIAL_PORT_EVENT_READY_TO_WRITE, NULL);
				}
			}
			break;

			default :
				break;
		}
	}
}

static void serial_port_spp_air_event_callback(spp_air_event_t event_id, void *param);
static void serial_port_spp_air_event_callback(spp_air_event_t event_id, void *param)
{
	if (NULL != g_serial_port_bt_cntx[SPP_AIR_TYPE].callback) {
		switch (event_id) {
			case SPP_AIR_CONNECT_IND: {
				spp_air_connect_ind_t *conn_ind = (spp_air_connect_ind_t *)param;
				g_serial_port_bt_cntx[SPP_AIR_TYPE].conn_handle = conn_ind->handle; 
				g_serial_port_bt_cntx[SPP_AIR_TYPE].connected = true; 
				g_serial_port_bt_cntx[SPP_AIR_TYPE].max_packet_length = conn_ind->max_packet_length; 
				//memcpy(g_serial_port_bt_cntx[SPP_AIR_TYPE].remote_addr, conn_ind->address, 6);
				g_serial_port_bt_cntx[SPP_AIR_TYPE].callback(SERIAL_PORT_DEV_BT_SPP, SERIAL_PORT_EVENT_BT_CONNECTION, NULL);
			}
			break;

			case SPP_AIR_DISCONNECT_IND: {
				spp_air_disconnect_ind_t *disconn_ind = (spp_air_disconnect_ind_t *)param;
				if (disconn_ind->handle == g_serial_port_bt_cntx[SPP_AIR_TYPE].conn_handle) {
					g_serial_port_bt_cntx[SPP_AIR_TYPE].callback(SERIAL_PORT_DEV_BT_SPP, SERIAL_PORT_EVENT_BT_DISCONNECTION, NULL);
					g_serial_port_bt_cntx[SPP_AIR_TYPE].conn_handle = 0;
					g_serial_port_bt_cntx[SPP_AIR_TYPE].connected = false; 
					//memset(g_serial_port_bt_cntx[SPP_AIR_TYPE].remote_addr, 0x0, 6);
				}
			}
			break;

			case SPP_AIR_RECIEVED_DATA_IND: {
				spp_air_data_received_ind_t *ready_read = (spp_air_data_received_ind_t *)param;
				if (ready_read->handle == g_serial_port_bt_cntx[SPP_AIR_TYPE].conn_handle) {
					g_serial_port_bt_cntx[SPP_AIR_TYPE].callback(SERIAL_PORT_DEV_BT_SPP, SERIAL_PORT_EVENT_READY_TO_READ, NULL); 
				}
			}
			break;
			
			case SPP_AIR_READY_TO_SEND_IND: {
				spp_air_ready_to_send_ind_t *ready_write = (spp_air_ready_to_send_ind_t *)param;
				if (ready_write->handle == g_serial_port_bt_cntx[SPP_AIR_TYPE].conn_handle) {
					g_serial_port_bt_cntx[SPP_AIR_TYPE].callback(SERIAL_PORT_DEV_BT_SPP, SERIAL_PORT_EVENT_READY_TO_WRITE, NULL);
				}
			}
			break;

			default :
				break;
		}
	}
}
#ifdef MTK_AIRUPDATE_ENABLE
static void serial_port_airupdate_event_callback(airupdate_event_t event_id, void *param);
static void serial_port_airupdate_event_callback(airupdate_event_t event_id, void *param)
{
	if (NULL != g_serial_port_bt_cntx[AIRUPDATE_TYPE].callback) {
		switch (event_id) {
			case AIRUPDATE_CONNECT_IND: {
				airupdate_connect_ind_t *conn_ind = (airupdate_connect_ind_t *)param;
				g_serial_port_bt_cntx[AIRUPDATE_TYPE].conn_handle = conn_ind->handle; 
				g_serial_port_bt_cntx[AIRUPDATE_TYPE].connected = true; 
				g_serial_port_bt_cntx[AIRUPDATE_TYPE].max_packet_length = conn_ind->max_packet_length; 
				//memcpy(g_serial_port_bt_cntx[SPP_AIR_TYPE].remote_addr, conn_ind->address, 6);
				g_serial_port_bt_cntx[AIRUPDATE_TYPE].callback(SERIAL_PORT_DEV_BT_AIRUPDATE, SERIAL_PORT_EVENT_BT_CONNECTION, NULL);
			}
			break;

			case AIRUPDATE_DISCONNECT_IND: {
				airupdate_disconnect_ind_t *disconn_ind = (airupdate_disconnect_ind_t *)param;
				if (disconn_ind->handle == g_serial_port_bt_cntx[AIRUPDATE_TYPE].conn_handle) {
					g_serial_port_bt_cntx[AIRUPDATE_TYPE].callback(SERIAL_PORT_DEV_BT_AIRUPDATE, SERIAL_PORT_EVENT_BT_DISCONNECTION, NULL);
					g_serial_port_bt_cntx[AIRUPDATE_TYPE].conn_handle = 0;
					g_serial_port_bt_cntx[AIRUPDATE_TYPE].connected = false; 
					//memset(g_serial_port_bt_cntx[SPP_AIR_TYPE].remote_addr, 0x0, 6);
				}
			}
			break;

			case AIRUPDATE_RECIEVED_DATA_IND: {
				airupdate_data_received_ind_t *ready_read = (airupdate_data_received_ind_t *)param;
				if (ready_read->handle == g_serial_port_bt_cntx[AIRUPDATE_TYPE].conn_handle) {
					g_serial_port_bt_cntx[AIRUPDATE_TYPE].callback(SERIAL_PORT_DEV_BT_AIRUPDATE, SERIAL_PORT_EVENT_READY_TO_READ, NULL); 
				}
			}
			break;

			case SPP_AIR_READY_TO_SEND_IND: {
				airupdate_ready_to_send_ind_t *ready_write = (airupdate_ready_to_send_ind_t *)param;
				if (ready_write->handle == g_serial_port_bt_cntx[AIRUPDATE_TYPE].conn_handle) {
					g_serial_port_bt_cntx[AIRUPDATE_TYPE].callback(SERIAL_PORT_DEV_BT_AIRUPDATE, SERIAL_PORT_EVENT_READY_TO_WRITE, NULL);
				}
			}
			break;

			default :
				break;
		}
	}
}
#endif
serial_port_status_t serial_port_bt_init(serial_port_dev_t device, serial_port_open_para_t *para, void *priv_data)
{
	if (SERIAL_PORT_DEV_BT_LE == device) {
		if (false != g_serial_port_bt_cntx[BLE_AIR_TYPE].initialized) {
			return SERIAL_PORT_STATUS_BUSY;
		}
		g_serial_port_bt_cntx[BLE_AIR_TYPE].callback = para->callback;
		g_serial_port_bt_cntx[BLE_AIR_TYPE].initialized = true;
		if (0 != ble_air_init(serial_port_ble_air_event_callback)) {
			memset(&g_serial_port_bt_cntx[BLE_AIR_TYPE], 0x00, sizeof(serial_port_bt_cntx_t));
			g_serial_port_bt_cntx[BLE_AIR_TYPE].callback = NULL;
			return SERIAL_PORT_STATUS_FAIL;
		}
		
	} else if (SERIAL_PORT_DEV_BT_SPP == device) {
	    if (false != g_serial_port_bt_cntx[SPP_AIR_TYPE].initialized) {
			return SERIAL_PORT_STATUS_BUSY;
		}
		g_serial_port_bt_cntx[SPP_AIR_TYPE].callback = para->callback;
		g_serial_port_bt_cntx[SPP_AIR_TYPE].initialized = true;
		if (0 != spp_air_init(serial_port_spp_air_event_callback)) {
			memset(&g_serial_port_bt_cntx[SPP_AIR_TYPE], 0x00, sizeof(serial_port_bt_cntx_t));
			g_serial_port_bt_cntx[SPP_AIR_TYPE].callback = NULL;
			return SERIAL_PORT_STATUS_FAIL;
		}
#ifdef MTK_AIRUPDATE_ENABLE
	} else if (SERIAL_PORT_DEV_BT_AIRUPDATE == device) {
	    if (false != g_serial_port_bt_cntx[AIRUPDATE_TYPE].initialized) {
			return SERIAL_PORT_STATUS_BUSY;
		}
		g_serial_port_bt_cntx[AIRUPDATE_TYPE].callback = para->callback;
		g_serial_port_bt_cntx[AIRUPDATE_TYPE].initialized = true;
		if (0 != airupdate_init(serial_port_airupdate_event_callback)) {
			memset(&g_serial_port_bt_cntx[AIRUPDATE_TYPE], 0x00, sizeof(serial_port_bt_cntx_t));
			g_serial_port_bt_cntx[AIRUPDATE_TYPE].callback = NULL;
			return SERIAL_PORT_STATUS_FAIL;
		}
#endif
	} else {
		return SERIAL_PORT_STATUS_INVALID_DEVICE;
	}
	
    return SERIAL_PORT_STATUS_OK;
}

static serial_port_status_t serial_port_ble_air_control(serial_port_ctrl_cmd_t cmd, serial_port_ctrl_para_t *para)
{
	serial_port_status_t ret = SERIAL_PORT_STATUS_OK;
	serial_port_write_data_t *serial_port_write_data;
	serial_port_read_data_t *serial_port_read_data;
	serial_port_get_read_avail_t *para_uart_get_rx_avail;
	//serial_port_write_data_t *serial_port_write_data_blocking;
	//serial_port_get_write_avail_t *para_uart_get_tx_avail;

	switch (cmd) {
		case SERIAL_PORT_CMD_WRITE_DATA: 
		case SERIAL_PORT_CMD_WRITE_DATA_BLOCKING: {
			serial_port_write_data = (serial_port_write_data_t *)para;
			if ((0 != g_serial_port_bt_cntx[BLE_AIR_TYPE].conn_handle) && 
				(true == g_serial_port_bt_cntx[BLE_AIR_TYPE].connected)) {
				serial_port_write_data->ret_size = ble_air_write_data(g_serial_port_bt_cntx[BLE_AIR_TYPE].conn_handle, serial_port_write_data->data, serial_port_write_data->size);
				log_hal_msgid_info("[BT_PORT] serial_port_ble_air_control: send_size[%d]\r\n",1, serial_port_write_data->ret_size);
				
			} else {
				return SERIAL_PORT_STATUS_DEV_NOT_READY;
			}
			if (serial_port_write_data->ret_size == 0) {
				ret = SERIAL_PORT_STATUS_FAIL;
			}
		}
		break;
		
		case SERIAL_PORT_CMD_READ_DATA:
		case SERIAL_PORT_CMD_READ_DATA_BLOCKING: {
			serial_port_read_data = (serial_port_read_data_t *)para;
			
			if ((0 != g_serial_port_bt_cntx[BLE_AIR_TYPE].conn_handle) && 
				(true == g_serial_port_bt_cntx[BLE_AIR_TYPE].connected)) {
				serial_port_read_data->ret_size = ble_air_read_data(g_serial_port_bt_cntx[BLE_AIR_TYPE].conn_handle, serial_port_read_data->buffer, serial_port_read_data->size);
				
				log_hal_msgid_info("[BT_PORT] serial_port_ble_air_control: read_size[%d]\r\n", 1, serial_port_read_data->ret_size);
			} else {
				return SERIAL_PORT_STATUS_DEV_NOT_READY;
			}

			if (serial_port_read_data->ret_size == 0) {
				ret = SERIAL_PORT_STATUS_FAIL;
			}
		}
		break;

	#if 0
		case SERIAL_PORT_CMD_GET_WRITE_AVAIL: {
			para_uart_get_tx_avail = (serial_port_get_write_avail_t *)para;
			if ((SERIAL_PORT_DEV_BT_LE == dev) && (g_serial_port_bt_cntx[0].conn_handle)) {
				para_uart_get_tx_avail->ret_size = ble_air_get_tx_available(conn_handle);
			}
			if (para_uart_get_tx_avail->ret_size == 0) {
				ret = SERIAL_PORT_STATUS_FAIL;
			}
		}
		break;
	#endif
		
		case SERIAL_PORT_CMD_GET_READ_AVAIL: {
			para_uart_get_rx_avail = (serial_port_get_read_avail_t *)para;
			if ((0 != g_serial_port_bt_cntx[BLE_AIR_TYPE].conn_handle) && 
				(true == g_serial_port_bt_cntx[BLE_AIR_TYPE].connected)) {
				para_uart_get_rx_avail->ret_size = ble_air_get_rx_available(g_serial_port_bt_cntx[BLE_AIR_TYPE].conn_handle);
			}
			if (para_uart_get_rx_avail->ret_size == 0) {
				ret = SERIAL_PORT_STATUS_FAIL;
			}
		}
		break;
		
		default:
//			ASSERT(0);
			ret = SERIAL_PORT_STATUS_UNSUPPORTED;
			break;
	}
	return ret;

}

#ifdef MTK_AIRUPDATE_ENABLE
static serial_port_status_t serial_port_airupdate_control(serial_port_ctrl_cmd_t cmd, serial_port_ctrl_para_t *para)
{
	serial_port_status_t ret = SERIAL_PORT_STATUS_OK;
	serial_port_write_data_t *serial_port_write_data;
	serial_port_read_data_t *serial_port_read_data;
	serial_port_get_read_avail_t *para_uart_get_rx_avail;
	//serial_port_write_data_t *serial_port_write_data_blocking;
	//serial_port_get_write_avail_t *para_uart_get_tx_avail;

	switch (cmd) {
		case SERIAL_PORT_CMD_WRITE_DATA: 
		case SERIAL_PORT_CMD_WRITE_DATA_BLOCKING: {
			serial_port_write_data = (serial_port_write_data_t *)para;
			
			if ((0 != g_serial_port_bt_cntx[AIRUPDATE_TYPE].conn_handle) && 
				(true == g_serial_port_bt_cntx[AIRUPDATE_TYPE].connected)) {
				serial_port_write_data->ret_size = airupdate_write_data(g_serial_port_bt_cntx[AIRUPDATE_TYPE].conn_handle, serial_port_write_data->data, serial_port_write_data->size);
				
			} else {
				return SERIAL_PORT_STATUS_DEV_NOT_READY;
			}
			if (serial_port_write_data->ret_size == 0) {
				ret = SERIAL_PORT_STATUS_FAIL;
			}
		}
		break;
		
		case SERIAL_PORT_CMD_READ_DATA:
		case SERIAL_PORT_CMD_READ_DATA_BLOCKING: {
			serial_port_read_data = (serial_port_read_data_t *)para;
			
			if ((0 != g_serial_port_bt_cntx[AIRUPDATE_TYPE].conn_handle) && 
				(true == g_serial_port_bt_cntx[AIRUPDATE_TYPE].connected)) {
				if (serial_port_read_data->size < g_serial_port_bt_cntx[AIRUPDATE_TYPE].max_packet_length) {
					log_hal_msgid_error("Please enalrge the size od input buffer over %d\r\n", 1,g_serial_port_bt_cntx[AIRUPDATE_TYPE].max_packet_length);
					return SERIAL_PORT_STATUS_INVALID_PARAMETER;
				}
				serial_port_read_data->ret_size = airupdate_read_data(g_serial_port_bt_cntx[AIRUPDATE_TYPE].conn_handle, serial_port_read_data->buffer, serial_port_read_data->size);
				
			} else {
				return SERIAL_PORT_STATUS_DEV_NOT_READY;
			}

			if (serial_port_read_data->ret_size == 0) {
				ret = SERIAL_PORT_STATUS_FAIL;
			}
		}
		break;

#if 0
		case SERIAL_PORT_CMD_GET_WRITE_AVAIL: {
			para_uart_get_tx_avail = (serial_port_get_write_avail_t *)para;
			if ((SERIAL_PORT_DEV_BT_LE == dev) && (g_serial_port_bt_cntx[0].conn_handle)) {
				para_uart_get_tx_avail->ret_size = ble_air_get_tx_available(conn_handle);
			}
			if (para_uart_get_tx_avail->ret_size == 0) {
				ret = SERIAL_PORT_STATUS_FAIL;
			}
		}
		break;
#endif
		
		case SERIAL_PORT_CMD_GET_READ_AVAIL: {
			para_uart_get_rx_avail = (serial_port_get_read_avail_t *)para;
			if ((0 != g_serial_port_bt_cntx[AIRUPDATE_TYPE].conn_handle) && 
				(true == g_serial_port_bt_cntx[AIRUPDATE_TYPE].connected)) {
				para_uart_get_rx_avail->ret_size = airupdate_get_rx_available(g_serial_port_bt_cntx[AIRUPDATE_TYPE].conn_handle);
			}
			if (para_uart_get_rx_avail->ret_size == 0) {
				ret = SERIAL_PORT_STATUS_FAIL;
			}
		}
		break;
		
		default:
//			ASSERT(0);
			ret = SERIAL_PORT_STATUS_UNSUPPORTED;
			break;
	}
	return ret;

}
#endif
static serial_port_status_t serial_port_spp_air_control(serial_port_ctrl_cmd_t cmd, serial_port_ctrl_para_t *para)
{
	serial_port_status_t ret = SERIAL_PORT_STATUS_OK;
	serial_port_write_data_t *serial_port_write_data;
	serial_port_read_data_t *serial_port_read_data;
	serial_port_get_read_avail_t *para_uart_get_rx_avail;
	//serial_port_write_data_t *serial_port_write_data_blocking;
	//serial_port_get_write_avail_t *para_uart_get_tx_avail;

	switch (cmd) {
		case SERIAL_PORT_CMD_WRITE_DATA: 
		case SERIAL_PORT_CMD_WRITE_DATA_BLOCKING: {
			serial_port_write_data = (serial_port_write_data_t *)para;
			
			if ((0 != g_serial_port_bt_cntx[SPP_AIR_TYPE].conn_handle) && 
				(true == g_serial_port_bt_cntx[SPP_AIR_TYPE].connected)) {
				serial_port_write_data->ret_size = spp_air_write_data(g_serial_port_bt_cntx[SPP_AIR_TYPE].conn_handle, serial_port_write_data->data, serial_port_write_data->size);
				
			} else {
				return SERIAL_PORT_STATUS_DEV_NOT_READY;
			}
			if (serial_port_write_data->ret_size == 0) {
				ret = SERIAL_PORT_STATUS_FAIL;
			}
		}
		break;
		
		case SERIAL_PORT_CMD_READ_DATA:
		case SERIAL_PORT_CMD_READ_DATA_BLOCKING: {
			serial_port_read_data = (serial_port_read_data_t *)para;
			
			if ((0 != g_serial_port_bt_cntx[SPP_AIR_TYPE].conn_handle) && 
				(true == g_serial_port_bt_cntx[SPP_AIR_TYPE].connected)) {
				if (serial_port_read_data->size < g_serial_port_bt_cntx[SPP_AIR_TYPE].max_packet_length) {
					log_hal_msgid_info("Please enalrge the size od input buffer over %d\r\n", 1, g_serial_port_bt_cntx[SPP_AIR_TYPE].max_packet_length);
					return SERIAL_PORT_STATUS_INVALID_PARAMETER;
				}
				serial_port_read_data->ret_size = spp_air_read_data(g_serial_port_bt_cntx[SPP_AIR_TYPE].conn_handle, serial_port_read_data->buffer, serial_port_read_data->size);
				
			} else {
				return SERIAL_PORT_STATUS_DEV_NOT_READY;
			}

			if (serial_port_read_data->ret_size == 0) {
				ret = SERIAL_PORT_STATUS_FAIL;
			}
		}
		break;

#if 0
		case SERIAL_PORT_CMD_GET_WRITE_AVAIL: {
			para_uart_get_tx_avail = (serial_port_get_write_avail_t *)para;
			if ((SERIAL_PORT_DEV_BT_LE == dev) && (g_serial_port_bt_cntx[0].conn_handle)) {
				para_uart_get_tx_avail->ret_size = ble_air_get_tx_available(conn_handle);
			}
			if (para_uart_get_tx_avail->ret_size == 0) {
				ret = SERIAL_PORT_STATUS_FAIL;
			}
		}
		break;
#endif
		
		case SERIAL_PORT_CMD_GET_READ_AVAIL: {
			para_uart_get_rx_avail = (serial_port_get_read_avail_t *)para;
			if ((0 != g_serial_port_bt_cntx[SPP_AIR_TYPE].conn_handle) && 
				(true == g_serial_port_bt_cntx[SPP_AIR_TYPE].connected)) {
				para_uart_get_rx_avail->ret_size = spp_air_get_rx_available(g_serial_port_bt_cntx[SPP_AIR_TYPE].conn_handle);
			}
			if (para_uart_get_rx_avail->ret_size == 0) {
				ret = SERIAL_PORT_STATUS_FAIL;
			}
		}
		break;
		
		default:
//			ASSERT(0);
			ret = SERIAL_PORT_STATUS_UNSUPPORTED;
			break;
	}
	return ret;

}


serial_port_status_t serial_port_bt_control(serial_port_dev_t dev, serial_port_ctrl_cmd_t cmd, serial_port_ctrl_para_t *para)
{
	if (SERIAL_PORT_DEV_BT_LE == dev) {
		return serial_port_ble_air_control(cmd, para);
	} else if (SERIAL_PORT_DEV_BT_SPP == dev) {
		return serial_port_spp_air_control(cmd, para);
#ifdef MTK_AIRUPDATE_ENABLE
	} else if (SERIAL_PORT_DEV_BT_AIRUPDATE == dev) {
		return serial_port_airupdate_control(cmd, para);
#endif
	} else {
		return SERIAL_PORT_STATUS_INVALID_DEVICE;

	}
}

/* Close BT port */
serial_port_status_t serial_port_bt_deinit(serial_port_dev_t port)
{
	if (SERIAL_PORT_DEV_BT_LE == port) {
		if (false != g_serial_port_bt_cntx[BLE_AIR_TYPE].initialized) {
			if (0 != ble_air_deinit(serial_port_ble_air_event_callback)) {
				log_hal_msgid_error("serial_port_bt_deinit, deinit ble air fail\r\n", 0);
			}
			memset(&g_serial_port_bt_cntx[BLE_AIR_TYPE], 0x00, sizeof(serial_port_bt_cntx_t));
			g_serial_port_bt_cntx[BLE_AIR_TYPE].callback = NULL;
		}
	} else if (SERIAL_PORT_DEV_BT_SPP == port) {
		if (false != g_serial_port_bt_cntx[SPP_AIR_TYPE].initialized) {
			if (0 != spp_air_deinit(serial_port_spp_air_event_callback)) {
				log_hal_msgid_error("serial_port_bt_deinit, deinit spp air fail\r\n", 0);
			}
			memset(&g_serial_port_bt_cntx[SPP_AIR_TYPE], 0x00, sizeof(serial_port_bt_cntx_t));
			g_serial_port_bt_cntx[SPP_AIR_TYPE].callback = NULL;
		}
#ifdef MTK_AIRUPDATE_ENABLE
	} else if (SERIAL_PORT_DEV_BT_AIRUPDATE == port) {
		if (false != g_serial_port_bt_cntx[AIRUPDATE_TYPE].initialized) {
			if (0 != airupdate_deinit(serial_port_airupdate_event_callback)) {
				log_hal_msgid_error("serial_port_bt_deinit, deinit Airupdate fail\r\n", 0);
			}
			memset(&g_serial_port_bt_cntx[AIRUPDATE_TYPE], 0x00, sizeof(serial_port_bt_cntx_t));
			g_serial_port_bt_cntx[AIRUPDATE_TYPE].callback = NULL;
		}
#endif
	} else {
		return SERIAL_PORT_STATUS_INVALID_DEVICE;
	}

    return SERIAL_PORT_STATUS_OK;
}

#else

serial_port_status_t serial_port_bt_init(serial_port_dev_t device, serial_port_open_para_t *para, void *priv_data)
{
    return SERIAL_PORT_STATUS_UNSUPPORTED;
}

serial_port_status_t serial_port_bt_control(serial_port_dev_t dev, serial_port_ctrl_cmd_t cmd, serial_port_ctrl_para_t *para)
{
    return SERIAL_PORT_STATUS_UNSUPPORTED;
}

serial_port_status_t serial_port_bt_deinit(serial_port_dev_t port)
{
    return SERIAL_PORT_STATUS_UNSUPPORTED;
}

#endif/*MTK_PORT_SERVICE_BT_ENABLE*/


#endif




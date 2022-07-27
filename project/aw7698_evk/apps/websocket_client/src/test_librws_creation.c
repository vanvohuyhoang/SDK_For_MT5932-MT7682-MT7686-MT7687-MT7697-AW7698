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


#include <stdlib.h>
#include <assert.h>
#include <string.h>

#if defined(CMAKE_BUILD)
#undef CMAKE_BUILD
#endif


#if defined(XCODE)
#include "librws.h"
#else
#include <librws.h>
#endif

#include "rws_memory.h"

#include "test_librws_creation.h"

#if defined(CMAKE_BUILD)
#undef CMAKE_BUILD
#endif


#define WEBSOCKET_CONNECTED (0x01)
#define WEBSOCKET_DISCONNECTED (0x02)
#define WEBSOCKET_DATA_NOT_RECVED (0x04)

#ifdef MTK_WEBSOCKET_SSL_ENABLE
#define ECHO_WEBSOCKET_CER												\
"-----BEGIN CERTIFICATE-----\r\n"   \
"MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/\r\n"  \
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\r\n"  \
"DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow\r\n"  \
"PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD\r\n"  \
"Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\r\n"  \
"AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O\r\n"  \
"rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq\r\n"  \
"OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b\r\n"  \
"xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw\r\n"  \
"7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD\r\n"  \
"aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV\r\n"  \
"HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG\r\n"  \
"SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69\r\n"  \
"ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr\r\n"  \
"AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz\r\n"  \
"R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5\r\n"  \
"JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo\r\n"  \
"Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ\r\n"  \
"-----END CERTIFICATE-----\r\n"

static const char echo_websocket_cer[] = ECHO_WEBSOCKET_CER;
#endif

static rws_socket _socket = NULL;
static int state_flags = 0;

static void on_socket_received_text(rws_socket socket, const char *text, const unsigned int length)
{
	char *buff = NULL;

    if (!socket || !text || !length)
    {
        LOG_I(websocket_example, "%s: Invalid parameter(s).", __FUNCTION__);
        return;
    }

    buff = (char *)rws_malloc(length + 1);
    if (!buff)
    {
        LOG_I(websocket_example, "%s: Not enough memory. len:%d", __FUNCTION__, length + 1);
        return;
    }

    state_flags &= (~WEBSOCKET_DATA_NOT_RECVED);

	memcpy(buff, text, length);
	buff[length] = 0;

	LOG_I(websocket_example, "%s: Wsoc Data received: %s", __FUNCTION__, buff);
    rws_free(buff);
    buff = NULL;
}

static void on_socket_received_bin(rws_socket socket, const void * data, const unsigned int length)
{
	char *buff = NULL;

    if (!socket || !data || !length)
    {
        LOG_I(websocket_example, "%s: Invalid parameter(s).", __FUNCTION__);
        return;
    }

    buff = (char *)rws_malloc(length + 1);
    if (!buff)
    {
        LOG_I(websocket_example, "%s: Not enough memory. len:%d", __FUNCTION__, length + 1);
        return;
    }

    state_flags &= ~WEBSOCKET_DATA_NOT_RECVED;

	memcpy(buff, data, length);
	buff[length] = 0;

	LOG_I(websocket_example, "%s: Wsoc Data received: %s", __FUNCTION__, buff);
    rws_free(buff);
    buff = NULL;
}

static void on_socket_connected(rws_socket socket)
{
	const char * test_send_text =
	"{\"version\":\"1.0\",\"supportedConnectionTypes\":[\"websocket\"],\"minimumVersion\":\"1.0\",\"channel\":\"/meta/handshake\"}";

	LOG_I(websocket_example, "%s: Wsoc Connected", __FUNCTION__);

    state_flags |= WEBSOCKET_CONNECTED;
    state_flags &= ~WEBSOCKET_DISCONNECTED;
	rws_socket_send_text(socket, test_send_text);
}

static void on_socket_disconnected(rws_socket socket)
{
	rws_error error = rws_socket_get_error(socket);
	if (error)
	{
        LOG_I(websocket_example, "%s: Socket disconnect with code, error: %i, %s",
              __FUNCTION__,
			  rws_error_get_code(error),
			  rws_error_get_description(error));
	}

    LOG_I(websocket_example, "%s: Wsoc Disconnected", __FUNCTION__);

    state_flags &= ~WEBSOCKET_CONNECTED;
    state_flags |= WEBSOCKET_DISCONNECTED;
    _socket = NULL;
}


int websoc_cli_test_int(const char *scheme, const char *host,
                            const char *path, const int port,
                            const char *cert)
{
    int sleep_count = 0;

    if (!scheme || !host || !path)
    {
        LOG_I(websocket_example, "%s: Invalid parameter(s).", __FUNCTION__);
        return -1;
    }

    if (_socket)
    {
        LOG_I(websocket_example, "%s: Socket is not closed.", __FUNCTION__);
        return -2;
    }

	_socket = rws_socket_create(); // create and store socket handle
	assert(_socket);

    state_flags = 0;
    state_flags |= WEBSOCKET_DATA_NOT_RECVED;

	rws_socket_set_scheme(_socket, scheme);
	rws_socket_set_host(_socket, host);
	rws_socket_set_path(_socket, path);
	rws_socket_set_port(_socket, port);

#ifdef MTK_WEBSOCKET_SSL_ENABLE
    if (cert)
    {
        rws_socket_set_server_cert(_socket, cert, strlen(cert) + 1);
    }
#endif

	rws_socket_set_on_disconnected(_socket, &on_socket_disconnected);
	rws_socket_set_on_connected(_socket, &on_socket_connected);
	rws_socket_set_on_received_text(_socket, &on_socket_received_text);
	rws_socket_set_on_received_bin(_socket, &on_socket_received_bin);

#if !defined(RWS_APPVEYOR_CI)
	// Connection denied for client applications
	rws_socket_connect(_socket);
#endif

    /* Connecting */
    while ((!(state_flags & WEBSOCKET_CONNECTED)) &&
           (!(state_flags & WEBSOCKET_DISCONNECTED)))
    {
        LOG_I(websocket_example, "wait for connection. loop time:%d.", sleep_count);
        rws_thread_sleep(1000);
        sleep_count++;
        if (50 == sleep_count)
        {
            break;
        }
    }

    /* Receiving data */
    sleep_count = 0;
    if (state_flags & WEBSOCKET_CONNECTED)
    {
        while ((state_flags & WEBSOCKET_DATA_NOT_RECVED) && _socket &&
               (state_flags & WEBSOCKET_CONNECTED))
        {
            LOG_I(websocket_example, "wait for data. loop time:%d.", sleep_count);
            rws_thread_sleep(1000);
            sleep_count++;
            if (20 == sleep_count)
            {
                break;
            }
        }
    }

    if (_socket)
    {
        rws_socket_disconnect_and_release(_socket);
    }

    _socket = NULL;

    LOG_I(websocket_example, "state_flags:%x.", state_flags);
	return state_flags & WEBSOCKET_DATA_NOT_RECVED ? -3 : 0;
}


void websoc_cli_test_entry(void)
{
    int ret = 0, ssl_ret = 0;

    LOG_I(websocket_example, "Test client ws.");
    ret = websoc_cli_test_int("ws", "echo.websocket.org", "/", 80, NULL);

#ifdef MTK_WEBSOCKET_SSL_ENABLE
    LOG_I(websocket_example, "Test client wss.");
    ssl_ret = websoc_cli_test_int("wss", "echo.websocket.org", "/", 443, echo_websocket_cer);
#endif

    LOG_I(websocket_example, "Test set get.");
    websoc_set_get_test();

    if (0 == ret && 0 == ssl_ret)
    {
        LOG_I(websocket_example, "example project test success.");
    }
    else
    {
        LOG_I(websocket_example, "example project test failed. ret:%d, ssl_ret:%d", ret, ssl_ret);
    }
}


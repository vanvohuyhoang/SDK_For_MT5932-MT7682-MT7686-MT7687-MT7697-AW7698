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
 
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "syslog.h"

#include "httpclient.h"
#include "timers.h"

#define BUF_SIZE        (1024 * 1)
#define HTTP_GET_URL    "http://www.aliyun.com/"
#define HTTPS_GET_URL   "https://www.baidu.com/"
#define HTTPS_MTK_CLOUD_POST_URL    "https://api.mediatek.com/mcs/v2/devices/D0n2yhrl/datapoints.csv"
#define HTTPS_MTK_CLOUD_HEADER      "deviceKey:FZoo0S07CpwUHcrt\r\n"
#define HTTPS_POST_TIME_TICK        (5000/portTICK_PERIOD_MS)

#define BAIDUIE_CER														\
"-----BEGIN CERTIFICATE-----\r\n"                                       \
"MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\r\n"  \
"A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\r\n"  \
"b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\r\n"  \
"MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\r\n"  \
"YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\r\n"  \
"aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\r\n"  \
"jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\r\n"  \
"xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\r\n"  \
"1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\r\n"  \
"snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\r\n"  \
"U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\r\n"  \
"9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\r\n"  \
"BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\r\n"  \
"AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\r\n"  \
"yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\r\n"  \
"38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\r\n"  \
"AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\r\n"  \
"DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\r\n"  \
"HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\r\n"                              \
"-----END CERTIFICATE-----\r\n"

static httpclient_t client = {0};
static int post_count = 5;
static const char baiduie_cer[] = BAIDUIE_CER;
log_create_module(http_client_retrieve_example, PRINT_LEVEL_INFO);
log_create_module(http_client_keepalive_example, PRINT_LEVEL_INFO);
log_create_module(http_client_get_example, PRINT_LEVEL_INFO);

/**
 * @brief      Http client "get" method working flow.
 * @param      None
 * @return     None
 */
HTTPCLIENT_RESULT httpclient_test_get(void)
{
    char *get_url = HTTP_GET_URL;
    char *post_url = HTTPS_GET_URL;
    httpclient_t client = {0};
    httpclient_data_t client_data = {0};
    char *buf, *header;
    HTTPCLIENT_RESULT ret = 0;
    int val_pos, val_len;

    LOG_I(http_client_get_example, "httpclient_test_get()");

    buf = pvPortMalloc(BUF_SIZE);
    header = pvPortMalloc(BUF_SIZE);
    if (buf == NULL || header == NULL) {
        LOG_I(http_client_get_example, "memory malloc failed.");
        return ret;
    }

    // Http "get"
    client_data.header_buf = header;
    client_data.header_buf_len = BUF_SIZE;
    client_data.response_buf = buf;
    client_data.response_buf_len = BUF_SIZE;
    client_data.response_buf[0] = '\0';
    ret = httpclient_get(&client, get_url, &client_data);
    if (ret < 0)
        goto fail; LOG_I(http_client_get_example, "data received: %s", client_data.response_buf);

    // get response header
    if (0
            == httpclient_get_response_header_value(client_data.header_buf, "Content-length",
                    &val_pos, &val_len))
        LOG_I(http_client_get_example, "Content-length: %.*s", val_len, client_data.header_buf + val_pos);

    // Https "get"
    client_data.header_buf = header;
    client_data.header_buf_len = BUF_SIZE;
    client_data.response_buf = buf;
    client_data.response_buf_len = BUF_SIZE;
    client_data.response_buf[0] = '\0';

#ifdef MTK_HTTPCLIENT_SSL_ENABLE
    client.server_cert = baiduie_cer;
    client.server_cert_len = sizeof(baiduie_cer);
#endif

    ret = httpclient_get(&client, post_url, &client_data);
    if (ret < 0)
        goto fail; LOG_I(http_client_get_example, "data received: %s", client_data.response_buf);

    // get response header
    if (0
            == httpclient_get_response_header_value(client_data.header_buf, "Content-length",
                    &val_pos, &val_len))
        LOG_I(http_client_get_example, "Content-length: %.*s", val_len, client_data.header_buf + val_pos);

    fail: vPortFree(buf);
    vPortFree(header);

    // Print final log
    if (ret >= 0)
        LOG_I(http_client_get_example, "http_client get test success.");
    else
        LOG_I(http_client_get_example, "http_client get fail, reason:%d.", ret);

    return ret;
}

static HTTPCLIENT_RESULT httpclient_test_keepalive_post(void)
{
    HTTPCLIENT_RESULT ret = HTTPCLIENT_ERROR_CONN;
    char *post_url = HTTPS_MTK_CLOUD_POST_URL;
    char *header = HTTPS_MTK_CLOUD_HEADER;
    httpclient_data_t client_data = {0};
    char *buf;
    char *content_type = "text/csv";
    char post_data[32];

    buf = pvPortMalloc(BUF_SIZE);
    if (buf == NULL) {
        LOG_I(http_client_keepalive_example, "memory malloc failed.");
        return ret;
    }

    client_data.response_buf = buf;
    client_data.response_buf_len = BUF_SIZE;
    client_data.post_content_type = content_type;
    sprintf(post_data, "1,,temperature:%d", (10 + post_count));
    client_data.post_buf = post_data;
    client_data.post_buf_len = strlen(post_data);
    httpclient_set_custom_header(&client, header);

    ret = httpclient_send_request(&client, post_url, HTTPCLIENT_POST, &client_data);
    if (ret < 0)
        goto fail;

    ret = httpclient_recv_response(&client, &client_data);
    if (ret < 0)
        goto fail;

    fail: vPortFree(buf);
    return ret;
}

/**
 * @brief      Send "keepalive" package by "post" method in every HTTP_POST_TIME_TICK.
 * @param      None
 * @return     None
 */
int g_test_keepalive_result = -1;
void httpclient_test_keepalive_timeout_handle(TimerHandle_t tmr)
{
    HTTPCLIENT_RESULT ret = HTTPCLIENT_OK;

    if (post_count--) {
        ret = httpclient_test_keepalive_post();
        if (ret >= 0)
            return; // continue
    }

    // Stop and delete timer
    xTimerStop(tmr, 0);
    xTimerDelete(tmr, 0);

    // Close http connection
    httpclient_close(&client);

    // Print final log
    if (ret >= 0) {
        LOG_I(http_client_keepalive_example, "http_client keepalive test success.");
        g_test_keepalive_result = 1;
    } else {
        LOG_I(http_client_keepalive_example, "keepalive example project test fail, reason:%d.", ret);
        g_test_keepalive_result = 0;
    }

    return;
}

/**
 * @brief      Http client connect to test server and test "keepalive" function.
 * @param      None
 * @return     None
 */
HTTPCLIENT_RESULT httpclient_test_keepalive(void)
{
    HTTPCLIENT_RESULT ret = HTTPCLIENT_ERROR_CONN;
    char *post_url = HTTPS_MTK_CLOUD_POST_URL;
    TimerHandle_t tmr;

    // Connect to server.
    ret = httpclient_connect(&client, post_url);
    if (ret < 0)
        goto fail;

    // Send "keepalive" package by "post" method.
    ret = httpclient_test_keepalive_post();
    if (ret < 0)
        goto fail;

    // Create and start timer 
    tmr = xTimerCreate("https_post_timer", HTTPS_POST_TIME_TICK, pdTRUE, NULL,
            httpclient_test_keepalive_timeout_handle);
    xTimerStart(tmr, 0);
    return ret;

    fail:
    // Close http connection
    httpclient_close(&client);

    // Print fail log
    LOG_I(http_client_keepalive_example, "keepalive test fail, reason:%d.", ret);

    return ret;
}

/**
 * @brief      Http client connect to test server and test "retrieve" function.
 * @param      None
 * @return     None
 */
HTTPCLIENT_RESULT httpclient_test_retrieve(void)
{
    char *get_url = HTTP_GET_URL;
    HTTPCLIENT_RESULT ret = 0;
    httpclient_t client = {0};
    char *buf;
    httpclient_data_t client_data = {0};
    int count = 0;

    buf = pvPortMalloc(BUF_SIZE);
    if (buf == NULL) {
        LOG_I(http_client_retrieve_example, "memory malloc failed.");
        return ret;
    }

    // Connect to server
    ret = httpclient_connect(&client, get_url);

    if (!ret) {
        client_data.response_buf = buf;
        client_data.response_buf_len = BUF_SIZE;

        // Send request to server
        ret = httpclient_send_request(&client, get_url, HTTPCLIENT_GET, &client_data);
        if (ret < 0)
            goto fail;

        do {
            // Receive response from server
            ret = httpclient_recv_response(&client, &client_data);
            if (ret < 0)
                goto fail;
            count += strlen(client_data.response_buf);
            LOG_I(http_client_retrieve_example, "data received: %s", client_data.response_buf);
        } while (ret == HTTPCLIENT_RETRIEVE_MORE_DATA);

        LOG_I(http_client_retrieve_example, "total length: %d", client_data.response_content_len);
    }

    fail:
    // Close http connection
    httpclient_close(&client);
    vPortFree(buf);

    // Print final log
    if (ret >= 0)
        LOG_I(http_client_retrieve_example, "http_client retrieve test success.");
    else
        LOG_I(http_client_retrieve_example, "retrieve example project test fail, reason:%d.", ret);

    return ret;
}

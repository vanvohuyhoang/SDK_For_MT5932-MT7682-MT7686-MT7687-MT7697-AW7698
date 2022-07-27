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


#ifndef __SECURITY_INTERFACE_H__
#define __SECURITY_INTERFACE_H__

#define IN
#define OUT
#define INOUT

typedef void  (*HMAC_MD5_CB)(
    unsigned char Key[],
    unsigned int KeyLen,
    unsigned char Message[],
    unsigned int MessageLen,
    unsigned char MAC[],
    unsigned int MACLen);

typedef void  (* HMAC_SHA1_CB) (
    unsigned char Key[],
    unsigned int KeyLen,
    unsigned char Message[],
    unsigned int MessageLen,
    unsigned char MAC[],
    unsigned int MACLen);

typedef int  (*AES_KEY_WRAP_CB)(
    unsigned char    *PlainText,
    unsigned int     PlainTextLen,
    unsigned char    *Key,
    unsigned int      KeyLen,
    unsigned char   *CipherText,
    unsigned int    *CipherTextLen);

typedef int	 (*AES_KEY_UNWRAP_CB)(
    unsigned char CipherText[],
    unsigned int  CipherTextLength,
    unsigned char Key[],
    unsigned int  KeyLength,
    unsigned char PlainText[],
    unsigned int *PlainTextLength);

typedef int   (* AES_CMAC_CB) (
    unsigned char  PlainText[],
    unsigned int  PlainTextLength,
    unsigned char  Key[],
    unsigned int KeyLength,
    unsigned char  MACText[],
    unsigned int *MACTextLength);
    
typedef struct _SECURITY_INTERFACE
{
    HMAC_MD5_CB   pfHmacMd5;
    HMAC_SHA1_CB  pfHmacSha1;
    AES_KEY_WRAP_CB pfAesKeyWrap;
    AES_KEY_UNWRAP_CB pfAesKeyUnwrap;
    AES_CMAC_CB   pfAesCmac;
}SECURITY_INTERFACE_T;

void RegisterSecurityInterface(SECURITY_INTERFACE_T * ptInterface);

int  CalculatePmk(char *password, unsigned char *ssid, int ssidlength, unsigned char *output) ;


void RT_AES_Decrypt (
    unsigned char *CipherBlock,
    unsigned int CipherBlockSize,
    unsigned char *Key,
    unsigned int KeyLength,
    unsigned char *PlainBlock,
    unsigned int *PlainBlockSize);


extern void  RT_HMAC_SHA1 (
    unsigned char Key[],
    unsigned int KeyLen,
    unsigned char Message[],
    unsigned int MessageLen,
    unsigned char MAC[],
    unsigned int MACLen);

extern void  RT_HMAC_MD5(
    IN  unsigned char Key[],
    IN  unsigned int KeyLen,
    IN  unsigned char Message[],
    IN  unsigned int MessageLen,
    OUT unsigned char MAC[],
    IN  unsigned int MACLen);

extern int  AES_Key_Wrap(
    IN unsigned char    *plaintext,
    IN unsigned int     p_len,
    IN unsigned char    *key,
    IN unsigned int      key_len,
    OUT unsigned char   *ciphertext,
    OUT unsigned int    *c_len);

extern int  AES_Key_Unwrap(
    IN unsigned char CipherText[],
    IN unsigned int  CipherTextLength,
    IN unsigned char Key[],
    IN unsigned int  KeyLength,
    OUT unsigned char PlainText[],
    OUT unsigned int *PlainTextLength);

extern void  WIFI_HAL_HMAC_SHA1 (
    unsigned char Key[],
    unsigned int KeyLen,
    unsigned char Message[],
    unsigned int MessageLen,
    unsigned char MAC[],
    unsigned int MACLen);

extern void  WIFI_HAL_HMAC_MD5(
    unsigned char Key[],
    unsigned int KeyLen,
    unsigned char Message[],
    unsigned int MessageLen,
    unsigned char MAC[],
    unsigned int MACLen);

extern int  WIFI_HAL_AES_Key_Wrap(
    unsigned char    *plaintext,
    unsigned int     p_len,
    unsigned char    *key,
    unsigned int      key_len,
    unsigned char   *ciphertext,
    unsigned int     *c_len);

extern int	 WIFI_HAL_AES_Key_Unwrap(
    unsigned char CipherText[],
    unsigned int  CipherTextLength,
    unsigned char Key[],
    unsigned int  KeyLength,
    unsigned char PlainText[],
    unsigned int *PlainTextLength);

extern int   WIFI_HAL_AES_CMAC (
    unsigned char PlainText[],
    unsigned int PlainTextLength,
    unsigned char Key[],
    unsigned int KeyLength,
    unsigned char MACText[],
    unsigned int *MACTextLength);

#endif /* __SECURITY_INTERFACE_H__ */

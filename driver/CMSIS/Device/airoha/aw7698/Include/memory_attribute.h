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
 
/*****************************************************
  Usage:
   1. function: (a or b)
      a. ATTR_TEXT_IN_TCM int func(int par);
      b. ATTR_TEXT_IN_TCM int func(int par)
        {
        }
   2. RO data:
      a. ATTR_RODATA_IN_TCM const int b = 8;
   3. RW data:
      a. ATTR_RWDATA_IN_TCM int b = 8;
   4. ZI data:
      a. ATTR_ZIDATA_IN_TCM int b;
      
  Note: must put these attributes at HEAD of declaration.
*****************************************************/

#ifndef MEMORY_ATTRIBUTE_H_
#define MEMORY_ATTRIBUTE_H_

#if defined(__GNUC__)  /*GCC VERSION*/

#define ATTR_TEXT_IN_TCM              __attribute__ ((__section__(".tcm_code")))
#define ATTR_RODATA_IN_TCM            __attribute__ ((__section__(".tcm_rodata")))
#define ATTR_RWDATA_IN_TCM            __attribute__ ((__section__(".tcm_rwdata")))
#define ATTR_ZIDATA_IN_TCM            __attribute__ ((__section__(".tcm_zidata")))


#define ATTR_TEXT_IN_RAM              __attribute__ ((__section__(".ram_code")))
#define ATTR_RODATA_IN_RAM            __attribute__ ((__section__(".ram_rodata")))
#define ATTR_RWDATA_IN_NONCACHED_RAM  __attribute__ ((__section__(".noncached_ram_rwdata")))
#define ATTR_ZIDATA_IN_NONCACHED_RAM  __attribute__ ((__section__(".noncached_ram_zidata")))

#define ATTR_RWDATA_IN_NONCACHED_RAM_4BYTE_ALIGN  __attribute__ ((__section__(".noncached_ram_rwdata"),__aligned__(4)))
#define ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN  __attribute__ ((__section__(".noncached_ram_zidata"),__aligned__(4)))

#define ATTR_TEXT_IN_SYSRAM              __attribute__ ((__section__(".sysram_code")))
#define ATTR_RODATA_IN_SYSRAM            __attribute__ ((__section__(".sysram_rodata")))

#define ATTR_RWDATA_IN_CACHED_SYSRAM     __attribute__ ((__section__(".cached_sysram_rwdata")))
#define ATTR_ZIDATA_IN_CACHED_SYSRAM     __attribute__ ((__section__(".cached_sysram_zidata")))

#define ATTR_RWDATA_IN_NONCACHED_SYSRAM  __attribute__ ((__section__(".noncached_sysram_rwdata")))
#define ATTR_ZIDATA_IN_NONCACHED_SYSRAM  __attribute__ ((__section__(".noncached_sysram_zidata")))

#define ATTR_RWDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN  __attribute__ ((__section__(".noncached_sysram_rwdata"),__aligned__(4)))
#define ATTR_ZIDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN  __attribute__ ((__section__(".noncached_sysram_zidata"),__aligned__(4)))

#define ATTR_PACKED __attribute__ ((__packed__))
#define ATTR_4BYTE_ALIGN __attribute__ ((__aligned__(4)))


#elif defined(__CC_ARM)  /*KEIL VERSION*/

#define ATTR_TEXT_IN_TCM              __attribute__ ((__section__("tcm_code")))
#define ATTR_RODATA_IN_TCM            __attribute__ ((__section__("tcm_rodata")))
#define ATTR_RWDATA_IN_TCM            __attribute__ ((__section__("tcm_rwdata")))
#define ATTR_ZIDATA_IN_TCM            __attribute__ ((__section__("tcm_zidata")))


#define ATTR_TEXT_IN_RAM              __attribute__ ((__section__("ram_code")))
#define ATTR_RODATA_IN_RAM            __attribute__ ((__section__("ram_rodata")))
#define ATTR_RWDATA_IN_NONCACHED_RAM  __attribute__ ((__section__("noncached_ram_rwdata")))
#define ATTR_ZIDATA_IN_NONCACHED_RAM  __attribute__ ((__section__("noncached_ram_zidata")))

#define ATTR_RWDATA_IN_NONCACHED_RAM_4BYTE_ALIGN  __attribute__ ((__section__("noncached_ram_rwdata"),__aligned__(4)))
#define ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN  __attribute__ ((__section__("noncached_ram_zidata"),__aligned__(4)))

#define ATTR_TEXT_IN_SYSRAM              __attribute__ ((__section__("sysram_code")))
#define ATTR_RODATA_IN_SYSRAM            __attribute__ ((__section__("sysram_rodata")))

#define ATTR_RWDATA_IN_CACHED_SYSRAM     __attribute__ ((__section__("cached_sysram_rwdata")))
#define ATTR_ZIDATA_IN_CACHED_SYSRAM     __attribute__ ((__section__("cached_sysram_zidata")))

#define ATTR_RWDATA_IN_NONCACHED_SYSRAM  __attribute__ ((__section__("noncached_sysram_rwdata")))
#define ATTR_ZIDATA_IN_NONCACHED_SYSRAM  __attribute__ ((__section__("noncached_sysram_zidata")))

#define ATTR_RWDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN  __attribute__ ((__section__("noncached_sysram_rwdata"),__aligned__(4)))
#define ATTR_ZIDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN  __attribute__ ((__section__("noncached_sysram_zidata"),__aligned__(4)))


#define ATTR_PACKED __attribute__ ((__packed__))
#define ATTR_4BYTE_ALIGN __attribute__ ((__aligned__(4)))

#elif defined(__ICCARM__) /*IAR*/

#define ATTR_TEXT_IN_TCM              _Pragma("location=\".tcm_code\"")
#define ATTR_RODATA_IN_TCM            _Pragma("location=\".tcm_rodata\"")
#define ATTR_RWDATA_IN_TCM            _Pragma("location=\".tcm_rwdata\"")
#define ATTR_ZIDATA_IN_TCM            _Pragma("location=\".tcm_zidata\"")


#define ATTR_TEXT_IN_RAM              _Pragma("location=\".ram_code\"")
#define ATTR_RODATA_IN_RAM            _Pragma("location=\".ram_rodata\"")
#define ATTR_RWDATA_IN_NONCACHED_RAM  _Pragma("location=\".noncached_ram_rwdata\"")
#define ATTR_ZIDATA_IN_NONCACHED_RAM  _Pragma("location=\".noncached_ram_zidata\"")

#define ATTR_RWDATA_IN_NONCACHED_RAM_4BYTE_ALIGN  _Pragma("location=\".noncached_ram_rwdata\"") \
                                                  _Pragma("data_alignment=4")
#define ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN  _Pragma("location=\".noncached_ram_zidata\"") \
                                                  _Pragma("data_alignment=4")

#define ATTR_TEXT_IN_SYSRAM              _Pragma("location=\".sysram_code\"")
#define ATTR_RODATA_IN_SYSRAM            _Pragma("location=\".sysram_rodata\"")

#define ATTR_RWDATA_IN_CACHED_SYSRAM      _Pragma("location=\".cached_sysram_rwdata\"")
#define ATTR_ZIDATA_IN_CACHED_SYSRAM     _Pragma("location=\".cached_sysram_zidata\"")

#define ATTR_RWDATA_IN_NONCACHED_SYSRAM  _Pragma("location=\".noncached_sysram_rwdata\"")
#define ATTR_ZIDATA_IN_NONCACHED_SYSRAM  _Pragma("location=\".noncached_sysram_zidata\"")

#define ATTR_RWDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN  _Pragma("location=\".noncached_sysram_rwdata\"") \
                                                _Pragma("data_alignment=4")
#define ATTR_ZIDATA_IN_NONCACHED_SYSRAM_4BYTE_ALIGN  _Pragma("location=\".noncached_sysram_zidata\"") \
                                                  _Pragma("data_alignment=4")

#define ATTR_PACKED __packed
#define ATTR_4BYTE_ALIGN _Pragma("data_alignment=4")

#endif
#endif



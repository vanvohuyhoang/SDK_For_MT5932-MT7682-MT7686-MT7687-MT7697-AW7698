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

/* Includes -----------------------------------------------------------------*/
#include "hal_resource_assignment.h"
#include "hal_uart.h"
#include "hal_ccni.h"
#include "hal_ccni_config.h"
#include "memory_attribute.h"
#include "exception_handler.h"
#include "exception_handler_n9.h"

#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
#include "offline_dump.h"
#endif /* EXCEPTION_MEMDUMP_MODE */


/* Private define -----------------------------------------------------------*/
#define EXCEPTION_EVENT_TO_N9                           IRQGEN_CM4_TO_N9_EXCEPTION
#define EXCEPTION_EVENT_FROM_N9                         IRQGEN_N9_TO_CM4_EXCEPTION

#define N9_REGION_TOTAL                                 2
#define N9_DLMRAM_START                                 0x40100000
#define N9_DLMRAM_END                                   0x40110000
#define N9_SYSRAM_START                                 0x40400000
#define N9_SYSRAM_END                                   0x40406000


/* Private typedef ----------------------------------------------------------*/
#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
typedef struct
{
    uint32_t address;
    uint32_t size;
}exception_minidump_region_n9_t;

typedef struct
{
    uint32_t length;
    char name[20];
    exception_minidump_region_n9_t regions[8];
    uint32_t data_checksum;
}exception_minidump_header_n9_t;
#endif /* EXCEPTION_MEMDUMP_MODE */


/* Private macro ------------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
const memory_region_type n9_memory_regions[N9_REGION_TOTAL] =
{
    {"DLMRAM", (unsigned int *)N9_DLMRAM_START, (unsigned int *)N9_DLMRAM_END, 1},
    {"SYSRAM", (unsigned int *)N9_SYSRAM_START, (unsigned int *)N9_SYSRAM_END, 1},
};

#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
uint32_t minidump_base_address_n9 = 0;
exception_minidump_header_n9_t minidump_header_n9 = {0};
#endif /* EXCEPTION_MEMDUMP_MODE */


/* Private functions --------------------------------------------------------*/
/******************************************************************************/
/*            Exception's memory dump Functions                               */
/******************************************************************************/
#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_TEXT)
static void exception_n9_dump_region_info(const memory_region_type *regions)
{
    uint32_t i = 0;
    unsigned int *current = 0;
    unsigned int *end = 0;

    for(i=0; i < N9_REGION_TOTAL; i++)
    {
        if(regions[i].is_dumped)
        {
            current = (unsigned int *)((uint32_t)(regions[i].start_address) & 0xfffffffc);
            end     = (unsigned int *)(regions[i].end_address);
            exception_printf("Region-%s: start_address = 0x%x, end_address = 0x%x\r\n",
                             regions[i].region_name,
                             (uint32_t)current,
                             (uint32_t)end);
        }
    }
}

static void exception_n9_dump_region_data_text(const memory_region_type *regions)
{
    uint32_t i = 0;
    unsigned int *current = 0;
    unsigned int *end = 0;

    /* dump regions' data */
    for(i=0; i < N9_REGION_TOTAL; i++)
    {
        if ( !regions[i].region_name )
        {
           break;
        }
        if ( !regions[i].is_dumped )
        {
           continue;
        }
        current = (unsigned int *)((uint32_t)(regions[i].start_address) & 0xfffffffc);
        end     = (unsigned int *)(regions[i].end_address);
        for (; current < end; current += 4)
        {
            if (*(current + 0) == 0 &&
                *(current + 1) == 0 &&
                *(current + 2) == 0 &&
                *(current + 3) == 0 )
            {
                continue;
            }
            exception_printf("0x%08x: %08x %08x %08x %08x\r\n",
                            (unsigned int)current,
                            *(current + 0),
                            *(current + 1),
                            *(current + 2),
                            *(current + 3));
        }
    }
}

#elif (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_BINARY)
static void exception_n9_dump_region_info(const memory_region_type *regions)
{
    uint32_t i = 0;
    unsigned int *current = 0;
    unsigned int *end = 0;

    for(i=0; i < N9_REGION_TOTAL; i++)
    {
        if(regions[i].is_dumped)
        {
            current = (unsigned int *)((uint32_t)(regions[i].start_address) & 0xfffffffc);
            end     = (unsigned int *)(regions[i].end_address);
            exception_printf("Region-%s: start_address = 0x%x, end_address = 0x%x\r\n",
                             regions[i].region_name,
                             (uint32_t)current,
                             (uint32_t)end);
        }
    }
}

static void exception_n9_dump_region_data_binary(const memory_region_type *regions)
{
    uint32_t i = 0;
    unsigned int *current = 0;
    unsigned int *end = 0;

    /* dump regions' data */
    for(i=0; i < N9_REGION_TOTAL; i++)
    {
        if(regions[i].is_dumped)
        {
            current = (unsigned int *)((uint32_t)(regions[i].start_address) & 0xfffffffc);
            end     = (unsigned int *)(regions[i].end_address);
            if(current < end)
            {
                log_dump_exception_data((const uint8_t *)current, (uint32_t)end - (uint32_t)current);
            }
        }
    }
}

#elif (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
static void exception_n9_dump_region_data_minidump(const memory_region_type *regions)
{
    uint32_t current_addr = 0;
    bool ret = false;

    /* static regions */
    if(minidump_base_address_n9 != 0)
    {
        /* dump DLRAM */
        current_addr = minidump_base_address_n9 + MINIDUMP_HEADER_LENGTH_N9 + MINIDUMP_CONTEXT_LENGTH_N9;
        minidump_header_n9.regions[0].address = 0x4010ED08;
        minidump_header_n9.regions[0].size = 0x2E0;
        ret = offline_dump_region_write(OFFLINE_REGION_MINI_DUMP,
                                        current_addr,
                                        (uint8_t *)0x4010ED08,
                                        0x2D4);
        if(ret != true)
        {
            minidump_base_address_n9 = 0;
            return;
        }

        current_addr += 0x2E0;
        minidump_header_n9.regions[1].address = 0x4010E400;
        minidump_header_n9.regions[1].size = 0x120;
        ret = offline_dump_region_write(OFFLINE_REGION_MINI_DUMP,
                                        current_addr,
                                        (uint8_t *)0x4010E400,
                                        0x120);
        if(ret != true)
        {
            minidump_base_address_n9 = 0;
            return;
        }

        /* dump SYSRAM */
        current_addr += 0x120;
        minidump_header_n9.regions[2].address = 0x40405C00;
        minidump_header_n9.regions[2].size = 0x400;
        ret = offline_dump_region_write(OFFLINE_REGION_MINI_DUMP,
                                        current_addr,
                                        (uint8_t *)0x40405C00,
                                        0x400);
        if(ret != true)
        {
            minidump_base_address_n9 = 0;
            return;
        }

        minidump_header_n9.name[0]  = 'N';
        minidump_header_n9.name[1]  = '9';
        minidump_header_n9.name[2]  = ' ';
        minidump_header_n9.name[3]  = ' ';
        minidump_header_n9.name[4]  = 'M';
        minidump_header_n9.name[5]  = 'I';
        minidump_header_n9.name[6]  = 'N';
        minidump_header_n9.name[7]  = 'I';
        minidump_header_n9.name[8]  = 'D';
        minidump_header_n9.name[9]  = 'U';
        minidump_header_n9.name[10] = 'M';
        minidump_header_n9.name[11] = 'P';
        minidump_header_n9.name[12] = 'S';
        minidump_header_n9.name[13] = 'U';
        minidump_header_n9.name[14] = 'C';
        minidump_header_n9.name[15] = 'C';
        minidump_header_n9.name[16] = 'E';
        minidump_header_n9.name[17] = 'E';
        minidump_header_n9.name[18] = 'D';
        minidump_header_n9.name[19] = 0;

        minidump_header_n9.length = MINIDUMP_DATA_SIZE_N9;

        ret = offline_dump_region_write(OFFLINE_REGION_MINI_DUMP,
                                        minidump_base_address_n9,
                                        (uint8_t *)&minidump_header_n9,
                                        sizeof(minidump_header_n9));
        if(ret != true)
        {
            minidump_base_address_n9 = 0;
            return;
        }
    }
}

#endif /* EXCEPTION_MEMDUMP_MODE */

/******************************************************************************/
/*            N9 Processor Exceptions Handlers                              */
/******************************************************************************/
void exception_alert_n9(void)
{
    hal_ccni_set_event(EXCEPTION_EVENT_TO_N9, NULL);
}

exception_slave_status_t exception_check_status_n9(void)
{
    uint32_t slave_status = HAL_CCNI_EVENT_STATUS_BUSY;

    if(HAL_CCNI_STATUS_OK == hal_ccni_query_event_status(EXCEPTION_EVENT_TO_N9, &slave_status))
    {
        if(slave_status == HAL_CCNI_EVENT_STATUS_IDLE)
        {
            /* N9 is ready */
            return EXCEPTION_SLAVE_STATUS_READY;
        }
        else
        {
            /* N9 is not ready */
            return EXCEPTION_SLAVE_STATUS_IDLE;
        }
    }

    return EXCEPTION_SLAVE_STATUS_IDLE;
}

void exception_dump_n9(void)
{
#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_TEXT)
    exception_printf("\r\nN9 Fault Dump:\r\n");

    /* print assert infomation */
    char *assert_info = (char *)0x40101aec;
    if(*assert_info != 0)
    {
        exception_printf("N9 assert failed:%s\r\n", assert_info);
    }

    /* dump regions' info */
    exception_printf("N9 Regions Information:\r\n");
    exception_n9_dump_region_info(n9_memory_regions);

    /* Print Regions' data */
    exception_n9_dump_region_data_text(n9_memory_regions);

    /* dump n9 end log */
    exception_printf("\r\nN9 memory dump completed.\r\n");

#elif (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_BINARY)
    exception_printf("\r\nN9 Fault Dump:\r\n");

    /* print assert infomation */
    char *assert_info = (char *)0x40101aec;
    if(*assert_info != 0)
    {
        exception_printf("N9 assert failed:%s\r\n", assert_info);
    }

    /* dump regions' info */
    exception_printf("N9 Regions Information:\r\n");
    exception_n9_dump_region_info(n9_memory_regions);

    /* Print Regions' data */
    exception_printf("N9 Regions Data:\r\n");
    exception_n9_dump_region_data_binary(n9_memory_regions);

    /* dump n9 end log */
    exception_printf("\r\nN9 memory dump completed.\r\n");

#elif (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
    if((minidump_base_address != 0))
    {
        minidump_base_address_n9 = minidump_base_address + MINIDUMP_ADDRESS_OFFSET_N9;

        exception_n9_dump_region_data_minidump(n9_memory_regions);
    }

#endif /* EXCEPTION_MEMDUMP_MODE */
}

void exception_forceddump_n9(void)
{
    // lock N9 RESET
    uint32_t rdata = *((volatile uint32_t *) (0xA2090030));

    if ( !(rdata & (0x1 << 24))) {
        *((volatile uint32_t*) (0xA2090030)) = (rdata & (~(0x1FF << 16))) | (0x11E << 16);
    }

#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_TEXT)
    exception_printf("\r\nN9 Fault Dump:\r\n");
    exception_printf("N9 Forced-Dump:\r\n");

    /* print assert infomation */
    char *assert_info = (char *)0x40101aec;
    if(*assert_info != 0)
    {
        exception_printf("N9 assert failed:%s\r\n", assert_info);
    }

    /* dump regions' info */
    exception_printf("N9 Regions Information:\r\n");
    exception_n9_dump_region_info(n9_memory_regions);

    /* Print Regions' data */
    exception_n9_dump_region_data_text(n9_memory_regions);

    /* dump n9 end log */
    exception_printf("\r\nN9 memory dump completed.\r\n");

#elif (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_BINARY)
    exception_printf("\r\nN9 Fault Dump:\r\n");
    exception_printf("N9 Forced-Dump:\r\n");

    /* print assert infomation */
    char *assert_info = (char *)0x40101aec;
    if(*assert_info != 0)
    {
        exception_printf("N9 assert failed:%s\r\n", assert_info);
    }

    /* dump regions' info */
    exception_printf("N9 Regions Information:\r\n");
    exception_n9_dump_region_info(n9_memory_regions);

    /* Print Regions' data */
    exception_printf("N9 Regions Data:\r\n");
    exception_n9_dump_region_data_binary(n9_memory_regions);

    /* dump n9 end log */
    exception_printf("\r\nN9 memory dump completed.\r\n");

#elif (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
    if((minidump_base_address != 0))
    {
        minidump_base_address_n9 = minidump_base_address + MINIDUMP_ADDRESS_OFFSET_N9;

        exception_n9_dump_region_data_minidump(n9_memory_regions);
    }
#endif /* EXCEPTION_MEMDUMP_MODE */
}


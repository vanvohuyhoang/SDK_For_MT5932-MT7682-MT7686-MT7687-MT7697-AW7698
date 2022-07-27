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
#include "exception_handler_dsp.h"

#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
#include "offline_dump.h"
#endif /* EXCEPTION_MEMDUMP_MODE */


/* Private define -----------------------------------------------------------*/
#define EXCEPTION_DSP_PATTERN1                          (0xff00a5a5)
#define EXCEPTION_DSP_PATTERN2                          (0x5a5a00ff)

#define EXCEPTION_CONTEXT_WORDS_DSP                     (106)
#define EXCEPTION_RESERVED_WORDS_DSP                    (111)

#define EXCEPTION_SHARE_MEMORY_ADDRESS_DSP0             (HW_SYSRAM_PRIVATE_MEMORY_EXCEPTION_START)
#define EXCEPTION_SHARE_MEMORY_ADDRESS_DSP1             (HW_SYSRAM_PRIVATE_MEMORY_EXCEPTION_START + 0x200)

#define EXCEPTION_EVENT_TO_DSP0                         IRQGEN_CM4_TO_DSP0_EXCEPTION
#define EXCEPTION_EVENT_TO_DSP1                         IRQGEN_CM4_TO_DSP1_EXCEPTION
#define EXCEPTION_EVENT_FROM_DSP0                       IRQGEN_DSP0_TO_CM4_EXCEPTION
#define EXCEPTION_EVENT_FROM_DSP1                       IRQGEN_DSP1_TO_CM4_EXCEPTION


/* Private typedef ----------------------------------------------------------*/
/* Total 106 word, 424 Byte */
typedef struct
{
    /* General core registers */
    uint32_t ar0;
    uint32_t ar1;
    uint32_t ar2;
    uint32_t ar3;
    uint32_t ar4;
    uint32_t ar5;
    uint32_t ar6;
    uint32_t ar7;
    uint32_t ar8;
    uint32_t ar9;
    uint32_t ar10;
    uint32_t ar11;
    uint32_t ar12;
    uint32_t ar13;
    uint32_t ar14;
    uint32_t ar15;
    uint32_t ar16;
    uint32_t ar17;
    uint32_t ar18;
    uint32_t ar19;
    uint32_t ar20;
    uint32_t ar21;
    uint32_t ar22;
    uint32_t ar23;
    uint32_t ar24;
    uint32_t ar25;
    uint32_t ar26;
    uint32_t ar27;
    uint32_t ar28;
    uint32_t ar29;
    uint32_t ar30;
    uint32_t ar31;
    /* Window option special registers */
    uint32_t windowbase;
    uint32_t windowstart;
    /* Loop option special registers */
    uint32_t lbeg;
    uint32_t lend;
    uint32_t lcount;
    /* Shift amount special registers */
    uint32_t sar;
    /* Comparison special registers */
    uint32_t scompare;
    /* Exception and Interrupt option special registers */
    uint32_t exccause;
    uint32_t excvaddr;
    uint32_t pc;
    uint32_t epc1;
    uint32_t epc2;
    uint32_t epc3;
    uint32_t epc4;
    uint32_t epc5;
    uint32_t epc6;
    uint32_t epcnmi;
    uint32_t depc;
    uint32_t ps;
    uint32_t eps2;
    uint32_t eps3;
    uint32_t eps4;
    uint32_t eps5;
    uint32_t eps6;
    uint32_t epsnmi;
    uint32_t excsave1;
    uint32_t excsave2;
    uint32_t excsave3;
    uint32_t excsave4;
    uint32_t excsave5;
    uint32_t excsave6;
    uint32_t excsavenmi;
    uint32_t intenable;
    uint32_t interrupt;
    /* Bool option special registers */
    uint32_t br;
    /* Coprocessor option special registers */
    uint32_t cpenable;
    /* Debug option special registers */
    uint32_t debugcause;
    uint32_t ibreakenable;
    uint32_t ibreaka0;
    uint32_t ibreaka1;
    uint32_t dbreaka0;
    uint32_t dbreaka1;
    uint32_t dbreakc0;
    uint32_t dbreakc1;
    /* DSP engine special registers( 8 Byte aligned ) */
    uint64_t aep0;
    uint64_t aep1;
    uint64_t aep2;
    uint64_t aep3;
    uint64_t aep4;
    uint64_t aep5;
    uint64_t aep6;
    uint64_t aep7;
    uint64_t aeq0;
    uint64_t aeq1;
    uint64_t aeq2;
    uint64_t aeq3;
    uint32_t ae_ovf_sar;
    uint32_t ae_bithead;
    uint32_t ae_ts_fts_bu_bp;
    uint32_t ae_sd_no;
    uint32_t ae_cbegin0;
    uint32_t ae_cend0;
} exception_context_dsp_t;

/* 4 + 4 + 4 + 4 + 4*4 + 4*2 + 4*2 + 4*2 + 4*EXCEPTION_RESERVED_WORDS + 4 + 4 + 4 = 512B  */
typedef struct
{
    uint32_t packet_head;
    uint32_t corenum;
    uint32_t count;
    uint32_t timestamp;
    /* These variables will be included in BCC verify */
    assert_expr_t assert_info;
    /* These variables will be included in BCC verify */
    exception_context_dsp_t* context_regs;
    /* These variables will be included in BCC verify */
    uint32_t context_regs_num;
    /* These variables will be included in BCC verify */
    memory_region_type* static_regions;
    /* These variables will be included in BCC verify */
    uint32_t static_regions_items;
    /* These variables will be included in BCC verify */
    memory_region_type* dynamic_regions;
    /* These variables will be included in BCC verify */
    uint32_t dynamic_regions_items;
    uint32_t context_backup[EXCEPTION_RESERVED_WORDS_DSP];
    uint32_t data_checksum;
    uint32_t packet_size;
    uint32_t packet_tail;
} exception_sharemem_dsp_t;

#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
typedef struct
{
    uint32_t address;
    uint32_t size;
}exception_minidump_region_dsp_t;

typedef struct
{
    uint32_t length;
    char name[20];
    uint32_t reason;
    assert_expr_t assert_info;
    uint32_t context_size;
    exception_minidump_region_dsp_t regions[8];
    uint32_t data_checksum;
}exception_minidump_header_dsp_t;
#endif /* EXCEPTION_MEMDUMP_MODE */


/* Private macro ------------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
uint32_t minidump_base_address_dsp = 0;
exception_minidump_header_dsp_t minidump_header_dsp = {0};
#endif /* EXCEPTION_MEMDUMP_MODE */


/* Private functions --------------------------------------------------------*/
/******************************************************************************/
/*            Exception's Common Functions                                    */
/******************************************************************************/
static exception_status_t exception_dsp_packet_checkout(int num, exception_sharemem_dsp_t *pESM)
{
    uint32_t i = 0;
    uint32_t check_sum = 0x0;
    exception_context_dsp_t* context_regs = NULL;

    /* Check packet head and tail */
    if((pESM->packet_head != EXCEPTION_DSP_PATTERN1) ||
       (pESM->packet_tail != EXCEPTION_DSP_PATTERN2))
    {
        return EXCEPTION_STATUS_ERROR;
    }

    /* Check core number */
    if(pESM->corenum != num)
    {
        return EXCEPTION_STATUS_ERROR;
    }

    /* Check variables (BCC verify) */
    check_sum ^= (pESM->assert_info).is_valid;
    check_sum ^= (uint32_t)((pESM->assert_info).expr);
    check_sum ^= (uint32_t)((pESM->assert_info).file);
    check_sum ^= (pESM->assert_info).line;
    check_sum ^= (uint32_t)(pESM->context_regs);
    check_sum ^= pESM->context_regs_num;
    check_sum ^= (uint32_t)(pESM->static_regions);
    check_sum ^= pESM->static_regions_items;
    check_sum ^= (uint32_t)(pESM->dynamic_regions);
    check_sum ^= pESM->dynamic_regions_items;
    if(check_sum != pESM->data_checksum)
    {
        return EXCEPTION_STATUS_ERROR;
    }

    if(num == 0)
    {
        /* CM4 need to transfer DSP0 address into CM4 view */
        context_regs = (exception_context_dsp_t *)hal_memview_dsp0_to_cm4((uint32_t)(pESM->context_regs));
    }
    else
    {
        /* CM4 need to transfer DSP1 address into CM4 view */
        context_regs = (exception_context_dsp_t *)hal_memview_dsp1_to_cm4((uint32_t)(pESM->context_regs));
    }

    /* check backup registers */
#if EXCEPTION_CONTEXT_WORDS_DSP < EXCEPTION_RESERVED_WORDS_DSP
    for(i = 0; i < EXCEPTION_CONTEXT_WORDS_DSP; i++)
    {
        if (*(pESM->context_backup + i) != *((uint32_t *)context_regs + i))
        {
            exception_printf("[Warning]context registers and backup registers are different\r\n");
            break;
        }
    }
#else /* EXCEPTION_CONTEXT_WORDS_DSP >= EXCEPTION_RESERVED_WORDS_DSP */
    for(i = 0; i < EXCEPTION_RESERVED_WORDS_DSP; i++)
    {
        if (*(pESM->context_backup + i) != *((uint32_t *)context_regs + i))
        {
            exception_printf("[Warning]context registers and backup registers are different\r\n");
            break;
        }
    }
#endif /* EXCEPTION_CONTEXT_WORDS_DSP < EXCEPTION_RESERVED_WORDS_DSP */

    return EXCEPTION_STATUS_OK;
}

/******************************************************************************/
/*            Exception's context dump Functions                              */
/******************************************************************************/
static void exception_dsp_dump_context(int num, exception_sharemem_dsp_t *pESM)
{
    /* Context Dump */
#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_TEXT) || (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_BINARY)
    exception_context_dsp_t* context_regs = NULL;

	exception_printf("\r\nDSP%d Fault Dump:\r\n", pESM->corenum);

    if(num == 0)
    {
        /* CM4 need to transfer DSP0 address into CM4 view */
        context_regs = (exception_context_dsp_t *)hal_memview_dsp0_to_cm4((uint32_t)(pESM->context_regs));

        /* Check if there is a assert */
        if ((pESM->assert_info).is_valid)
        {
           /* CM4 need to transfer DSP0 address into CM4 view */
            exception_printf("DSP%d assert failed: %s, file: %s, line: %d\r\n",
                            pESM->corenum,
                            (const char *)hal_memview_dsp0_to_cm4((uint32_t)((pESM->assert_info).expr)),
                            (const char *)hal_memview_dsp0_to_cm4((uint32_t)((pESM->assert_info).file)),
                            (pESM->assert_info).line);
        }
    }
    else
    {
        /* CM4 need to transfer DSP1 address into CM4 view */
        context_regs = (exception_context_dsp_t *)hal_memview_dsp1_to_cm4((uint32_t)(pESM->context_regs));

        /* Check if there is a assert */
        if ((pESM->assert_info).is_valid)
        {
            /* CM4 need to transfer DSP1 address into CM4 view */
            exception_printf("DSP%d assert failed: %s, file: %s, line: %d\r\n",
                            pESM->corenum,
                            (const char *)hal_memview_dsp1_to_cm4((uint32_t)((pESM->assert_info).expr)),
                            (const char *)hal_memview_dsp1_to_cm4((uint32_t)((pESM->assert_info).file)),
                            (pESM->assert_info).line);
        }
    }

    /* Info Dump */
    exception_printf("Exception Count = 0x%08x\r\n", pESM->count);
    exception_printf("Exception Time = 0x%08x\r\n", pESM->timestamp);
    exception_printf("Exception Reason = 0x%08x\r\n", context_regs->exccause);

    /* General core registers */
    exception_printf("DSP%d Register dump begin:\r\n", pESM->corenum);
    exception_printf("ar0  = 0x%08x\r\n", context_regs->ar0);
    exception_printf("ar1  = 0x%08x\r\n", context_regs->ar1);
    exception_printf("ar2  = 0x%08x\r\n", context_regs->ar2);
    exception_printf("ar3  = 0x%08x\r\n", context_regs->ar3);
    exception_printf("ar4  = 0x%08x\r\n", context_regs->ar4);
    exception_printf("ar5  = 0x%08x\r\n", context_regs->ar5);
    exception_printf("ar6  = 0x%08x\r\n", context_regs->ar6);
    exception_printf("ar7  = 0x%08x\r\n", context_regs->ar7);
    exception_printf("ar8  = 0x%08x\r\n", context_regs->ar8);
    exception_printf("ar9  = 0x%08x\r\n", context_regs->ar9);
    exception_printf("ar10  = 0x%08x\r\n", context_regs->ar10);
    exception_printf("ar11  = 0x%08x\r\n", context_regs->ar11);
    exception_printf("ar12  = 0x%08x\r\n", context_regs->ar12);
    exception_printf("ar13  = 0x%08x\r\n", context_regs->ar13);
    exception_printf("ar14  = 0x%08x\r\n", context_regs->ar14);
    exception_printf("ar15  = 0x%08x\r\n", context_regs->ar15);
    exception_printf("ar16  = 0x%08x\r\n", context_regs->ar16);
    exception_printf("ar17  = 0x%08x\r\n", context_regs->ar17);
    exception_printf("ar18  = 0x%08x\r\n", context_regs->ar18);
    exception_printf("ar19  = 0x%08x\r\n", context_regs->ar19);
    exception_printf("ar20  = 0x%08x\r\n", context_regs->ar20);
    exception_printf("ar21  = 0x%08x\r\n", context_regs->ar21);
    exception_printf("ar22  = 0x%08x\r\n", context_regs->ar22);
    exception_printf("ar23  = 0x%08x\r\n", context_regs->ar23);
    exception_printf("ar24  = 0x%08x\r\n", context_regs->ar24);
    exception_printf("ar25  = 0x%08x\r\n", context_regs->ar25);
    exception_printf("ar26  = 0x%08x\r\n", context_regs->ar26);
    exception_printf("ar27  = 0x%08x\r\n", context_regs->ar27);
    exception_printf("ar28  = 0x%08x\r\n", context_regs->ar28);
    exception_printf("ar29  = 0x%08x\r\n", context_regs->ar29);
    exception_printf("ar30  = 0x%08x\r\n", context_regs->ar30);
    exception_printf("ar31  = 0x%08x\r\n", context_regs->ar31);
    /* Window option special registers */
    exception_printf("windowbase  = 0x%08x\r\n", context_regs->windowbase);
    exception_printf("windowstart  = 0x%08x\r\n", context_regs->windowstart);
    /* Loop option special registers */
    exception_printf("lbeg  = 0x%08x\r\n", context_regs->lbeg);
    exception_printf("lend  = 0x%08x\r\n", context_regs->lend);
    exception_printf("lcount  = 0x%08x\r\n", context_regs->lcount);
    /* Shift amount special registers */
    exception_printf("sar  = 0x%08x\r\n", context_regs->sar);
    /* Comparison special registers */
    exception_printf("scompare  = 0x%08x\r\n", context_regs->scompare);
    /* Exception and Interrupt option special registers */
    exception_printf("exccause  = 0x%08x\r\n", context_regs->exccause);
    exception_printf("excvaddr  = 0x%08x\r\n", context_regs->excvaddr);
    exception_printf("pc  = 0x%08x\r\n", context_regs->pc);
    exception_printf("epc1  = 0x%08x\r\n", context_regs->epc1);
    exception_printf("epc2  = 0x%08x\r\n", context_regs->epc2);
    exception_printf("epc3  = 0x%08x\r\n", context_regs->epc3);
    exception_printf("epc4  = 0x%08x\r\n", context_regs->epc4);
    exception_printf("epc5  = 0x%08x\r\n", context_regs->epc5);
    exception_printf("epc6  = 0x%08x\r\n", context_regs->epc6);
    exception_printf("epcnmi  = 0x%08x\r\n", context_regs->epcnmi);
    exception_printf("depc  = 0x%08x\r\n", context_regs->depc);
    exception_printf("ps  = 0x%08x\r\n", context_regs->ps);
    exception_printf("eps2  = 0x%08x\r\n", context_regs->eps2);
    exception_printf("eps3  = 0x%08x\r\n", context_regs->eps3);
    exception_printf("eps4  = 0x%08x\r\n", context_regs->eps4);
    exception_printf("eps5  = 0x%08x\r\n", context_regs->eps5);
    exception_printf("eps6  = 0x%08x\r\n", context_regs->eps6);
    exception_printf("epsnmi  = 0x%08x\r\n", context_regs->epsnmi);
    exception_printf("excsave1  = 0x%08x\r\n", context_regs->excsave1);
    exception_printf("excsave2  = 0x%08x\r\n", context_regs->excsave2);
    exception_printf("excsave3  = 0x%08x\r\n", context_regs->excsave3);
    exception_printf("excsave4  = 0x%08x\r\n", context_regs->excsave4);
    exception_printf("excsave5  = 0x%08x\r\n", context_regs->excsave5);
    exception_printf("excsave6  = 0x%08x\r\n", context_regs->excsave6);
    exception_printf("excsavenmi  = 0x%08x\r\n", context_regs->excsavenmi);
    exception_printf("intenable  = 0x%08x\r\n", context_regs->intenable);
    exception_printf("interrupt  = 0x%08x\r\n", context_regs->interrupt);
    /* Bool option special registers */
    exception_printf("br  = 0x%08x\r\n", context_regs->br);
    /* Coprocessor option special registers */
    exception_printf("cpenable  = 0x%08x\r\n", context_regs->cpenable);
    /* Debug option special registers */
    exception_printf("debugcause  = 0x%08x\r\n", context_regs->debugcause);
    exception_printf("ibreakenable  = 0x%08x\r\n", context_regs->ibreakenable);
    exception_printf("ibreaka0  = 0x%08x\r\n", context_regs->ibreaka0);
    exception_printf("ibreaka1  = 0x%08x\r\n", context_regs->ibreaka1);
    exception_printf("dbreaka0  = 0x%08x\r\n", context_regs->dbreaka0);
    exception_printf("dbreaka1  = 0x%08x\r\n", context_regs->dbreaka1);
    exception_printf("dbreakc0  = 0x%08x\r\n", context_regs->dbreakc0);
    exception_printf("dbreakc1  = 0x%08x\r\n", context_regs->dbreakc1);
    /* DSP engine special registers */
    exception_printf("aep0_L  = 0x%08x\r\n", (uint32_t)(context_regs->aep0 & 0xffffffff));
    exception_printf("aep0_H  = 0x%08x\r\n", (uint32_t)((context_regs->aep0 >> 32) & 0xffffffff));
    exception_printf("aep1_L  = 0x%08x\r\n", (uint32_t)(context_regs->aep1 & 0xffffffff));
    exception_printf("aep1_H  = 0x%08x\r\n", (uint32_t)((context_regs->aep1 >> 32) & 0xffffffff));
    exception_printf("aep2_L  = 0x%08x\r\n", (uint32_t)(context_regs->aep2 & 0xffffffff));
    exception_printf("aep2_H  = 0x%08x\r\n", (uint32_t)((context_regs->aep2 >> 32) & 0xffffffff));
    exception_printf("aep3_L  = 0x%08x\r\n", (uint32_t)(context_regs->aep3 & 0xffffffff));
    exception_printf("aep3_H  = 0x%08x\r\n", (uint32_t)((context_regs->aep3 >> 32) & 0xffffffff));
    exception_printf("aep4_L  = 0x%08x\r\n", (uint32_t)(context_regs->aep4 & 0xffffffff));
    exception_printf("aep4_H  = 0x%08x\r\n", (uint32_t)((context_regs->aep4 >> 32) & 0xffffffff));
    exception_printf("aep5_L  = 0x%08x\r\n", (uint32_t)(context_regs->aep5 & 0xffffffff));
    exception_printf("aep5_H  = 0x%08x\r\n", (uint32_t)((context_regs->aep5 >> 32) & 0xffffffff));
    exception_printf("aep6_L  = 0x%08x\r\n", (uint32_t)(context_regs->aep6 & 0xffffffff));
    exception_printf("aep6_H  = 0x%08x\r\n", (uint32_t)((context_regs->aep6 >> 32) & 0xffffffff));
    exception_printf("aep7_L  = 0x%08x\r\n", (uint32_t)(context_regs->aep7 & 0xffffffff));
    exception_printf("aep7_H  = 0x%08x\r\n", (uint32_t)((context_regs->aep7 >> 32) & 0xffffffff));
    exception_printf("aeq0_L  = 0x%08x\r\n", (uint32_t)(context_regs->aeq0 & 0xffffffff));
    exception_printf("aeq0_H  = 0x%08x\r\n", (uint32_t)((context_regs->aeq0 >> 32) & 0xffffffff));
    exception_printf("aeq1_L  = 0x%08x\r\n", (uint32_t)(context_regs->aeq1 & 0xffffffff));
    exception_printf("aeq1_H  = 0x%08x\r\n", (uint32_t)((context_regs->aeq1 >> 32) & 0xffffffff));
    exception_printf("aeq2_L  = 0x%08x\r\n", (uint32_t)(context_regs->aeq2 & 0xffffffff));
    exception_printf("aeq2_H  = 0x%08x\r\n", (uint32_t)((context_regs->aeq2 >> 32) & 0xffffffff));
    exception_printf("aeq3_L  = 0x%08x\r\n", (uint32_t)(context_regs->aeq3 & 0xffffffff));
    exception_printf("aeq3_H  = 0x%08x\r\n", (uint32_t)((context_regs->aeq3 >> 32) & 0xffffffff));
    exception_printf("ae_ovf_sar  = 0x%08x\r\n", context_regs->ae_ovf_sar);
    exception_printf("ae_bithead  = 0x%08x\r\n", context_regs->ae_bithead);
    exception_printf("ae_ts_fts_bu_bp  = 0x%08x\r\n", context_regs->ae_ts_fts_bu_bp);
    exception_printf("ae_sd_no  = 0x%08x\r\n", context_regs->ae_sd_no);
    exception_printf("ae_cbegin0  = 0x%08x\r\n", context_regs->ae_cbegin0);
    exception_printf("ae_cend0  = 0x%08x\r\n", context_regs->ae_cend0);
    exception_printf("DSP%d Register dump end:\r\n", pESM->corenum);

#elif (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
    exception_context_dsp_t* context_regs = NULL;
    uint32_t current_addr = 0;
    bool ret = false;

    if(num == 0)
    {
        /* CM4 need to transfer DSP0 address into CM4 view */
        context_regs = (exception_context_dsp_t *)hal_memview_dsp0_to_cm4((uint32_t)(pESM->context_regs));

        /* Check if there is a assert */
        if ((pESM->assert_info).is_valid)
        {
           /* CM4 need to transfer DSP0 address into CM4 view */
            minidump_header_dsp.assert_info.is_valid = (pESM->assert_info).is_valid;
            minidump_header_dsp.assert_info.expr = (const char *)hal_memview_dsp0_to_cm4((uint32_t)((pESM->assert_info).expr));
            minidump_header_dsp.assert_info.file = (const char *)hal_memview_dsp0_to_cm4((uint32_t)((pESM->assert_info).file));
            minidump_header_dsp.assert_info.line = (pESM->assert_info).line;
        }
        else
        {
            minidump_header_dsp.assert_info.is_valid = false;
        }

        if(minidump_base_address_dsp != 0)
        {
            minidump_header_dsp.regions[0].address = *(uint32_t *)((uint32_t)context_regs + context_regs->windowbase * 4 * 4 + 4) - 16;
            minidump_header_dsp.regions[0].size = MINIDUMP_CURRENTSTACK_LENGTH_DSP0;
            // minidump_header_dsp.regions[1].address = 0;
            // minidump_header_dsp.regions[1].size = 0;

            /* dump the data on exception share memory */
            current_addr = minidump_base_address_dsp + MINIDUMP_HEADER_LENGTH_DSP0;
            ret = offline_dump_region_write(OFFLINE_REGION_MINI_DUMP,
                                            current_addr,
                                            (uint8_t *)context_regs,
                                            EXCEPTION_CONTEXT_WORDS_DSP * 4);
            if(ret != true)
            {
                minidump_base_address_dsp = 0;
                return;
            }

            minidump_header_dsp.context_size = EXCEPTION_CONTEXT_WORDS_DSP * 4;
        }
    }
    else if(num == 1)
    {
        /* CM4 need to transfer DSP1 address into CM4 view */
        context_regs = (exception_context_dsp_t *)hal_memview_dsp1_to_cm4((uint32_t)(pESM->context_regs));

        /* Check if there is a assert */
        if ((pESM->assert_info).is_valid)
        {
            /* CM4 need to transfer DSP1 address into CM4 view */
            minidump_header_dsp.assert_info.is_valid = (pESM->assert_info).is_valid;
            minidump_header_dsp.assert_info.expr = (const char *)hal_memview_dsp1_to_cm4((uint32_t)((pESM->assert_info).expr));
            minidump_header_dsp.assert_info.file = (const char *)hal_memview_dsp1_to_cm4((uint32_t)((pESM->assert_info).file));
            minidump_header_dsp.assert_info.line = (pESM->assert_info).line;
        }
        else
        {
            minidump_header_dsp.assert_info.is_valid = false;
        }

        if(minidump_base_address_dsp != 0)
        {
            minidump_header_dsp.regions[0].address = *(uint32_t *)((uint32_t)context_regs + context_regs->windowbase * 4 * 4 + 4) - 16;
            minidump_header_dsp.regions[0].size = MINIDUMP_CURRENTSTACK_LENGTH_DSP1;
            // minidump_header_dsp.regions[1].address = 0;
            // minidump_header_dsp.regions[1].size = 0;

            /* dump the data on exception share memory */
            current_addr = minidump_base_address_dsp + MINIDUMP_HEADER_LENGTH_DSP1;
            ret = offline_dump_region_write(OFFLINE_REGION_MINI_DUMP,
                                            current_addr,
                                            (uint8_t *)context_regs,
                                            EXCEPTION_CONTEXT_WORDS_DSP * 4);
            if(ret != true)
            {
                minidump_base_address_dsp = 0;
                return;
            }

            minidump_header_dsp.context_size = EXCEPTION_CONTEXT_WORDS_DSP * 4;
        }
    }

#endif /* EXCEPTION_MEMDUMP_MODE */
}

/******************************************************************************/
/*            Exception's memory dump Functions                               */
/******************************************************************************/
#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_TEXT) || (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_BINARY)
static void exception_dsp_dump_region_info(uint32_t num, uint32_t items, memory_region_type *regions_base)
{
    uint32_t i = 0;
    unsigned int *current = 0;
    unsigned int *end = 0;
    memory_region_type *region = NULL;
    char *region_name = NULL;

    for (i = 0; i < items; i++)
    {
        if(num == 0)
        {
            /* CM4 need to transfer DSP0 address into CM4 view */
            region = (memory_region_type *)hal_memview_dsp0_to_cm4((uint32_t)(regions_base + i));
            region_name = (char *)hal_memview_dsp0_to_cm4((uint32_t)(region->region_name));
        }
        else
        {
            /* CM4 need to transfer DSP1 address into CM4 view */
            region = (memory_region_type *)hal_memview_dsp1_to_cm4((uint32_t)(regions_base + i));
            region_name = (char *)hal_memview_dsp1_to_cm4((uint32_t)(region->region_name));
        }

        if (region->is_dumped)
        {
            /* Make sure start address 4-byte align */
            current = (unsigned int *)((uint32_t)(region->start_address) & 0xfffffffc);
            end     = (unsigned int *)(region->end_address);
            if(current < end)
            {
                /* CM4 need to print address in DSPx view */
                exception_printf("Region-%s: start_address = 0x%x, end_address = 0x%x\r\n", region_name, (unsigned int)current, (unsigned int)end);
            }
        }
    }
}
#endif /* EXCEPTION_MEMDUMP_MODE */

#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_TEXT)
static void exception_dsp_dump_region_data_text(uint32_t num, uint32_t items, memory_region_type *regions_base)
{
    unsigned int i = 0;
    unsigned int j = 0;
    memory_region_type *region = NULL;
    unsigned int *current = 0;
    unsigned int *end = 0;
    unsigned int start_address = 0;

    for (i = 0; i < items; i++)
    {
        if(num == 0)
        {
            /* CM4 need to transfer DSP0 address into CM4 view */
            region = (memory_region_type *)hal_memview_dsp0_to_cm4((uint32_t)(regions_base + i));
            /* Make sure start address 4-byte align */
            current = (unsigned int *)(hal_memview_dsp0_to_cm4((uint32_t)(region->start_address)) & 0xfffffffc);
            start_address = (uint32_t)(region->start_address) & 0xfffffffc;
            end     = (unsigned int *)(hal_memview_dsp0_to_cm4((uint32_t)(region->end_address)));

        }
        else if(num == 1)
        {
            /* CM4 need to transfer DSP1 address into CM4 view */
            region = (memory_region_type *)hal_memview_dsp1_to_cm4((uint32_t)(regions_base + i));
            /* Make sure start address 4-byte align */
            current = (unsigned int *)(hal_memview_dsp1_to_cm4((uint32_t)(region->start_address)) & 0xfffffffc);
            start_address = (uint32_t)(region->start_address) & 0xfffffffc;
            end     = (unsigned int *)(hal_memview_dsp1_to_cm4((uint32_t)(region->end_address)));
        }

        if (region->is_dumped)
        {
            for (j = 0; current < end; current += 4, j++)
            {
                if (*(current + 0) == 0 &&
                    *(current + 1) == 0 &&
                    *(current + 2) == 0 &&
                    *(current + 3) == 0 )
                {
                        continue;
                }

                /* CM4 need to print DSPx address */
                exception_printf("0x%08x: %08x %08x %08x %08x\r\n",
                                (start_address + j * 0x10),
                                *(current + 0),
                                *(current + 1),
                                *(current + 2),
                                *(current + 3));
            }
        }
    }
}

#elif (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_BINARY)
static void exception_dsp_dump_region_data_binary(uint32_t num, uint32_t items, memory_region_type *regions_base)
{
    unsigned int i = 0;
    unsigned int *current = 0;
    unsigned int *end = 0;
    memory_region_type *region = NULL;

    for (i = 0; i < items; i++)
    {
        if(num == 0)
        {
            /* CM4 need to transfer DSP0 address into CM4 view */
            region = (memory_region_type *)hal_memview_dsp0_to_cm4((uint32_t)(regions_base + i));
            /* Make sure start address 4-byte align */
            current = (unsigned int *)(hal_memview_dsp0_to_cm4((uint32_t)(region->start_address)) & 0xfffffffc);
            end     = (unsigned int *)(hal_memview_dsp0_to_cm4((uint32_t)(region->end_address)));

        }
        else
        {
            /* CM4 need to transfer DSP1 address into CM4 view */
            region = (memory_region_type *)hal_memview_dsp1_to_cm4((uint32_t)(regions_base + i));
            /* Make sure start address 4-byte align */
            current = (unsigned int *)(hal_memview_dsp1_to_cm4((uint32_t)(region->start_address)) & 0xfffffffc);
            end     = (unsigned int *)(hal_memview_dsp1_to_cm4((uint32_t)(region->end_address)));
        }

        if (region->is_dumped)
        {
            if(current < end)
            {
                log_dump_exception_data((const uint8_t *)current, (uint32_t)end - (uint32_t)current);
            }
        }
    }
}

#elif (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
static exception_status_t exception_dsp_minidump_check_address(uint32_t num, uint32_t items, memory_region_type *regions_base, uint32_t address)
{
    uint32_t i = 0;
    uint32_t start, end;
    memory_region_type *region = NULL;

    for (i = 0; i < items ; i++) {
        if(num == 0)
        {
            /* CM4 need to transfer DSP0 address into CM4 view */
            region = (memory_region_type *)hal_memview_dsp0_to_cm4((uint32_t)(regions_base + i));
        }
        else
        {
            /* CM4 need to transfer DSP1 address into CM4 view */
            region = (memory_region_type *)hal_memview_dsp1_to_cm4((uint32_t)(regions_base + i));
        }

        start = (uint32_t)(region->start_address);
        end   = (uint32_t)(region->end_address);

        if((address >= start) && (address <= end))
        {
            return EXCEPTION_STATUS_OK;
        }

    }

    return EXCEPTION_STATUS_ERROR;
}

static void exception_dsp_dump_region_data_minidump(uint32_t num, uint32_t items, memory_region_type *regions_base)
{
    uint32_t current_addr = 0;
    bool ret = false;

    if(num == 0)
    {
        /* static regions */
        if(minidump_base_address_dsp != 0)
        {
            /* dump current stacks */
            current_addr = minidump_base_address_dsp + MINIDUMP_HEADER_LENGTH_DSP0 + MINIDUMP_CONTEXT_LENGTH_DSP0;
            ret = exception_dsp_minidump_check_address(0, items, regions_base, minidump_header_dsp.regions[0].address);
            if(ret == EXCEPTION_STATUS_OK)
            {
                ret = offline_dump_region_write(OFFLINE_REGION_MINI_DUMP,
                                                current_addr,
                                                (uint8_t *)(hal_memview_dsp0_to_cm4(minidump_header_dsp.regions[0].address)),
                                                minidump_header_dsp.regions[0].size);
                if(ret != true)
                {
                    minidump_base_address_dsp = 0;
                    return;
                }
            }
            else
            {
                minidump_header_dsp.regions[0].address = 0xffffffff - MINIDUMP_CURRENTSTACK_LENGTH_DSP0;
                minidump_header_dsp.regions[0].size = MINIDUMP_CURRENTSTACK_LENGTH_DSP0;
            }

            /* dump system stacks */
            // current_addr += minidump_header_dsp.regions[0].size;
            // ret = offline_dump_region_write(OFFLINE_REGION_MINI_DUMP,
            //                                 current_addr,
            //                                 (uint8_t *)(minidump_header_dsp.regions[1].address),
            //                                 minidump_header_dsp.regions[1].size);
            // if(ret != true)
            // {
            //     minidump_base_address_dsp = 0;
            //     return;
            // }
        }
    }
    else if(num == 1)
    {
        /* static regions */
        if(minidump_base_address_dsp != 0)
        {
            /* dump current stacks */
            current_addr = minidump_base_address_dsp + MINIDUMP_HEADER_LENGTH_DSP1 + MINIDUMP_CONTEXT_LENGTH_DSP1;
            ret = exception_dsp_minidump_check_address(1, items, regions_base, minidump_header_dsp.regions[0].address);
            if(ret == EXCEPTION_STATUS_OK)
            {
                ret = offline_dump_region_write(OFFLINE_REGION_MINI_DUMP,
                                                current_addr,
                                                (uint8_t *)(hal_memview_dsp1_to_cm4(minidump_header_dsp.regions[0].address)),
                                                minidump_header_dsp.regions[0].size);
                if(ret != true)
                {
                    minidump_base_address_dsp = 0;
                    return;
                }
            }
            else
            {
                minidump_header_dsp.regions[0].address = 0xffffffff - MINIDUMP_CURRENTSTACK_LENGTH_DSP1;
                minidump_header_dsp.regions[0].size = MINIDUMP_CURRENTSTACK_LENGTH_DSP1;
            }

            /* dump system stacks */
            // current_addr += minidump_header_dsp.regions[0].size;
            // ret = offline_dump_region_write(OFFLINE_REGION_MINI_DUMP,
            //                                 current_addr,
            //                                 (uint8_t *)(minidump_header_dsp.regions[1].address),
            //                                 minidump_header_dsp.regions[1].size);
            // if(ret != true)
            // {
            //     minidump_base_address_dsp = 0;
            //     return;
            // }
        }
    }
}

#endif /* EXCEPTION_MEMDUMP_MODE */

static void exception_dsp_dump_memory(int num, exception_sharemem_dsp_t *pESM)
{
    /* Memory Dump */
#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_TEXT)
    /* Print Regions' information */
    exception_printf("DSP%d Regions Information:\r\n", num);
    /* static regions */
    exception_dsp_dump_region_info(num, pESM->static_regions_items, pESM->static_regions);
    /* dynamic regions */
    exception_dsp_dump_region_info(num, pESM->dynamic_regions_items, pESM->dynamic_regions);

    /* Print Regions' data */
    /* static regions */
    exception_dsp_dump_region_data_text(num, pESM->static_regions_items, pESM->static_regions);
    /* dynamic regions */
    exception_dsp_dump_region_data_text(num, pESM->dynamic_regions_items, pESM->dynamic_regions);

    /* dump dsp end log */
    exception_printf("\r\nDSP%d memory dump completed.\r\n", num);

#elif (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_BINARY)
    /* Print Regions' information */
    exception_printf("DSP%d Regions Information:\r\n", num);
    /* static regions */
    exception_dsp_dump_region_info(num, pESM->static_regions_items, pESM->static_regions);
    /* dynamic regions */
    exception_dsp_dump_region_info(num, pESM->dynamic_regions_items, pESM->dynamic_regions);

    /* Print Memory one by one regions */
    exception_printf("DSP%d Regions Data:\r\n", num);
    /* static regions */
    exception_dsp_dump_region_data_binary(num, pESM->static_regions_items, pESM->static_regions);
    /* dynamic regions */
    exception_dsp_dump_region_data_binary(num, pESM->dynamic_regions_items, pESM->dynamic_regions);

    /* dump dsp end log */
    exception_printf("\r\nDSP%d memory dump completed.\r\n", num);

#elif (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
    exception_dsp_dump_region_data_minidump(num, pESM->static_regions_items, pESM->static_regions);

#endif /* EXCEPTION_MEMDUMP_MODE */
}

/******************************************************************************/
/*            Exception's dump processor Functions                            */
/******************************************************************************/
exception_status_t exception_dsp_dump_preprocess(int num, exception_sharemem_dsp_t *pESM)
{
#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_TEXT) || (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_BINARY)
    /* Check Exception Share Memory data */
    if(exception_dsp_packet_checkout(num, pESM) != EXCEPTION_STATUS_OK)
    {
        exception_printf("\r\nDSP%d data packet is dirty\r\n", num);
        return EXCEPTION_STATUS_ERROR;
    }

    return EXCEPTION_STATUS_OK;

#elif (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
    /* Check Exception Share Memory data */
    if((exception_dsp_packet_checkout(num, pESM) != EXCEPTION_STATUS_OK) ||
       (minidump_base_address == 0))
    {
        /* init dsp minidump base address */
        minidump_base_address_dsp = 0;
        return EXCEPTION_STATUS_ERROR;
    }
    else
    {
        /* init dsp minidump base address */
        if(num == 0)
        {
            minidump_base_address_dsp = minidump_base_address + MINIDUMP_ADDRESS_OFFSET_DSP0;
        }
        else if(num == 1)
        {
            minidump_base_address_dsp = minidump_base_address + MINIDUMP_ADDRESS_OFFSET_DSP1;
        }
        else
        {
            minidump_base_address_dsp = 0;
            return EXCEPTION_STATUS_ERROR;
        }
    }

    return EXCEPTION_STATUS_OK;

#endif /* EXCEPTION_MEMDUMP_MODE */
}

void exception_dsp_dump_postprocess(int num, exception_sharemem_dsp_t *pESM)
{
#if (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_TEXT) || (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_BINARY)
#elif (EXCEPTION_MEMDUMP_MODE == EXCEPTION_MEMDUMP_MINIDUMP)
    /* update dsp minidump header */
    if(minidump_base_address != 0)
    {
        if(minidump_base_address_dsp == 0)
        {
            minidump_header_dsp.name[0]  = 'D';
            minidump_header_dsp.name[1]  = 'S';
            minidump_header_dsp.name[2]  = 'P';
            minidump_header_dsp.name[4]  = 'M';
            minidump_header_dsp.name[5]  = 'I';
            minidump_header_dsp.name[6]  = 'N';
            minidump_header_dsp.name[7]  = 'I';
            minidump_header_dsp.name[8]  = 'D';
            minidump_header_dsp.name[9]  = 'U';
            minidump_header_dsp.name[10] = 'M';
            minidump_header_dsp.name[11] = 'P';
            minidump_header_dsp.name[12] = 'F';
            minidump_header_dsp.name[13] = 'A';
            minidump_header_dsp.name[14] = 'I';
            minidump_header_dsp.name[15] = 'L';
            minidump_header_dsp.name[16] = 'E';
            minidump_header_dsp.name[17] = 'D';
            minidump_header_dsp.name[18] = 0;
            minidump_header_dsp.name[19] = 0;
        }
        else
        {
            minidump_header_dsp.name[0]  = 'D';
            minidump_header_dsp.name[1]  = 'S';
            minidump_header_dsp.name[2]  = 'P';
            minidump_header_dsp.name[4]  = 'M';
            minidump_header_dsp.name[5]  = 'I';
            minidump_header_dsp.name[6]  = 'N';
            minidump_header_dsp.name[7]  = 'I';
            minidump_header_dsp.name[8]  = 'D';
            minidump_header_dsp.name[9]  = 'U';
            minidump_header_dsp.name[10] = 'M';
            minidump_header_dsp.name[11] = 'P';
            minidump_header_dsp.name[12] = 'S';
            minidump_header_dsp.name[13] = 'U';
            minidump_header_dsp.name[14] = 'C';
            minidump_header_dsp.name[15] = 'C';
            minidump_header_dsp.name[16] = 'E';
            minidump_header_dsp.name[17] = 'E';
            minidump_header_dsp.name[18] = 'D';
            minidump_header_dsp.name[19] = 0;
        }

        if(num == 0)
        {
            minidump_header_dsp.name[3]  = '0';
            minidump_header_dsp.length = MINIDUMP_DATA_SIZE_DSP0;

            offline_dump_region_write(OFFLINE_REGION_MINI_DUMP,
                                      minidump_base_address + MINIDUMP_ADDRESS_OFFSET_DSP0,
                                      (uint8_t *)&minidump_header_dsp,
                                      sizeof(minidump_header_dsp));
        }
        else if(num == 1)
        {
            minidump_header_dsp.name[3]  = '1';
            minidump_header_dsp.length = MINIDUMP_DATA_SIZE_DSP1;

            offline_dump_region_write(OFFLINE_REGION_MINI_DUMP,
                                      minidump_base_address + MINIDUMP_ADDRESS_OFFSET_DSP1,
                                      (uint8_t *)&minidump_header_dsp,
                                      sizeof(minidump_header_dsp));
        }
    }

#endif /* EXCEPTION_MEMDUMP_MODE */
}

/******************************************************************************/
/*            DSP0 Processor Exceptions Handlers                              */
/******************************************************************************/
void exception_alert_dsp0(void)
{
    hal_ccni_set_event(EXCEPTION_EVENT_TO_DSP0, NULL);
}

exception_slave_status_t exception_check_status_dsp0(void)
{
    uint32_t slave_status = HAL_CCNI_EVENT_STATUS_BUSY;

    if(HAL_CCNI_STATUS_OK == hal_ccni_query_event_status(EXCEPTION_EVENT_TO_DSP0, &slave_status))
    {
        if(slave_status == HAL_CCNI_EVENT_STATUS_IDLE)
        {
            /* DSP0 is ready */
            return EXCEPTION_SLAVE_STATUS_READY;
        }
        else
        {
            /* DSP0 is not ready */
            return EXCEPTION_SLAVE_STATUS_IDLE;
        }
    }

    return EXCEPTION_SLAVE_STATUS_IDLE;
}

void exception_dump_dsp0(void)
{
    exception_sharemem_dsp_t *pESM = (exception_sharemem_dsp_t *)EXCEPTION_SHARE_MEMORY_ADDRESS_DSP0;

    /* prepare dump */
    if(exception_dsp_dump_preprocess(0, pESM) != EXCEPTION_STATUS_OK)
    {
        return;
    }

    /* dump dsp context, such as core registers, time ... */
    exception_dsp_dump_context(0, pESM);

    /* dump dsp memory*/
    exception_dsp_dump_memory(0, pESM);

    /* finish the dump */
    exception_dsp_dump_postprocess(0, pESM);

}

/******************************************************************************/
/*            DSP1 Processor Exceptions Handlers                              */
/******************************************************************************/
void exception_alert_dsp1(void)
{
    hal_ccni_set_event(EXCEPTION_EVENT_TO_DSP1, NULL);
}

exception_slave_status_t exception_check_status_dsp1(void)
{
    uint32_t slave_status = HAL_CCNI_EVENT_STATUS_BUSY;

    if(HAL_CCNI_STATUS_OK == hal_ccni_query_event_status(EXCEPTION_EVENT_TO_DSP1, &slave_status))
    {
        if(slave_status == HAL_CCNI_EVENT_STATUS_IDLE)
        {
            /* DSP1 is ready */
            return EXCEPTION_SLAVE_STATUS_READY;
        }
        else
        {
            /* DSP1 is not ready */
            return EXCEPTION_SLAVE_STATUS_IDLE;
        }
    }

    return EXCEPTION_SLAVE_STATUS_IDLE;
}

void exception_dump_dsp1(void)
{
    exception_sharemem_dsp_t *pESM = (exception_sharemem_dsp_t *)EXCEPTION_SHARE_MEMORY_ADDRESS_DSP1;

    /* prepare dump */
    if(exception_dsp_dump_preprocess(1, pESM) != EXCEPTION_STATUS_OK)
    {
        return;
    }

    /* dump dsp context, such as core registers, time ... */
    exception_dsp_dump_context(1, pESM);

    /* dump dsp memory*/
    exception_dsp_dump_memory(1, pESM);

    /* finish the dump */
    exception_dsp_dump_postprocess(1, pESM);

}

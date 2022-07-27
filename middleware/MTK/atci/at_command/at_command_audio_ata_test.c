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
 
#include "at_command_audio_ata_test.h"
#include "hal_audio_internal.h"
#include <string.h>
#if defined(__GNUC__)
//#if defined(HAL_I2S_MODULE_ENABLED) || defined(HAL_AUDIO_MODULE_ENABLED)

#if 1

//#if defined(HAL_AUDIO_TEST_ENABLE)
#if 1

#define SPM_BASE (0xA2120000)

static bool audio_smt_flag = false;
void audio_test_hp_hw_sine_(uint8_t chennel)
{
    //Digital part
    *(volatile uint32_t *)0xA2030B20 = 0x000001FC;   //XO_PDN_CLRD0
    hal_gpt_delay_us(250);
  //  if (!audio_smt_flag) { // disable HW sine when smt test
        *(volatile uint32_t *)0x70000000 = 0x00000000;   //AUDIO_TOP_CON0
        *(volatile uint32_t *)0x70000010 = 0x00000001;   //AFE_DAC_CON0
        *(volatile uint32_t *)0x70000124 = 0x60606001;   //AFE_ADDA_UL_DL_CON0
        if(chennel&SMT_CH_LEFT)
            *(volatile uint32_t *)0x700001F0 = 0x04AC2AC0;   //AFE_SINEGEN_CON0
        else if(chennel&SMT_CH_RIGHT)
            *(volatile uint32_t *)0x700001F0 = 0x04AC0AC2;   //AFE_SINEGEN_CON0
        *(volatile uint32_t *)0x700001DC = 0x00000024;   //AFE_SINEGEN_CON2
        //*(volatile uint32_t *)0x700001DC = 0x00000004;   //AFE_SINEGEN_CON2
  //  }
    *(volatile uint32_t *)0x70000260 = 0x00000000;   //AFE_ADDA_PREDIS_CON0
    *(volatile uint32_t *)0x70000264 = 0x00000000;   //AFE_ADDA_PREDIS_CON1
    *(volatile uint32_t *)0x70000D40 = 0x80000000;   //AFE_ADDA_PREDIS_CON2
    *(volatile uint32_t *)0x70000D44 = 0x00000000;   //AFE_ADDA_PREDIS_CON3
    *(volatile uint32_t *)0x70000C50 = 0x0700701E;   //AFE_ADDA_DL_SDM_DCCOMP_CON
    *(volatile uint32_t *)0x7000010C = 0x00000000;   //AFE_ADDA_DL_SRC2_CON1
    *(volatile uint32_t *)0x70000108 = 0x83001801;   //AFE_ADDA_DL_SRC2_CON0
    *(volatile uint32_t *)0x70000908 = 0x00000006;   //AFUNC_AUD_CON2
    *(volatile uint32_t *)0x70000900 = 0x0000CBA1;   //AFUNC_AUD_CON0
    *(volatile uint32_t *)0x70000908 = 0x00000003;   //AFUNC_AUD_CON2
    *(volatile uint32_t *)0x70000908 = 0x0000000B;   //AFUNC_AUD_CON2
    *(volatile uint32_t *)0x70000E6C = 0x00000050;   //AFE_ANA_GAIN_MUX
    hal_gpt_delay_us(50);

    //Analog part
    *(volatile uint32_t *)0xA2070224 = 0x00003020;   //AUDDEC_ANA_CON9
    *(volatile uint32_t *)0xA2070224 = 0x00000020;   //AUDDEC_ANA_CON9
    hal_gpt_delay_us(50);
    *(volatile uint32_t *)0xA207022C = 0x00000004;   //AUDDEC_ANA_CON11
    hal_gpt_delay_us(50);
    *(volatile uint32_t *)0xA207022C = 0x00000183;   //AUDDEC_ANA_CON11
    hal_gpt_delay_us(50);
    *(volatile uint32_t *)0xA207022C = 0x00000182;   //AUDDEC_ANA_CON11
    hal_gpt_delay_us(100);
    *(volatile uint32_t *)0xA2070224 = 0x00000120;   //AUDDEC_ANA_CON9
    *(volatile uint32_t *)0xA2070224 = 0x00000320;   //AUDDEC_ANA_CON9
    *(volatile uint32_t *)0xA2070224 = 0x00000720;   //AUDDEC_ANA_CON9
*((volatile uint32_t*)(0xA2070224)) = 0x00003720;
    hal_gpt_delay_us(100);
    *(volatile uint32_t *)0xA2070228 = 0x00000010;   //AUDDEC_ANA_CON10
    *(volatile uint32_t *)0xA2070228 = 0x00000030;   //AUDDEC_ANA_CON10
    *(volatile uint32_t *)0xA2070228 = 0x00000070;   //AUDDEC_ANA_CON10
    *(volatile uint32_t *)0xA2070228 = 0x00000170;   //AUDDEC_ANA_CON10
    *(volatile uint32_t *)0xA2070228 = 0x00001170;   //AUDDEC_ANA_CON10
    *(volatile uint32_t *)0xA2070228 = 0x00009170;   //AUDDEC_ANA_CON10
    hal_gpt_delay_us(100);
    *(volatile uint32_t *)0x70000F50 = 0x00000000;   //ZCD_CON0
    *(volatile uint32_t *)0xA2070204 = 0x00003000;   //AUDDEC_ANA_CON1
    *(volatile uint32_t *)0xA207020C = 0x00000100;   //AUDDEC_ANA_CON3
    *(volatile uint32_t *)0xA2070220 = 0x0000003F;   //AUDDEC_ANA_CON8
    *(volatile uint32_t *)0xA207021C = 0x00000103;   //AUDDEC_ANA_CON7
    *(volatile uint32_t *)0xA2070220 = 0x00000000;   //AUDDEC_ANA_CON8
    *(volatile uint32_t *)0xA2070228 = 0x00009173;   //AUDDEC_ANA_CON10
    *(volatile uint32_t *)0xA2070214 = 0x00000000;   //AUDDEC_ANA_CON5
    if(chennel&SMT_CH_LEFT)
        *(volatile uint32_t *)0x70000F58 = 0x00002C0B;   //ZCD_CON2
    else if(chennel&SMT_CH_RIGHT)
        *(volatile uint32_t *)0x70000F58 = 0x00000B2C;
    *(volatile uint32_t *)0x70000EC4 = 0x0E021188;   //AFE_CLASSG_LPSRCH_CFG0
    *(volatile uint32_t *)0xA2070208 = 0x00000024;   //AUDDEC_ANA_CON2
    *(volatile uint32_t *)0xA207020C = 0x00000300;     //AUDDEC_ANA_CON3
    *(volatile uint32_t *)0xA207020C = 0x00000300;     //AUDDEC_ANA_CON3
    *(volatile uint32_t *)0xA2070208 = 0x000001A4;     //AUDDEC_ANA_CON2
    *(volatile uint32_t *)0xA2070208 = 0x000031A4;     //AUDDEC_ANA_CON2
    *(volatile uint32_t *)0xA2070204 = 0x0000300C;     //AUDDEC_ANA_CON1
    *(volatile uint32_t *)0xA2070204 = 0x000030CC;     //AUDDEC_ANA_CON1
    *(volatile uint32_t *)0xA2070204 = 0x000030FC;     //AUDDEC_ANA_CON1
    *(volatile uint32_t *)0xA2070208 = 0x00003DA4;     //AUDDEC_ANA_CON2
    *(volatile uint32_t *)0xA2070208 = 0x0000FDA4;     //AUDDEC_ANA_CON2
    hal_gpt_delay_us(50);
    *(volatile uint32_t *)0xA2070208 = 0x0000CDA4;     //AUDDEC_ANA_CON2
    *(volatile uint32_t *)0xA2070204 = 0x000030FF;     //AUDDEC_ANA_CON1
    hal_gpt_delay_us(50);
    *(volatile uint32_t *)0x70000EC4 = 0x0E0A1188;   //AFE_CLASSG_LPSRCH_CFG0
    hal_gpt_delay_us(600);
    *(volatile uint32_t *)0x70000EC4 = 0x0E121188;   //AFE_CLASSG_LPSRCH_CFG0
    hal_gpt_delay_us(600);
    *(volatile uint32_t *)0x70000EC4 = 0x0E1A1188;   //AFE_CLASSG_LPSRCH_CFG0
    hal_gpt_delay_us(600);
    *(volatile uint32_t *)0x70000EC4 = 0x0E221188;   //AFE_CLASSG_LPSRCH_CFG0
    hal_gpt_delay_us(600);
    *(volatile uint32_t *)0x70000EC4 = 0x0E2A1188;   //AFE_CLASSG_LPSRCH_CFG0
    hal_gpt_delay_us(600);
    *(volatile uint32_t *)0x70000EC4 = 0x0E321188;   //AFE_CLASSG_LPSRCH_CFG0
    hal_gpt_delay_us(600);
    *(volatile uint32_t *)0x70000EC4 = 0x0E3A1188;   //AFE_CLASSG_LPSRCH_CFG0
    hal_gpt_delay_us(600);
    *(volatile uint32_t *)0xA207020C = 0x00000311;   //AUDDEC_ANA_CON3
    hal_gpt_delay_us(600);
    *(volatile uint32_t *)0xA207020C = 0x000003FF;   //AUDDEC_ANA_CON3
    hal_gpt_delay_us(600);
    *(volatile uint32_t *)0xA2070208 = 0x0000CC24;   //AUDDEC_ANA_CON2
    hal_gpt_delay_us(100);
    *(volatile uint32_t *)0x70000F58 = 0x00000AEB;   //ZCD_CON2
    *(volatile uint32_t *)0x70000F58 = 0x00000208;   //ZCD_CON2                               // Modify here to mute left or right
    *(volatile uint32_t *)0xA2070204 = 0x000030F3;   //AUDDEC_ANA_CON1
    *(volatile uint32_t *)0xA2070208 = 0x0000C024;   //AUDDEC_ANA_CON2
    *(volatile uint32_t *)0xA2070228 = 0x00009170;   //AUDDEC_ANA_CON10
    hal_gpt_delay_us(100);
    *((volatile uint32_t*)(0xA2070224)) = 0x00003721;
    *(volatile uint32_t *)0xA2070230 = 0x00000100;   //AUDDEC_ANA_CON12
    hal_gpt_delay_us(100);
    *(volatile uint32_t *)0xA2070200 = 0x00000020;   //AUDDEC_ANA_CON0
    *(volatile uint32_t *)0xA2070200 = 0x00000120;   //AUDDEC_ANA_CON0
    *(volatile uint32_t *)0xA2070200 = 0x0000012F;   //AUDDEC_ANA_CON0
    hal_gpt_delay_us(100);
    *(volatile uint32_t *)0xA2070204 = 0x000035F3;   //AUDDEC_ANA_CON1
    hal_gpt_delay_us(100);
    *(volatile uint32_t *)0xA207020C = 0x000001FF;   //AUDDEC_ANA_CON3
}

void audio_mtcmos_on(void)
{

    uint32_t audio_control;
    /* AUDIO MTCMOS ON */
    *((volatile uint32_t*)(SPM_BASE + 0x020C)) = 0x16;
    hal_gpt_delay_us(1);
    *((volatile uint32_t*)(SPM_BASE + 0x020C)) = 0x1E;
    hal_gpt_delay_us(1);
    audio_control = *((volatile uint32_t*)(SPM_BASE + 0x0700));
    *((volatile uint32_t*)(SPM_BASE + 0x0700)) = audio_control & 0xFF07;
    hal_gpt_delay_us(1);
    *((volatile uint32_t*)(SPM_BASE + 0x0700)) = audio_control & 0xFF03;
    hal_gpt_delay_us(1);
    *((volatile uint32_t*)(SPM_BASE + 0x0700)) = audio_control & 0xFF01;
    hal_gpt_delay_us(1);
    *((volatile uint32_t*)(SPM_BASE + 0x0700)) = audio_control & 0xFF00;
    hal_gpt_delay_us(1);
    *((volatile uint32_t*)(SPM_BASE + 0x0700)) = 0;
    hal_gpt_delay_us(1);
    *((volatile uint32_t*)(SPM_BASE + 0x020C)) = 0x0E;
    hal_gpt_delay_us(1);
    *((volatile uint32_t*)(SPM_BASE + 0x020C)) = 0x0C;
    hal_gpt_delay_us(1);
    *((volatile uint32_t*)(SPM_BASE + 0x020C)) = 0x1C;
    hal_gpt_delay_us(1);
    *((volatile uint32_t*)(SPM_BASE + 0x020C)) = 0x1D;
    hal_gpt_delay_us(1);
    *((volatile uint32_t*)(SPM_BASE + 0x020C)) = 0x0D;
    hal_gpt_delay_us(1);
    *((volatile uint32_t*)(0xA2290020)) = 0; /* INFRA_CFG_MTCMOS2[0] : audiosys_prot_en */
    while((*((volatile uint32_t*)(0xA2290020))&0x100) != 0); /* INFRA_CFG_MTCMOS2[8] : audiosys_prot_rdy */
    *((volatile uint32_t*)(0xA2030B20)) = 0xFFFFFFFF; // clock CG all on
}


static void audio_turn_off_afe(void)
{
    //Digital part
    *(volatile uint32_t *)0x70000900 = 0x0000CB20;   //AFUNC_AUD_CON0
    *(volatile uint32_t *)0x70000908 = 0x00000000;   //AFUNC_AUD_CON2
    *(volatile uint32_t *)0x70000108 = 0x00000000;   //AFE_ADDA_DL_SRC2_CON0
    *(volatile uint32_t *)0x700001F0 = 0x00000000;   //AFE_SINEGEN_CON0
    *(volatile uint32_t *)0x70000124 = 0x60606000;   //AFE_ADDA_UL_DL_CON0
    *(volatile uint32_t *)0x70000010 = 0x00000000;   //AFE_DAC_CON0
    *(volatile uint32_t *)0x70000000 = 0x000CC000;   //AUDIO_TOP_CON0
    *(volatile uint32_t *)0xA2030B20 = 0x00000000;   //XO_PDN_CLRD0
    //Analog part
    *(volatile uint32_t *)0xA207020C = 0x000003FF;   //AUDDEC_ANA_CON3
    *(volatile uint32_t *)0xA2070228 = 0x00009173;   //AUDDEC_ANA_CON10
    *(volatile uint32_t *)0xA2070204 = 0x000030F3;   //AUDDEC_ANA_CON1
    *(volatile uint32_t *)0xA2070200 = 0x00000060;   //AUDDEC_ANA_CON0
    *(volatile uint32_t *)0xA2070230 = 0x00000000;   //AUDDEC_ANA_CON12
    *(volatile uint32_t *)0xA2070224 = 0x00000720;   //AUDDEC_ANA_CON9
    *(volatile uint32_t *)0xA2070208 = 0x0000CC24;   //AUDDEC_ANA_CON2
    *(volatile uint32_t *)0xA2070204 = 0x000030FF;   //AUDDEC_ANA_CON1
    *(volatile uint32_t *)0xA2070204 = 0x000030FC;   //AUDDEC_ANA_CON1
    *(volatile uint32_t *)0xA2070208 = 0x0000FDA4;   //AUDDEC_ANA_CON2
    hal_gpt_delay_us(50);
    *(volatile uint32_t *)0xA2070208 = 0x00003DA4;   //AUDDEC_ANA_CON2
    *(volatile uint32_t *)0xA2070208 = 0x000031A4;   //AUDDEC_ANA_CON2
    *(volatile uint32_t *)0xA2070204 = 0x000030CC;   //AUDDEC_ANA_CON1
    *(volatile uint32_t *)0xA2070204 = 0x0000300C;   //AUDDEC_ANA_CON1
    *(volatile uint32_t *)0xA2070204 = 0x00003000;   //AUDDEC_ANA_CON1
    *(volatile uint32_t *)0xA2070208 = 0x0000C1A4;   //AUDDEC_ANA_CON2
    *(volatile uint32_t *)0xA2070208 = 0x0000C024;   //AUDDEC_ANA_CON2
    *(volatile uint32_t *)0xA2070228 = 0x00009170;   //AUDDEC_ANA_CON10
    *(volatile uint32_t *)0xA207020C = 0x00000100;   //AUDDEC_ANA_CON3
    *(volatile uint32_t *)0x70000F50 = 0x00000000;   //ZCD_CON0
    *(volatile uint32_t *)0xA2070220 = 0x00008000;   //AUDDEC_ANA_CON8
    hal_gpt_delay_us(100);
    *(volatile uint32_t *)0xA2070228 = 0x00009160;   //AUDDEC_ANA_CON10
    *(volatile uint32_t *)0xA2070228 = 0x00009140;   //AUDDEC_ANA_CON10
    *(volatile uint32_t *)0xA2070228 = 0x00009100;   //AUDDEC_ANA_CON10
    hal_gpt_delay_us(100);
    *(volatile uint32_t *)0xA2070224 = 0x00000320;   //AUDDEC_ANA_CON9
    *(volatile uint32_t *)0xA2070224 = 0x00000120;   //AUDDEC_ANA_CON9
    *(volatile uint32_t *)0xA2070224 = 0x00000020;   //AUDDEC_ANA_CON9
    hal_gpt_delay_us(100);
    *(volatile uint32_t *)0xA207022C = 0x00000004;   //AUDDEC_ANA_CON11
    hal_gpt_delay_us(50);
    *(volatile uint32_t *)0xA2070224 = 0x00000030;   //AUDDEC_ANA_CON9
    hal_gpt_delay_us(100);
}

void audio_test_hp_hw_sine(uint8_t chennel)
{
    //Digital part
    *(volatile uint32_t *)0xA2030B20 = 0x000001FC;   //XO_PDN_CLRD0
    hal_gpt_delay_us(250);
    *(volatile uint32_t *)0x70000000 = 0x20004000;//AUDIO_TOP_CON0
    *(volatile uint32_t *)0x70000010 = 0x00000001;   //AFE_DAC_CON0
    *(volatile uint32_t *)0x700001F0 = 0x04AC2AC2;   //AFE_SINEGEN_CON0
    *(volatile uint32_t *)0x700001DC = 0x00000024;   //AFE_SINEGEN_CON2
    //*(volatile uint32_t *)0x700001DC = 0x00000004;   //AFE_SINEGEN_CON2
    *(volatile uint32_t *)0x70000260 = 0x00000000;   //AFE_ADDA_PREDIS_CON0
    *(volatile uint32_t *)0x70000264 = 0x00000000;   //AFE_ADDA_PREDIS_CON1
    *(volatile uint32_t *)0x70000D40 = 0x80000000;   //AFE_ADDA_PREDIS_CON2
    *(volatile uint32_t *)0x70000D44 = 0x00000000;   //AFE_ADDA_PREDIS_CON3
    *(volatile uint32_t *)0x70000C50 = 0x0700701E;   //AFE_ADDA_DL_SDM_DCCOMP_CON
    *(volatile uint32_t *)0x7000010C = 0x00000000;   //AFE_ADDA_DL_SRC2_CON1
    *(volatile uint32_t *)0x70000108 = 0x83001801;   //AFE_ADDA_DL_SRC2_CON0
    *(volatile uint32_t *)0x70000908 = 0x00000006;   //AFUNC_AUD_CON2
    *(volatile uint32_t *)0x70000900 = 0x0000CBA1;   //AFUNC_AUD_CON0
    *(volatile uint32_t *)0x70000908 = 0x00000003;   //AFUNC_AUD_CON2
    *(volatile uint32_t *)0x70000908 = 0x0000000B;   //AFUNC_AUD_CON2
    *(volatile uint32_t *)0x70000E6C = 0x00000050;   //AFE_ANA_GAIN_MUX
    hal_gpt_delay_us(50);
    //Analog part
    *(volatile uint32_t *)0xA2070224 = 0x00003020;   //AUDDEC_ANA_CON9
    *(volatile uint32_t *)0xA2070224 = 0x00000020;   //AUDDEC_ANA_CON9
    hal_gpt_delay_us(50);
    *(volatile uint32_t *)0xA207022C = 0x00000004;   //AUDDEC_ANA_CON11
    hal_gpt_delay_us(50);
    *(volatile uint32_t *)0xA207022C = 0x00000183;   //AUDDEC_ANA_CON11
    hal_gpt_delay_us(50);
    *(volatile uint32_t *)0xA207022C = 0x00000182;   //AUDDEC_ANA_CON11
    hal_gpt_delay_us(100);
    *(volatile uint32_t *)0xA2070224 = 0x00000120;   //AUDDEC_ANA_CON9
    *(volatile uint32_t *)0xA2070224 = 0x00000320;   //AUDDEC_ANA_CON9
    *(volatile uint32_t *)0xA2070224 = 0x00000720;   //AUDDEC_ANA_CON9
*((volatile uint32_t*)(0xA2070224)) = 0x00003720;
    hal_gpt_delay_us(100);
    *(volatile uint32_t *)0xA2070228 = 0x00000010;   //AUDDEC_ANA_CON10
    *(volatile uint32_t *)0xA2070228 = 0x00000030;   //AUDDEC_ANA_CON10
    *(volatile uint32_t *)0xA2070228 = 0x00000070;   //AUDDEC_ANA_CON10
    *(volatile uint32_t *)0xA2070228 = 0x00000170;   //AUDDEC_ANA_CON10
    *(volatile uint32_t *)0xA2070228 = 0x00001170;   //AUDDEC_ANA_CON10
    *(volatile uint32_t *)0xA2070228 = 0x00009170;   //AUDDEC_ANA_CON10
    hal_gpt_delay_us(100);
    *(volatile uint32_t *)0x70000F50 = 0x00000000;   //ZCD_CON0
    *(volatile uint32_t *)0xA2070204 = 0x00003000;   //AUDDEC_ANA_CON1
    *(volatile uint32_t *)0xA207020C = 0x00000100;   //AUDDEC_ANA_CON3
    *(volatile uint32_t *)0xA2070220 = 0x0000003F;   //AUDDEC_ANA_CON8
    *(volatile uint32_t *)0xA207021C = 0x00000103;   //AUDDEC_ANA_CON7
    *(volatile uint32_t *)0xA2070220 = 0x00000000;   //AUDDEC_ANA_CON8
    *(volatile uint32_t *)0xA2070228 = 0x00009173;   //AUDDEC_ANA_CON10
    *(volatile uint32_t *)0xA2070214 = 0x00000000;   //AUDDEC_ANA_CON5
    if(chennel&SMT_CH_LEFT)
        *(volatile uint32_t *)0x70000F58 = 0x00003E0B;   //ZCD_CON2
    else if(chennel&SMT_CH_RIGHT)
        *(volatile uint32_t *)0x70000F58 = 0x00000B3E;
    *(volatile uint32_t *)0x70000EC4 = 0x0E021188;   //AFE_CLASSG_LPSRCH_CFG0
    *(volatile uint32_t *)0xA2070208 = 0x00000024;   //AUDDEC_ANA_CON2
    *(volatile uint32_t *)0xA207020C = 0x00000300;     //AUDDEC_ANA_CON3
    *(volatile uint32_t *)0xA207020C = 0x00000300;     //AUDDEC_ANA_CON3
    *(volatile uint32_t *)0xA2070208 = 0x000001A4;     //AUDDEC_ANA_CON2
    *(volatile uint32_t *)0xA2070208 = 0x000031A4;     //AUDDEC_ANA_CON2
    *(volatile uint32_t *)0xA2070204 = 0x0000300C;     //AUDDEC_ANA_CON1
    *(volatile uint32_t *)0xA2070204 = 0x000030CC;     //AUDDEC_ANA_CON1
    *(volatile uint32_t *)0xA2070204 = 0x000030FC;     //AUDDEC_ANA_CON1
    *(volatile uint32_t *)0xA2070208 = 0x00003DA4;     //AUDDEC_ANA_CON2
    *(volatile uint32_t *)0xA2070208 = 0x0000FDA4;     //AUDDEC_ANA_CON2
    hal_gpt_delay_us(50);
    *(volatile uint32_t *)0xA2070208 = 0x0000CDA4;     //AUDDEC_ANA_CON2
    *(volatile uint32_t *)0xA2070204 = 0x000030FF;     //AUDDEC_ANA_CON1
    hal_gpt_delay_us(50);

    *(volatile uint32_t *)0x70000EC4 = 0x0E0A1188;   //AFE_CLASSG_LPSRCH_CFG0
    hal_gpt_delay_us(600);
    *(volatile uint32_t *)0x70000EC4 = 0x0E121188;   //AFE_CLASSG_LPSRCH_CFG0
    hal_gpt_delay_us(600);
    *(volatile uint32_t *)0x70000EC4 = 0x0E1A1188;   //AFE_CLASSG_LPSRCH_CFG0
    hal_gpt_delay_us(600);
    *(volatile uint32_t *)0x70000EC4 = 0x0E221188;   //AFE_CLASSG_LPSRCH_CFG0
    hal_gpt_delay_us(600);
    *(volatile uint32_t *)0x70000EC4 = 0x0E2A1188;   //AFE_CLASSG_LPSRCH_CFG0
    hal_gpt_delay_us(600);
    *(volatile uint32_t *)0x70000EC4 = 0x0E321188;   //AFE_CLASSG_LPSRCH_CFG0
    hal_gpt_delay_us(600);
    *(volatile uint32_t *)0x70000EC4 = 0x0E3A1188;   //AFE_CLASSG_LPSRCH_CFG0
    hal_gpt_delay_us(600);
    *(volatile uint32_t *)0xA207020C = 0x00000311;   //AUDDEC_ANA_CON3
    hal_gpt_delay_us(600);
    *(volatile uint32_t *)0xA207020C = 0x000003FF;   //AUDDEC_ANA_CON3
    hal_gpt_delay_us(600);
    *(volatile uint32_t *)0xA2070208 = 0x0000CC24;   //AUDDEC_ANA_CON2
    hal_gpt_delay_us(100);
    *(volatile uint32_t *)0x70000F58 = 0x00000AEB;   //ZCD_CON2
    *(volatile uint32_t *)0x70000F58 = 0x00000208;   //ZCD_CON2                               // Modify here to mute left or right
    *(volatile uint32_t *)0xA2070204 = 0x000030F3;   //AUDDEC_ANA_CON1
    *(volatile uint32_t *)0xA2070208 = 0x0000C024;   //AUDDEC_ANA_CON2
    *(volatile uint32_t *)0xA2070228 = 0x00009170;   //AUDDEC_ANA_CON10
    hal_gpt_delay_us(100);
*((volatile uint32_t*)(0xA2070224)) = 0x00003721;
    *(volatile uint32_t *)0xA2070230 = 0x00000100;   //AUDDEC_ANA_CON12
    hal_gpt_delay_us(100);
    *(volatile uint32_t *)0xA2070200 = 0x00000020;   //AUDDEC_ANA_CON0
    *(volatile uint32_t *)0xA2070200 = 0x00000120;   //AUDDEC_ANA_CON0
    *(volatile uint32_t *)0xA2070200 = 0x0000012F;   //AUDDEC_ANA_CON0
    hal_gpt_delay_us(100);
    *(volatile uint32_t *)0xA2070204 = 0x000035F3;   //AUDDEC_ANA_CON1
    hal_gpt_delay_us(100);
    *(volatile uint32_t *)0xA207020C = 0x000001FF;   //AUDDEC_ANA_CON3

}


void hal_audio_init_stream_buf(fft_buf_t *fft_bufs){
    if(!fft_bufs)
        return ;
    fft_bufs->cpyIdx = &fft_bufs->bitstream_buf[0];
    memset(fft_bufs->cpyIdx,0,FFT_BUFFER_SIZE << 1);
}

void audio_smt_test_pure_on_off(bool enable,smt_ch chennel)
{

    if (enable) {
        uint32_t interconnection = 0;
        audio_smt_flag = true;
        audio_mtcmos_on();

        interconnection = *(volatile uint32_t *)0x70000438;
        *(volatile uint32_t *)0x70000438 = interconnection | 0x0000100; // I8->O8
        interconnection = *(volatile uint32_t *)0x70000440;
        *(volatile uint32_t *)0x70000440 = interconnection | 0x0000200; // I9->O9
        audio_test_hp_hw_sine_(chennel);
        pmu_power_enable_6388(PMU_BUCK_VAUD18,PMU_ON);

    } else {
          audio_turn_off_afe();
          *(volatile uint32_t*)0x70000114 = 0x00000000;   //AFE_ADDA_UL_SRC_CON0
          audio_smt_flag = false;
    }
}


#endif /* defined(HAL_AUDIO_TEST_ENABLE) */
#endif /* defined(HAL_I2S_MODULE_ENABLED) || defined(HAL_AUDIO_MODULE_ENABLED) */
#endif /*defined(__GNUC__)*/

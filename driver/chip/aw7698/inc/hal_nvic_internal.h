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
 
#ifndef _HAL_NVIC_INTERNAL_H_
#define _HAL_NVIC_INTERNAL_H_
#include "hal_nvic.h"

#ifdef HAL_NVIC_MODULE_ENABLED
/* Please refer to configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY= 4 in freertos_config.h */
#define FREERTOS_ISR_API_PRIORITY  (4)          /* please keep the setting */
/* DEFAULT_IRQ_PRIORITY is from FREERTOS_ISR_API_PRIORITY =  4  to  configKERNEL_INTERRUPT_PRIORITY -1 = (0xFF - 1)
    as pend_sv should be lowest irq priority to avoid unneccessary freert_os schedule */
#define DEFAULT_IRQ_PRIORITY  (FREERTOS_ISR_API_PRIORITY + 1)     /* please keep the setting */

#define RESERVED_IRQ_PRIORITY   ((1<<__NVIC_PRIO_BITS) - 1)
#define DMA_IRQ_PRIORITY        (DEFAULT_IRQ_PRIORITY)     
#define SPI_MST_IRQ_PRIORITY    (DEFAULT_IRQ_PRIORITY + 1) 
#define SPI_SLV_IRQ_PRIORITY    (DEFAULT_IRQ_PRIORITY + 2) 
#define SDIO_SLV_IRQ_PRIORITY   (DEFAULT_IRQ_PRIORITY + 3) 
#define SDIO_MST_IRQ_PRIORITY   (DEFAULT_IRQ_PRIORITY + 4) 
#define TRNG_IRQ_PRIORITY       (DEFAULT_IRQ_PRIORITY + 5) 
#define CRYPTO_IRQ_PRIORITY     (DEFAULT_IRQ_PRIORITY + 6) 
#define UART0_IRQ_PRIORITY      (DEFAULT_IRQ_PRIORITY + 7) 
#define UART1_IRQ_PRIORITY      (DEFAULT_IRQ_PRIORITY + 8) 
#define UART2_IRQ_PRIORITY      (DEFAULT_IRQ_PRIORITY + 9) 
#define UART3_IRQ_PRIORITY      (DEFAULT_IRQ_PRIORITY + 10)
#define I2S_IRQ_PRIORITY        (DEFAULT_IRQ_PRIORITY + 11)
#define I2C0_IRQ_PRIORITY       (DEFAULT_IRQ_PRIORITY + 12)
#define I2C1_IRQ_PRIORITY       (DEFAULT_IRQ_PRIORITY + 13)
#define RTC_IRQ_PRIORITY        (DEFAULT_IRQ_PRIORITY + 14)
#define GPTimer_IRQ_PRIORITY    (DEFAULT_IRQ_PRIORITY + 15)
#define SPM_IRQ_PRIORITY        (DEFAULT_IRQ_PRIORITY + 16)
/*set RGU interrupt highest : this would be benifit for system hang issue check */
#define RGU_IRQ_PRIORITY        (0)
#define PMU_DIG_IRQ_PRIORITY    (DEFAULT_IRQ_PRIORITY + 18)
#define EINT_IRQ_PRIORITY       (DEFAULT_IRQ_PRIORITY + 19)
#define SFC_IRQ_PRIORITY        (DEFAULT_IRQ_PRIORITY + 20)
#define BTIF_IRQ_PRIORITY       (DEFAULT_IRQ_PRIORITY + 21)
#define CONNSYS0_IRQ_PRIORITY   (DEFAULT_IRQ_PRIORITY + 22)
#define CONNSYS1_IRQ_PRIORITY   (DEFAULT_IRQ_PRIORITY + 23)
//RESERVED IRQ
//RESERVED IRQ
#define SW_ISR0_IRQ_PRIORITY    (DEFAULT_IRQ_PRIORITY)
#define SW_ISR1_IRQ_PRIORITY    (DEFAULT_IRQ_PRIORITY + 24)
#define SW_ISR2_IRQ_PRIORITY    (DEFAULT_IRQ_PRIORITY + 25)
#define SW_ISR3_IRQ_PRIORITY    (DEFAULT_IRQ_PRIORITY + 26)
//RESERVED IRQ
//RESERVED IRQ


extern uint32_t save_and_set_interrupt_mask(void);
extern void restore_interrupt_mask(uint32_t mask);
extern hal_nvic_status_t nvic_irq_software_trigger(hal_nvic_irq_t irq_number);
uint32_t hal_nvic_query_exception_number(void);
void nvic_mask_all_interrupt(void);
void nvic_unmask_all_interrupt(void);
void nvic_clear_all_pending_interrupt(void);

#endif //HAL_NVIC_MODULE_ENABLED
#endif //_HAL_NVIC_INTERNAL_H_


/*
    FreeRTOS V8.2.0 - Copyright (C) 2015 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the ARM CM4F port.
 *----------------------------------------------------------*/

#include "FreeRTOS.h"
#include "port_tick.h"
#include "hal_clock.h"
#include "hal_nvic.h"
#include "timers.h"

#if configUSE_TICKLESS_IDLE == 2
#include "task.h"
#include "memory_attribute.h"
#include "hal_sleep_manager.h"
#include "hal_sleep_manager_internal.h"
#include "hal_sleep_manager_platform.h"
#include "core_cm4.h"
#include "hal_gpt.h"
#include "hal_rtc.h"
#include "hal_log.h"
#include "hal_eint.h"
#include "hal_wdt.h"
#endif

#define MaximumIdleTime 10  //ms
#define DEEP_SLEEP_HW_WAKEUP_TIME 2
#define DEEP_SLEEP_SW_BACKUP_RESTORE_TIME 2

//#define TICKLESS_DEEBUG_ENABLE
#ifdef  TICKLESS_DEEBUG_ENABLE
#define log_debug(_message,...) printf(_message, ##__VA_ARGS__)
#else
#define log_debug(_message,...)
#endif

#if configUSE_TICKLESS_IDLE != 0
/*
 * The number of OS GPT increments that make up one tick period.
 */
static uint32_t ulTimerCountsForOneTick = 0;

/*
 * The maximum number of tick periods that can be suppressed is limited by the
 * 32 bit resolution of the OS GPT timer.
 */
static uint32_t xMaximumPossibleSuppressedTicks = 0;
#endif

#if configUSE_TICKLESS_IDLE == 2
static uint32_t TimeStampSystick, TimeStampCounter;
void tickless_log_timestamp(void);
static void get_rtc_real_clock_freq(void);
TimerHandle_t timestamp_timer = NULL;
float RTC_Freq; /* RTC 32.768KHz Freq*/
static int32_t timestamp_diff = 0, skip_time = 0;;
static long unsigned int before_idle_time;

#ifdef  SLEEP_MANAGEMENT_DEBUG_ENABLE
extern uint32_t eint_get_status(void);
uint32_t wakeup_eint;
#endif

void os_gpt0_pause(void);
void os_gpt0_resume(bool update, uint32_t new_compare);

void doIdelSystickCalibration(uint32_t maxSystickCompensation)
{
    static long unsigned int after_idle_time, sleep_time = 0;
    static uint32_t nowCount, nowTick, skip_time = 0;
    static uint32_t ulCompleteTickPeriods, ulCompleteTickPeriods_timestamp, ulCompletedCountDecrements, ulCompletedTickDecrements;
    
    /* calculate time to jump */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &nowCount);
    nowTick = ((uint32_t)xTaskGetTickCount() * (1000 / configTICK_RATE_HZ));

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&after_idle_time);

    if (after_idle_time >= before_idle_time) {
        sleep_time = after_idle_time - before_idle_time;
    } else {
        sleep_time = after_idle_time + (0xFFFFFFFF - before_idle_time);
    }

    ulCompleteTickPeriods = (sleep_time / 1000) / (1000 / configTICK_RATE_HZ);

    /* get counter distance from last record */
    if (nowCount >= TimeStampCounter) {
        ulCompletedCountDecrements = nowCount - TimeStampCounter;
    } else {
        ulCompletedCountDecrements = nowCount + (0xFFFFFFFF - TimeStampCounter);
    }
    /* get ostick distance from last record */
    if (nowTick >= TimeStampSystick) {
        ulCompletedTickDecrements = nowTick - TimeStampSystick;
    } else {
        ulCompletedTickDecrements = nowTick + (0xFFFFFFFF - TimeStampSystick);
    }
    /* get counter distance for this sleep */
    ulCompletedCountDecrements = (unsigned int)(((float)ulCompletedCountDecrements) - ((float)ulCompletedTickDecrements * RTC_Freq));
    /* calculate ticks for jumping */
    ulCompleteTickPeriods_timestamp = ((unsigned int)(((float)ulCompletedCountDecrements) / RTC_Freq)) / ((1000 / configTICK_RATE_HZ));

    if (skip_time > 0) {
        skip_time = 0;
        ulCompleteTickPeriods++;
    }

    if (ulCompleteTickPeriods_timestamp > ulCompleteTickPeriods) {
        ulCompleteTickPeriods++;
    } else {
        if ((ulCompleteTickPeriods_timestamp < ulCompleteTickPeriods) && (ulCompleteTickPeriods > 2)) {
            ulCompleteTickPeriods--;
        }
    }
    /* Limit OS Tick Compensation Value */
    if (ulCompleteTickPeriods >= (maxSystickCompensation)) {
        ulCompleteTickPeriods = maxSystickCompensation;
    }
    timestamp_diff = ulCompleteTickPeriods - ulCompleteTickPeriods_timestamp;

    vTaskStepTick(ulCompleteTickPeriods);
    return;
}

void doSleepSystickCalibration(uint32_t maxSystickCompensation)
{
    static uint32_t ulCompleteTickPeriods, ulCompletedCountDecrements, ulCompletedTickDecrements;
    static uint32_t nowCount, nowTick;

    /* calculate time(ostick) to jump */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &nowCount);
    nowTick = ((uint32_t)xTaskGetTickCount() * (1000 / configTICK_RATE_HZ));
    /* get counter distance from last record */
    if (nowCount >= TimeStampCounter) {
        ulCompletedCountDecrements = nowCount - TimeStampCounter;
    } else {
        ulCompletedCountDecrements = nowCount + (0xFFFFFFFF - TimeStampCounter);
    }
    /* get ostick distance from last record */
    if (nowTick >= TimeStampSystick) {
        ulCompletedTickDecrements = nowTick - TimeStampSystick;
    } else {
        ulCompletedTickDecrements = nowTick + (0xFFFFFFFF - TimeStampSystick);
    }
    /* get counter distance for this sleep */
    ulCompletedCountDecrements = (unsigned int)(((float)ulCompletedCountDecrements) - ((float)ulCompletedTickDecrements * RTC_Freq));
    /* calculate ticks for jumping */
    ulCompleteTickPeriods = ((unsigned int)(((float)ulCompletedCountDecrements) / RTC_Freq)) / ((1000 / configTICK_RATE_HZ));

    /* Limit OS Tick Compensation Value */
    if (ulCompleteTickPeriods > (maxSystickCompensation - 1)) {
        ulCompleteTickPeriods = maxSystickCompensation - 1;
    }

    vTaskStepTick(ulCompleteTickPeriods);    
    log_debug("nowCount=%u\r\n"  , (unsigned int)nowCount);
    log_debug("nowTick=%u\r\n"  , (unsigned int)nowTick);
    log_debug("TimeStampCounter=%u\r\n"  , (unsigned int)TimeStampCounter);
    log_debug("TimeStampSystick=%u\r\n"  , (unsigned int)TimeStampSystick);    
    log_debug("CTP=%u\r\n"  , (unsigned int)ulCompleteTickPeriods);
    return;    
}

void AST_vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
    volatile static unsigned int ulAST_Reload_ms;
    
    __asm volatile("cpsid i");

    /* Calculate total idle time to ms */
    ulAST_Reload_ms = (xExpectedIdleTime - 1) * (1000 / configTICK_RATE_HZ);
    ulAST_Reload_ms = ulAST_Reload_ms - DEEP_SLEEP_SW_BACKUP_RESTORE_TIME - DEEP_SLEEP_HW_WAKEUP_TIME;

    if (eTaskConfirmSleepModeStatus() == eAbortSleep) {
        /* Restart OS GPT. */
        os_gpt0_resume(false, 0);
        /* Re-enable interrupts */
        __asm volatile("cpsie i");
        return;
    } else {    
        /* Enter Sleep mode */
        if (ulAST_Reload_ms > 0) {
            hal_sleep_manager_set_sleep_time((uint32_t)ulAST_Reload_ms);
            hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_SLEEP);
        } 

        /* Calculate and Calibration Sleep Time to OS Tick */
        doSleepSystickCalibration(xExpectedIdleTime);
        
        /* Restart OS GPT. */
        os_gpt0_resume(false, 0);

#ifdef  SLEEP_MANAGEMENT_DEBUG_ENABLE
        wakeup_eint = eint_get_status();
#endif
        /* Re-enable interrupts */
        __asm volatile("cpsie i");

        sleep_management_dump_debug_log(SLEEP_MANAGEMENT_DEBUG_LOG_DUMP);
        
#ifdef  SLEEP_MANAGEMENT_DEBUG_ENABLE
#ifdef  SLEEP_MANAGEMENT_DEBUG_SLEEP_WAKEUP_LOG_ENABLE
        sleep_management_dump_wakeup_source(sleep_management_status.wakeup_source, wakeup_eint);
#endif
#endif
        log_debug("\r\nEIT=%u\r\n"  , (unsigned int)xExpectedIdleTime);
        log_debug("RL=%u\r\n"       , (unsigned int)ulAST_Reload_ms);
    }
}

void tickless_handler(uint32_t xExpectedIdleTime)
{
    static long unsigned int ulReloadValue;
    
    /* Enter a critical section but don't use the taskENTER_CRITICAL()
    method as that will mask interrupts that should exit sleep mode. */
    __asm volatile("cpsid i");

    /* Stop the OS GPT momentarily.  */
    os_gpt0_pause();
    
    /* Check Log FIFO size
    unsigned int send_space;
    send_space = hal_uart_get_available_send_space(HAL_UART_0);
    if(send_space == 8192){
        if(hal_sleep_manager_is_sleep_handle_alive(16) == true) {
            sleep_management_lock_sleep(UNLOCK_SLEEP,16);
        }              
    }*/
    
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&before_idle_time);

    if ((xExpectedIdleTime > (MaximumIdleTime / (1000 / configTICK_RATE_HZ))) && (hal_sleep_manager_is_sleep_locked() == 0)) {
        AST_vPortSuppressTicksAndSleep(xExpectedIdleTime);
        timestamp_diff = 0;
        return;
    }
   
    /* Make sure the OS GPT reload value does not overflow the counter. */
    if (xExpectedIdleTime > (xMaximumPossibleSuppressedTicks)) {
        xExpectedIdleTime = (xMaximumPossibleSuppressedTicks);
    }   

    /*disable wdt*/
#ifdef MTK_SYSTEM_HANG_CHECK_ENABLE
    hal_wdt_disable(HAL_WDT_DISABLE_MAGIC);
#endif

    /* If a context switch is pending or a task is waiting for the scheduler
    to be unsuspended then abandon the low power entry. */
    if (eTaskConfirmSleepModeStatus() == eAbortSleep) {
        /* Restart OS GPT. */
        os_gpt0_resume(false, 0);

        /* enable wdt*/
#ifdef MTK_SYSTEM_HANG_CHECK_ENABLE
        hal_wdt_enable(HAL_WDT_ENABLE_MAGIC);
#endif

        /* Re-enable interrupts - see comments above the cpsid instruction()
        above. */
        __asm volatile("cpsie i");
    } else {
        if ((timestamp_diff < 0) && (xExpectedIdleTime > 2)) {
            ulReloadValue = ((xExpectedIdleTime - 2UL) * (1000 / configTICK_RATE_HZ));
            skip_time = 1;
        } else {
            ulReloadValue = ((xExpectedIdleTime - 1UL) * (1000 / configTICK_RATE_HZ));
        }
        /* Enter Idle mode */
        if (ulReloadValue > 0) {
            hal_sleep_manager_set_sleep_time((uint32_t)ulReloadValue);
            hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_IDLE);
        }
        
        /* Calculate and Calibration Idle Time to OS Tick */
        doIdelSystickCalibration(xExpectedIdleTime);
     
        /* Restart OS GPT. */
        os_gpt0_resume(false, 0);

        /* enable wdt*/
#ifdef MTK_SYSTEM_HANG_CHECK_ENABLE
        hal_wdt_enable(HAL_WDT_ENABLE_MAGIC);
#endif

        /* Re-enable interrupts - see comments above the cpsid instruction() above. */
        __asm volatile("cpsie i");

        log_debug("\r\nST_CPT=%u\r\n"   , (unsigned int)xExpectedIdleTime);
    }
}

static void tickless_log_timestamp_callback(TimerHandle_t expiredTimer)
{
    xTimerChangePeriod( timestamp_timer, 1000 * 60 * 60 * 12 / portTICK_PERIOD_MS, 0 ); //12hours  
    tickless_log_timestamp();
}

void tickless_log_timestamp()
{    
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &TimeStampCounter);
    TimeStampSystick = (uint32_t)xTaskGetTickCount() * (1000 / configTICK_RATE_HZ);
    get_rtc_real_clock_freq(); 
}

extern uint32_t f32k_measure_count(uint16_t fixed_clock, uint16_t tested_clock, uint16_t window_setting);
static void get_rtc_real_clock_freq(void)
{
    #define windows_cnt 1999
    /* waiting for RTC Driver ready */
  
    uint32_t rtc_count,now=0,last=0;    
   
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &last);	    
    rtc_count = f32k_measure_count(1, 1, windows_cnt); 

    /* use 26MHz xtal */
    RTC_Freq = (float)rtc_count;    
    RTC_Freq = (26000000.f/RTC_Freq)*(windows_cnt + 1.f);
    RTC_Freq /= 1000;                       
  
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &now);	
    
    #if 0
    printf("***************************************************\r\n");
    printf("f32k_measure_count:%d,%d,%d\r\n",(unsigned int)(RTC_Freq*100000),rtc_count,now-last);
    printf("***************************************************\r\n");      
    #endif     
}
#endif

GPT_REGISTER_T *os_gpt0 = OS_GPT0;
OS_GPT_REGISTER_GLOABL_T *os_gpt_glb = OS_GPTGLB;
bool reset_gpt_to_systick = false;

extern void xPortSysTickHandler(void);
void os_gpt_interrupt_handle(hal_nvic_irq_t irq_number)
{
    os_gpt0->GPT_IRQ_ACK = 0x01;

    /* Run FreeRTOS tick handler*/
    xPortSysTickHandler();
}

void os_gpt_init(uint32_t ms)
{
    /* set 13 divided with 13M source */
    os_gpt0->GPT_CON_UNION.GPT_CON |= (1 << 16);   // disable clock before config
    os_gpt0->GPT_CLK = 0xc;
    os_gpt0->GPT_CON_UNION.GPT_CON &= ~(1 << 16);   // enable clock
    os_gpt0->GPT_COMPARE = ms * 1000;

    /* clear */
    os_gpt0->GPT_CLR = 0x01;
    while (os_gpt0->GPT_COUNT);

    /* enable IRQ */
    os_gpt0->GPT_IRQ_EN = 0x1;
    /* enable GPT0 clk and repeat mode and enable GPT0 */
    os_gpt0->GPT_CON_UNION.GPT_CON |= 0x101;

    /* register and enable IRQ */
    hal_nvic_register_isr_handler(OS_GPT_IRQn, (hal_nvic_isr_t)os_gpt_interrupt_handle);
    NVIC_EnableIRQ(OS_GPT_IRQn);
    os_gpt_glb->OS_GPT_IRQMSK &= 0x2;
    //os_gpt_glb->OS_GPT_WAKEUPMSK &= 0x2;  // mask as system will dead when boot-up, must unmask after sleep<->wake is ok
}

void os_gpt0_pause(void)
{
    os_gpt0->GPT_CON_UNION.GPT_CON &= 0xFFFFFFFE;
}

void os_gpt0_resume(bool update, uint32_t new_compare)
{
    if (update) {
        reset_gpt_to_systick = true;
        os_gpt0->GPT_COMPARE = new_compare;
    }
    os_gpt0->GPT_CON_UNION.GPT_CON |= 0x00000001;
}

void vPortSetupTimerInterrupt(void)
{
#ifdef TEMP_FPGA_ML1S_HACKS
/* Tick has to be 20 times slower when using slow bitfiles */
extern bool use_slow_bitfile;
if (use_slow_bitfile==true)
    os_gpt_init(portTICK_PERIOD_MS*20); /* 1tick = 1ms */
else
#endif //TEMP_FPGA_ML1S_HACKS
    os_gpt_init(portTICK_PERIOD_MS); /* 1tick = 1ms */
    
#if configUSE_TICKLESS_IDLE == 2
    static int init_timestamp = 0;
    if (init_timestamp == 0) {
        init_timestamp = 1;

        tickless_log_timestamp();
        timestamp_timer = xTimerCreate("timestamp_timer",
                                       1000 * 3 / portTICK_PERIOD_MS, 
                                       true,
                                       NULL,
                                       tickless_log_timestamp_callback);        
                                       
        if (timestamp_timer == NULL) {
            printf("timestamp_timer create fail\n");
        } else {
            if (xTimerStart(timestamp_timer, 0) != pdPASS) {
                printf("timestamp_timer xTimerStart fail\n");
            }
        }
    }
#endif /* configUSE_TICKLESS_IDLE */

#if configUSE_TICKLESS_IDLE != 0
    /* Calculate the constants required to configure the tick interrupt. */
    {
        /* OS GPT one count equal 1us */
        ulTimerCountsForOneTick = (1000000 / configTICK_RATE_HZ);
        /* OS GPT is 32 bits timer */
        xMaximumPossibleSuppressedTicks = 0xFFFFFFFF / ulTimerCountsForOneTick;
    }
#endif /* configUSE_TICKLESS_IDLE  != 0*/
}

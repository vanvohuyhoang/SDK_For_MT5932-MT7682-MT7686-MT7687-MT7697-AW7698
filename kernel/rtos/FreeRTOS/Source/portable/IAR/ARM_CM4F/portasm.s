/*
 * FreeRTOS Kernel V10.1.1
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

#include <FreeRTOSConfig.h>

	RSEG    CODE:CODE(2)
	thumb

	EXTERN pxCurrentTCB
	EXTERN vTaskSwitchContext

	#ifdef MTK_SYSTEM_HANG_TRACER_ENABLE
	EXTERN systemhang_task_exit_trace
	EXTERN systemhang_task_enter_trace
	#endif

	#ifdef MTK_SYSTEM_HANG_CHECK_ENABLE
        EXTERN xportWdtFeed
        #endif

	EXTERN Flash_ReturnReady

	#if (configCHECK_FOR_STACK_OVERFLOW > 0)
	EXTERN hal_dwt_init
	EXTERN vPortCurrentTaskStackOverflowCheck
	#endif /* (configCHECK_FOR_STACK_OVERFLOW > 0) */


	PUBLIC xPortPendSVHandler
	PUBLIC vPortSVCHandler
	PUBLIC vPortStartFirstTask
	PUBLIC vPortEnableVFP


/*-----------------------------------------------------------*/

	SECTION .ram_code:CODE:REORDER:NOROOT(2)
xPortPendSVHandler:
	/* must suspend flash before fetch code from flash */
	cpsid i
	push {lr}
	ldr r0, =Flash_ReturnReady
	blx r0
	pop {lr}
	cpsie i

	mrs r0, psp
	isb
	/* Get the location of the current TCB. */
	ldr	r3, =pxCurrentTCB
	ldr	r2, [r3]

	/* Is the task using the FPU context?  If so, push high vfp registers. */
	tst r14, #0x10
	it eq
	vstmdbeq r0!, {s16-s31}

	/* Save the core registers. */
	stmdb r0!, {r4-r11, r14}

	/* Save the new top of stack into the first member of the TCB. */
	str r0, [r2]

	stmdb sp!, {r0, r3}
	mov r0, #configMAX_SYSCALL_INTERRUPT_PRIORITY
	msr basepri, r0
	dsb
	isb

#ifdef MTK_SYSTEM_HANG_TRACER_ENABLE
	mrs r0, psp
	bl systemhang_task_exit_trace
#endif /* MTK_SYSTEM_HANG_TRACER_ENABLE */

	bl vTaskSwitchContext

#ifdef MTK_SYSTEM_HANG_TRACER_ENABLE
	bl systemhang_task_enter_trace
#endif /* MTK_SYSTEM_HANG_TRACER_ENABLE */

	#ifdef MTK_SYSTEM_HANG_CHECK_ENABLE
        bl xportWdtFeed
        #endif

	/* Enable the stack overflow check by DWT. */
	#if (configCHECK_FOR_STACK_OVERFLOW > 0)
	bl vPortCurrentTaskStackOverflowCheck
	#endif /* (configCHECK_FOR_STACK_OVERFLOW > 0) */

	mov r0, #0
	msr basepri, r0
	ldmia sp!, {r0, r3}

	/* The first item in pxCurrentTCB is the task top of stack. */
	ldr r1, [r3]
	ldr r0, [r1]

	/* Pop the core registers. */
	ldmia r0!, {r4-r11, r14}

	/* Is the task using the FPU context?  If so, pop the high vfp registers
	too. */
	tst r14, #0x10
	it eq
	vldmiaeq r0!, {s16-s31}

	msr psp, r0
	isb
	#ifdef WORKAROUND_PMU_CM001 /* XMC4000 specific errata */
		#if WORKAROUND_PMU_CM001 == 1
			push { r14 }
			pop { pc }
		#endif
	#endif

	bx r14


/*-----------------------------------------------------------*/

	SECTION .ram_code:CODE:REORDER:NOROOT(2)
vPortSVCHandler:
	/* must suspend flash before fetch code from flash */
	cpsid i
	ldr r0, =Flash_ReturnReady
	blx r0
	cpsie i

	/* enable current task stack overflow check */
	#if (configCHECK_FOR_STACK_OVERFLOW > 0)
	bl hal_dwt_init
	bl vPortCurrentTaskStackOverflowCheck
	#endif /* (configCHECK_FOR_STACK_OVERFLOW > 0)  */

	/* Get the location of the current TCB. */
	ldr	r3, =pxCurrentTCB
	ldr r1, [r3]
	ldr r0, [r1]
	/* Pop the core registers. */
	ldmia r0!, {r4-r11, r14}
	msr psp, r0
	isb
	mov r0, #0
	msr	basepri, r0
	bx r14

/*-----------------------------------------------------------*/

vPortStartFirstTask
	/* Use the NVIC offset register to locate the stack. */
	ldr r0, =0xE000ED08
	ldr r0, [r0]
	ldr r0, [r0]
	/* Set the msp back to the start of the stack. */
	msr msp, r0
	/* Clear the bit that indicates the FPU is in use in case the FPU was used
	before the scheduler was started - which would otherwise result in the
	unnecessary leaving of space in the SVC stack for lazy saving of FPU
	registers. */
	mov r0, #0
	msr control, r0
	/* Call SVC to start the first task. */
	cpsie i
	cpsie f
	dsb
	isb
	svc 0

/*-----------------------------------------------------------*/

vPortEnableVFP:
	/* The FPU enable bits are in the CPACR. */
	ldr.w r0, =0xE000ED88
	ldr	r1, [r0]

	/* Enable CP10 and CP11 coprocessors, then save back. */
	orr	r1, r1, #( 0xf << 20 )
	str r1, [r0]
	bx	r14



	END


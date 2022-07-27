/*
 * Copyright (c) 2016 Raspberry Pi (Trading) Ltd.
 * Copyright (c) 2016 Stephen Warren <swarren@wwwdotorg.org>
 * Copyright (c) 2016 Oleksandr Tymoshenko <gonzo@freebsd.org>
 * Copyright (c) 2021 Ivan T. Ivanov <iivanov@suse.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * * Neither the name of the copyright holder nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define BIT(x) 					(1 << (x))

/* Secure Configuration Register */
#define SCR_SMD					BIT(7)

/* CPU Auxiliary Control Register */
#define CPUACTLR_EL1				S3_1_C15_C2_0
#define CPUACTLR_EL1_DIS_LOAD_PASS_STORE	BIT(55)

/* System Control Register */
#define SCTLR_M_BIT				BIT(0)

/* Translation Control Register */
#define TCR_EPD0_BIT				BIT(7)
#define TCR_EPD1_BIT				BIT(23)

/* Function ID's */
#define SMCCC_VERSION				0x80000000
#define SMCCC_ARCH_FEATURES			0x80000001
#define SMCCC_ARCH_WORKAROUND_1			0x80008000
#define SMCCC_ARCH_WORKAROUND_2			0x80007fff

#define PSCI_VERSION				0x84000000
#define PSCI_CPU_OFF				0x84000002
#define PSCI_CPU_ON				0xC4000003
#define PSCI_AFFINITY_INFO			0xC4000004
#define PSCI_MIGRATE_INFO_TYPE			0x84000006
#define PSCI_MIGRATE_INFO_UP_CPU		0xC4000007
#define PSCI_SYSTEM_OFF				0x84000008
#define PSCI_SYSTEM_RESET			0x84000009
#define PSCI_FEATURES				0x8400000a

#define WDOG_ADDRESS				0xfe100000
#define WDOG_RSTC				0x1c
#define WDOG_RSTS				0x20
#define WDOG_WDOG				0x24

#define WDOG_PASSWORD				0x5a000000
#define WDOG_WDOG_TIMEOUT			(WDOG_PASSWORD | 10)  /* ~150ms */
#define WDOG_RSTC_CLR				0xffffffcf
#define WDOG_RSTC_FULL_RESET			(WDOG_PASSWORD | 0x00000020)
#define WDOG_RSTS_CLR				0xfffffaaa
#define WDOG_RSTS_FULL_HALT			(WDOG_PASSWORD | 0x0000003f)

#define CPU_COUNT				4
#define CPU_STACK_SIZE				0x100

.extern in_el3
.globl setup_psci_monitor


	.macro	save_registers
	stp	x29, x30, [sp, -16]!
	.endm

	.macro	restore_registers
	ldp	x29, x30, [sp], 16
	.endm

	/* Macro for mitigating against speculative execution beyond ERET */
	.macro	exception_return
	eret
	dsb	nsh
	isb
	.endm

	/*
	 * Enables/disables page table walk as speculative AT instructions
	 * using an out-of-context translation regime could cause subsequent
	 * requests to generate an incorrect translation.
	 */
	.macro	wa_speculative_at
	mrs	x30, tcr_el1
	mrs	x29, sctlr_el1
	stp	x29, x30, [sp, -16]!

	/* ------------------------------------------------------------
	 * Must follow below order in order to disable page table
	 * walk for lower ELs (EL1 and EL0). First step ensures that
	 * page table walk is disabled for stage1 and second step
	 * ensures that page table walker should use TCR_EL1.EPDx
	 * bits to perform address translation. ISB ensures that CPU
	 * does these 2 steps in order.
	 *
	 * 1. Update TCR_EL1.EPDx bits to disable page table walk by
	 *    stage1.
	 * 2. Enable MMU bit to avoid identity mapping via stage2
	 *    and force TCR_EL1.EPDx to be used by the page table
	 *    walker.
	 * ------------------------------------------------------------
	 */
	orr	x30, x30, TCR_EPD0_BIT
	orr	x30, x30, TCR_EPD1_BIT
	msr	tcr_el1, x30
	isb
	orr	x29, x29, SCTLR_M_BIT
	msr	sctlr_el1, x29

	/* -----------------------------------------------------------
	 * Must follow below order to ensure that page table walk is
	 * not enabled until restoration of all EL1 system registers.
	 * TCR_EL1 register
	 * should be updated at the end which restores previous page
	 * table walk setting of stage1 i.e.(TCR_EL1.EPDx) bits. ISB
	 * ensures that CPU does below steps in order.
	 *
	 * 1. Ensure all other system registers are written before
	 *    updating SCTLR_EL1 using ISB.
	 * 2. Restore SCTLR_EL1 register.
	 * 3. Ensure SCTLR_EL1 written successfully using ISB.
	 * 4. Restore TCR_EL1 register.
	 * -----------------------------------------------------------
	 */
	isb
	ldp	x29, x30, [sp], 16
	msr	sctlr_el1, x29
	isb
	msr	tcr_el1, x30
	.endm

	/*
	 * Mitigations for CVE-2017-5715 on Arm Cortex CPUs involves
	 * invalidation of the branch predictor.
	 */
	.macro	wa_cve_2017_5715
	mrs	x29, sctlr_el3
	/* Disable MMU */
	bic	x30, x29, SCTLR_M_BIT
	msr	sctlr_el3, x30
	isb

	/* Restore MMU */
	msr	sctlr_el3, x29
	isb
	.endm

	.macro	harden_exception
	save_registers
	wa_cve_2017_5715
	wa_speculative_at
	restore_registers
	exception_return
	.endm

.ltorg

.align 11
el3_vectors:
	/* Sync, Current EL using SP0 */
	exception_return
	.align  7
	/* IRQ, Current EL using SP0 */
	exception_return
	.align  7
	 /* FIQ, Current EL using SP0 */
	exception_return
	.align  7
	/* SError, Current EL using SP0 */
	exception_return
	.align  7
	/* Sync, Current EL using SPx */
	exception_return
	.align  7
	/* IRQ, Current EL using SPx */
	exception_return
	.align  7
	/* FIQ, Current EL using SPx */
	exception_return
	.align  7
	/* SError, Current EL using SPx */
	exception_return
	.align  7
	/* Sync, Lower EL using AArch64 */
	save_registers
	wa_cve_2017_5715
	wa_speculative_at
	bl	handle_sync
	restore_registers
	exception_return
	.align  7
	/* IRQ, Lower EL using AArch64 */
	harden_exception
	.align  7
	/* FIQ, Lower EL using AArch64 */
	harden_exception
	.align  7
	/* SError, Lower EL using AArch64 */
	harden_exception
	.align  7
	/* Sync, Lower EL using AArch32 */
	harden_exception
	.align  7
	/* IRQ, Lower EL using AArch32 */
	harden_exception
	.align  7
	/* FIQ, Lower EL using AArch32 */
	harden_exception
	.align  7
	/* SError, Lower EL using AArch32 */
	harden_exception

handle_sync:
	ldr	w29, =SMCCC_ARCH_WORKAROUND_1
	cmp	w0, w29
	b.eq	smccc_arch_wa_1_done
	ldr	w29, =SMCCC_ARCH_WORKAROUND_2
	cmp	w0, w29
	b.eq	smccc_arch_wa_2_enable_disable
	ldr	w29, =PSCI_VERSION
	cmp	w0, w29
	b.eq	psci_version
	ldr	w29, =PSCI_CPU_ON
	cmp	w0, w29
	b.eq	psci_cpu_on
	ldr	w29, =PSCI_FEATURES
	cmp	w0, w29
	b.eq	psci_features_query
	ldr	w29, =SMCCC_ARCH_FEATURES
	cmp	w0, w29
	b.eq	smccc_arch_features_query
	ldr	w29, =SMCCC_VERSION
	cmp	w0, w29
	b.eq	smccc_version
	ldr	w29, =PSCI_MIGRATE_INFO_TYPE
	cmp	w0, w29
	b.eq	psci_migrate_info_type
	ldr	w29, =PSCI_MIGRATE_INFO_UP_CPU
	cmp	w0, w29
	b.eq	psci_migrate_info_up_cpu
	ldr	w29, =PSCI_CPU_OFF
	cmp	w0, w29
	b.eq	psci_cpu_off
	ldr	w29, =PSCI_AFFINITY_INFO
	cmp	w0, w29
	b.eq	psci_affinity_info
	ldr	w29, =PSCI_SYSTEM_OFF
	cmp	w0, w29
	b.eq	psci_system_off
	ldr	w29, =PSCI_SYSTEM_RESET
	cmp	w0, w29
	b.eq	psci_system_reset
	/* fall through to invalid ID case */
	ldr	w0, =0xFFFFFFFF
smccc_arch_wa_1_done:
	ret

smccc_arch_wa_2_enable_disable:
	/*
	 * Mitigations for CVE-2018-3639 on Arm Cortex CPUs involve disabling
	 * the bypassing of writes by reads (including speculative reads).
	 *
	 * Cortex-A57, Cortex-A72 - Permanently set bit 55
	 * (Disable load pass store) of CPUACTLR_EL1 (S3_1_C15_C2_0)
	 */
	mrs	x0, CPUACTLR_EL1
	orr	x0, x0, CPUACTLR_EL1_DIS_LOAD_PASS_STORE
	/*
	 * A non-zero value indicates that the mitigation
	 * for CVE-2018-3639 must be enabled.
	 */
	cbnz	w1, 1f
	bic	x0, x0, CPUACTLR_EL1_DIS_LOAD_PASS_STORE
1:
	msr	CPUACTLR_EL1, x0
	isb
	dsb	sy
	ret

psci_migrate_info_type:
	/*
	 * The Trusted OS will only run one core.
	 * The Trusted OS does not support the MIGRATE function
	 */
	mov	w0, 1
	ret

smccc_version:
	/*
	 * Version v1.1 is required for SMCCC_ARCH_WORKAROUND_1
	 * and SMCCC_ARCH_WORKAROUND_2 support
	 */
	ldr	w0, =0x10001
	ret

psci_cpu_off:
	mrs	x6, MPIDR_EL1
	and	x6, x6, #0x3
	adr 	x5, spin_cpu0
	mov	x4, 0
	str	x4, [x5, x6, lsl #3]
	dmb	sy

	/* Go to secondary CPUs WFE boot loop */
	restore_registers
	adr	x30, in_el3
	ret

psci_cpu_on:
	/* x1 - target CPU, x2 - Entrypoint */
	adr	x29, spin_cpu0
	and	x1, x1, 3
	str 	x2, [x29, x1, lsl 3]
	dsb	sy
	sev
	mov	w0, 0
	ret

psci_migrate_info_up_cpu:
	mov	w0, 0	/* Pretend that monitor is resiendt on CPU0 */
	ret

psci_version:
	/* Return v1.0 */
	mov	w0, 0x00010000
	ret

psci_system_off:
	ldr	x0, =WDOG_ADDRESS
	ldr	w1, [x0, WDOG_RSTS]
	ldr	w2, =WDOG_RSTS_CLR
	and	w1, w1, w2
	ldr	w2, =WDOG_RSTS_FULL_HALT
	orr 	w1, w1, w2
	str	w1, [x0, WDOG_RSTS]
	/* fall through */
psci_system_reset:
	ldr	x0, =WDOG_ADDRESS
	ldr	w1, =WDOG_WDOG_TIMEOUT
	str	w1, [x0, WDOG_WDOG]
	ldr	w1, [x0, WDOG_RSTC]
	ldr	w2, =WDOG_RSTC_CLR
	and	w1, w1, w2
	ldr	w2, =WDOG_RSTC_FULL_RESET
	orr	w1, w1, w2
	str	w1, [x0, WDOG_RSTC]
loop:
	wfe
	b	loop

psci_features_query:
	ldr	w29, =SMCCC_VERSION
	cmp	w1, w29
	/* Version read is supported */
	b.eq	return_success
	ldr	w29, =SMCCC_ARCH_WORKAROUND_1
	cmp	w1, w29
	/* Check for workaround is supported */
	b.eq	return_success
	ldr	w29, =SMCCC_ARCH_WORKAROUND_2
	cmp	w1, w29
	/* Check for workaround is supported */
	b.eq	return_success
	ldr	w0, =0xFFFFFFFF /* not supported */
	ret

smccc_arch_features_query:
	ldr	w29, =SMCCC_ARCH_WORKAROUND_1
	cmp	w1, w29
	/*
	 * Return 0 - SMCCC_ARCH_WORKAROUND_1 can be invoked safely on all
	 * PEs in the system. The PE on which SMCCC_ARCH_FEATURES is called
	 * requires firmware mitigation for CVE-2017-5715.
	 */
	b.eq	return_success
	ldr	w29, =SMCCC_ARCH_WORKAROUND_2
	cmp	w1, w29
	/*
	 * Return 0 - SMCCC_ARCH_WORKAROUND_2 can be invoked safely on all
	 * PEs in the system. The PE on which SMCCC_ARCH_FEATURES is called
	 * requires dynamic firmware mitigation for CVE-2018- 3639 using
	 * SMCCC_ARCH_WORKAROUND_2.
	 */
	b.eq	return_success
	ldr	w0, =0xFFFFFFFF		/* not supported */
	ret

return_success:
	mov	w0, 0
	ret

psci_affinity_info:
	and	x1, x1, 3
	adr	x29, spin_cpu0
	dsb	sy
	ldr	x0, [x29, x1, lsl 3]
	/* If no address assume that CPU is in WFE loop */
	cmp	w0, 0
	cset	w0, eq		/* CPU is ON(0) or OFF(1) */
	ret

setup_psci_monitor:
	adr	x0, el3_vectors
	msr	vbar_el3, x0

	/* Get CPU number */
	mrs	x6, MPIDR_EL1
	and	x6, x6, 0x3

	/* Setup per CPU stack */
	msr     SPsel, 1
	adrp	x1, stack_top
	add	x1, x1, :lo12: stack_top
	mov	x3, CPU_STACK_SIZE
	mul	x3, x6, x3
	sub	x1, x1, x3
	mov	sp, x1

	cbnz	x6, 1f

	stp	x29, x30, [sp, -16]!
	ldr	w0, dtb_ptr32
	/*
	 * Insert psci device tree node and increase reserved
	 * memory with one 4K page
	 */
	bl	fixup_dt_blob
	ldp	x29, x30, [sp], 16
1:
	/* Enable SMC */
	mrs	x0, SCR_EL3
	bic	x0, x0, SCR_SMD
	msr	SCR_EL3, x0
	ret

	.align 5
stack_bottom:
	.space (CPU_COUNT * CPU_STACK_SIZE), 0
stack_top:

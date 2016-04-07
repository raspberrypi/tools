/*
 * Copyright (c) 2016 Raspberry Pi (Trading) Ltd.
 * Copyright (c) 2016 Stephen Warren <swarren@wwwdotorg.org>
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

#define BIT(x) (1 << (x))

#define LOCAL_CONTROL		0x40000000
#define LOCAL_PRESCALER		0x40000008

#define OSC_FREQ		19200000

#define SCR_RW			BIT(10)
#define SCR_HCE			BIT(8)
#define SCR_SMD			BIT(7)
#define SCR_RES1_5		BIT(5)
#define SCR_RES1_4		BIT(4)
#define SCR_NS			BIT(0)
#define SCR_VAL \
    (SCR_RW | SCR_HCE | SCR_SMD | SCR_RES1_5 | SCR_RES1_4 | SCR_NS)

#define CPUECTLR_EL1		S3_1_C15_C2_1
#define CPUECTLR_EL1_SMPEN	BIT(6)

#define SPSR_EL3_D		BIT(9)
#define SPSR_EL3_A		BIT(8)
#define SPSR_EL3_I		BIT(7)
#define SPSR_EL3_F		BIT(6)
#define SPSR_EL3_MODE_EL2H	9
#define SPSR_EL3_VAL \
    (SPSR_EL3_D | SPSR_EL3_A | SPSR_EL3_I | SPSR_EL3_F | SPSR_EL3_MODE_EL2H)

.globl _start
_start:
	/*
	 * LOCAL_CONTROL:
	 * Bit 9 clear: Increment by 1 (vs. 2).
	 * Bit 8 clear: Timer source is 19.2MHz crystal (vs. APB).
	 */
	mov x0, LOCAL_CONTROL
	str wzr, [x0]
	/* LOCAL_PRESCALER; divide-by (0x80000000 / register_val) == 1 */
	mov w1, 0x80000000
	str w1, [x0, #(LOCAL_PRESCALER - LOCAL_CONTROL)]

	/* Set up CNTFRQ_EL0 */
	ldr x0, =OSC_FREQ
	msr CNTFRQ_EL0, x0

	/* Set up CNTVOFF_EL2 */
	msr CNTVOFF_EL2, xzr

	/* Enable FP/SIMD */
	/* All set bits below are res1; bit 10 (TFP) is set to 0 */
	mov x0, #0x33ff
	msr CPTR_EL3, x0

	/* Set up SCR */
	mov x0, #SCR_VAL
	msr SCR_EL3, x0

	/* Set SMPEN */
	mov x0, #CPUECTLR_EL1_SMPEN
	msr CPUECTLR_EL1, x0

	/*
	 * Set up SCTLR_EL2
	 * All set bits below are res1. LE, no WXN/I/SA/C/A/M
	 */
	ldr x0, =0x30c50830
	msr SCTLR_EL2, x0

	/* Switch to EL2 */
	mov x0, #SPSR_EL3_VAL
	msr spsr_el3, x0
	adr x0, in_el2
	msr elr_el3, x0
	eret
in_el2:

	mrs x6, MPIDR_EL1
	and x6, x6, #0x3
	cbz x6, primary_cpu

	adr x5, spin_table
secondary_spin:
	wfe
	ldr x4, [x5, x6, lsl #3]
	cbz x4, secondary_spin
	mov x0, #0
	b boot_kernel

primary_cpu:
	ldr x4, kernel_entry
	ldr x0, dtb_ptr

boot_kernel:
	mov x1, #0
	mov x2, #0
	mov x3, #0
	br x4

.ltorg

.org 0xd0
.globl kernel_entry
kernel_entry:
	.quad 0x8000

.org 0xd8
.globl spin_table
spin_table:
	.quad 0 // CPU 0 @ 0xd8
	.quad 0 // CPU 1 @ 0xe0
	.quad 0 // CPU 2 @ 0xe8
	.quad 0 // CPU 3 @ 0xf0

.org 0xf8
.globl dtb_ptr
dtb_ptr:
	.quad 0x100

.org 0x100
.globl dtb_space
dtb_space:

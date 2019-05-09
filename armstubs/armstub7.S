/*
Copyright (c) 2016 Raspberry Pi (Trading) Ltd.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

.arch_extension sec
.arch_extension virt

.section .init
.globl _start
/* the vector table for secure state and HYP mode */
_start:
	b jmp_loader 	/* reset */
#if defined(BCM2711) && (BCM2711 == 1)
osc:	.word 54000000
#else
osc:	.word 19200000
#endif

/*
 * secure monitor handler
 * U-boot calls this "software interrupt" in start.S
 * This is executed on a "smc" instruction, we use a "smc #0" to switch
 * to non-secure state.
 * We use only r0 and r1 here, due to constraints in the caller.
 */
_secure_monitor:
	movw	r1, #0x131			@ set NS, AW, FW, HVC
	mcr	p15, 0, r1, c1, c1, 0		@ write SCR (with NS bit set)

	movw	r0, #0x1da			@ Set HYP_MODE | F_BIT | I_BIT | A_BIT
	msr     spsr_cxfs, r0                   @ Set full SPSR

#if defined(BCM2711) && (BCM2711 == 1)
	mov r1, #0x22       			@ Set L2 read/write latency to 2
	mcr	p15, 1, r1, c9, c0, 2		@ Write L2CTLR
#endif

	movs	pc, lr				@ return to non-secure SVC

value:		.word 0x63fff
#define machid 3138
#if defined(BCM2711) && (BCM2711 == 1)

#define PRESCALER_2711	0xff800008
#define MBOX_2711	0xff8000cc

mbox:		.word MBOX_2711
GIC_DISTB:	.word 0xff841000
#else
mbox:		.word 0x400000cc
#endif
jmp_loader:
@ Check which proc we are and run proc 0 only

.if !BCM2710
	mrc p15, 0, r0, c1, c0, 1 @ Read Auxiliary Control Register
	orr r0, r0, #(1<<6)       @ SMP
	mcr p15, 0, r0, c1, c0, 1 @ Write Auxiliary Control Register
.else
	mrrc p15, 1, r0, r1, c15  @ CPU Extended Control Register
	orr r0, r0, #(1<<6)       @ SMP
	and r1, r1, #(~3)         @ Set L2 load data prefetch to 0b00 = 16
	mcrr p15, 1, r0, r1, c15  @ CPU Extended Control Register
.endif
	mrc p15, 0, r0, c1, c0, 0 @ Read System Control Register
	/* Cortex A72 manual 4.3.67 says SMP must be set before enabling the cache. */
	orr r0, r0, #(1<<12)      @ icache enable
	mcr p15, 0, r0, c1, c0, 0 @ Write System Control Register

	mrc p15, 0, r7, c0, c0, 5 @ Put core number into R7

#ifdef GIC

#define GIC_CPUB_offset 0x1000

#define GICC_CTRLR	0x0
#define GICC_PMR	0x4
#define IT_NR		0x8		@ Number of interrupt enable registers (256 total irqs)
#define GICD_CTRLR	0x0
#define GICD_IGROUPR	0x80

setup_gic:				@ Called from secure mode - set all interrupts to group 1 and enable.
	ldr	r2, GIC_DISTB

	ands    r7,r7, #3		@ primary core

	movne	r0, #3			@ Enable group 0 and 1 IRQs from distributor
	strne	r0, [r2,  #GICD_CTRLR]

	mov     r0, #~0
	mov	    r1, #~0			@ group 1 all the things
	strd    r0, r1, [r2,#(GICD_IGROUPR)]!	@ update to bring the CPU registers within range
	strd    r0, r1, [r2,#8]
	strd    r0, r1, [r2,#16]

	movw	r1, #0x1e7
	str	r1, [r2, #(GIC_CPUB_offset - GICD_IGROUPR) ]! @ Enable group  1 IRQs from CPU interface

	movw	r1, #0xff
	str	r1, [r2, #GICC_PMR] @ priority mask

#endif

	mov r0, #1
	mcr p15, 0, r0, c14, c3, 1 @ CNTV_CTL (enable=1, imask=0)

@ set to non-sec
	ldr	r1, value			@ value = 0x63fff
	mcr	p15, 0, r1, c1, c1, 2		@ NSACR = all copros to non-sec
@ timer frequency
	ldr	r1, osc				@ osc = 19.2 / 54MHz
	mcr	p15, 0, r1, c14, c0, 0		@ write CNTFRQ
#if defined(BCM2711) && (BCM2711 == 1)
	mov	r1, #0x80000000			@ Set ARM_LOCAL_TIMER_PRE_ADD to 1
	ldr	r2, mbox
	str	r1, [r2, #(PRESCALER_2711 - MBOX_2711)]
#endif

	adr	ip, _start
	mcr	p15, 0, ip, c12, c0, 1		@ set MVBAR to secure vectors

	isb
	smc	#0				@ call into MONITOR mode

	mcr	p15, 0, ip, c12, c0, 0		@ write non-secure copy of VBAR

	ldrd	r2,r3, atags		@ ATAGS and kernel

	ands    r7,r7, #3                          @ primary core
	beq     9f

	ldr	r5, mbox		@ mbox

1:
	wfe
	ldr	r3, [r5, r7, lsl #4]
	cmp	r3, #0          @ magic
	beq	1b
@ clear mailbox
	str	r3, [r5, r7, lsl #4]
9:
	mov	r0, #0
	movw  r1, #machid		@ BCM2708 machine id
	bx	r3

.org 0xf0
.word 0x5afe570b	@ magic value to indicate firmware should overwrite atags and kernel
.word 0			@ version
atags:	.word 0x0	@ device tree address
kernel:	.word 0x0	@ kernel start address

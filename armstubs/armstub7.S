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
	mrc	p15, 1, r1, c9, c0, 2		@ Read L2CTLR
	orr     r1, r1, #0x22			@ Set L2 read/write latency to 2
	mcr	p15, 1, r1, c9, c0, 2		@ Write L2CTLR
#endif

	movs	pc, lr				@ return to non-secure SVC

value:	.word 0x63fff
machid:	.word 3138
#if defined(BCM2711) && (BCM2711 == 1)
mbox: 	.word 0xFF80008C
#else
mbox: 	.word 0x4000008C
#endif
prescaler:	.word 0xff800008
GIC_DISTB:	.word 0xff841000
GIC_CPUB:	.word 0xff842000

#define GICC_CTRLR	0x0
#define GICC_PMR	0x4
#define IT_NR		0x7		@ Number of interrupt enable registers (256 total irqs)
#define GICD_CTRLR	0x0
#define GICD_IGROUPR	0x80

@ Shoehorn the GIC code between the reset vector and fixed-offset magic numbers at 240b

setup_gic:				@ Called from secure mode - set all interrupts to group 1 and enable.
	mrc	p15, 0, r0, c0, c0, 5
	ubfx	r0, r0, #0, #2
	cmp	r0, #0			@ primary core
	beq	2f
	ldr	r2, GIC_DISTB
	add	r2, r2, #GICD_CTRLR
	mov	r0, #3			@ Enable group 0 and 1 IRQs from distributor
	str	r0, [r2]
2:
	ldr	r0, GIC_CPUB
	add	r0, r0, #GICC_CTRLR
	movw	r1, #0x1e7
	str	r1, [r0]		@ Enable group  1 IRQs from CPU interface
	ldr	r0, GIC_CPUB
	add	r0, r0, #GICC_PMR	@ priority mask
	movw	r1, #0xff
	str	r1, [r0]
	mov	r0, #IT_NR
	mov	r1, #~0			@ group 1 all the things
	ldr	r2, GIC_DISTB
	add	r2, r2, #(GICD_IGROUPR)
3:
	str	r1, [r2]
	add	r2, r2, #4
	sub	r0, r0, #1
	cmp	r0, #0
	bne	3b
	str	r1, [r2]
	mov	pc, lr

.org 0xf0
.word 0x5afe570b	@ magic value to indicate firmware should overwrite atags and kernel
.word 0			@ version
atags:	.word 0x0	@ device tree address
kernel:	.word 0x0	@ kernel start address

jmp_loader:
@ Check which proc we are and run proc 0 only
#ifdef GIC
	bl	setup_gic
#endif

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
	/* Cortex A72 manual 4.3.67 says says SMP must be set before enabling the cache. */
#ifndef BCM2711
	orr r0, r0, #(1<<2)       @ cache enable
#endif
	orr r0, r0, #(1<<12)      @ icache enable
	mcr p15, 0, r0, c1, c0, 0 @ Write System Control Register
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
	ldr	r2, prescaler
	str	r1, [r2]
#endif

	adr	r1, _start
	mcr	p15, 0, r1, c12, c0, 1		@ set MVBAR to secure vectors
	mrc	p15, 0, ip, c12, c0, 0		@ save secure copy of VBAR

	isb
	smc	#0				@ call into MONITOR mode

	mcr	p15, 0, ip, c12, c0, 0		@ write non-secure copy of VBAR

	ldr	r4, kernel			@ kernel address to execute from
	mrc     p15, 0, r0, c0, c0, 5
	ubfx    r0, r0, #0, #2
	cmp     r0, #0                          @ primary core
	beq     9f

	mov	r5, #1
	lsl	r5, r0
@ modify the 0xff for force_core mode
	tst	r5, #0xff                       @ enabled cores
	beq	10f

	ldr	r5, mbox		@ mbox
	mov	r3, #0			@ magic

	add	r5, #(0x400000CC-0x4000008C)	@ mbox
1:
	wfe
	ldr	r4, [r5, r0, lsl #4]
	cmp	r4, r3
	beq	1b
@ clear mailbox
	str	r4, [r5, r0, lsl #4]
9:
	mov	r0, #0
	ldr	r1, machid		@ BCM2708 machine id
	ldr	r2, atags		@ ATAGS
	bx	r4
10:
	wfi
	b	10b


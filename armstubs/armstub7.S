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

#define HYP_MODE        0x1a
#define T_BIT           0x20
#define F_BIT           0x40
#define I_BIT           0x80
#define A_BIT           0x100

.arch_extension sec
.arch_extension virt

.section .init
.globl _start
/* the vector table for secure state and HYP mode */
_start:
	b jmp_loader 	/* reset */
	.word 0 /* undef */
	adr pc, _secure_monitor
	.word 0
	.word 0
atags:	.word 0x100 /* Reserved */
value:	.word 0x63fff /* irq will be overwritten later */
osc:	.word 19200000 /* fiq will be overwritten later */

/*
 * secure monitor handler
 * U-boot calls this "software interrupt" in start.S
 * This is executed on a "smc" instruction, we use a "smc #0" to switch
 * to non-secure state.
 * We use only r0 and r1 here, due to constraints in the caller.
 */
_secure_monitor:
	mrc	p15, 0, r1, c1, c1, 0		@ read SCR
	bic	r1, r1, #0x4e			@ clear IRQ, FIQ, EA, nET bits
	orr	r1, r1, #0x31			@ enable NS, AW, FW bits
	orr	r1, r1, #0x100			@ allow HVC instruction

	mcr	p15, 0, r1, c1, c1, 0		@ write SCR (with NS bit set)

	mov     r0, #(F_BIT | I_BIT | A_BIT)    @ Set A, I and F
	orr     r0, r0, #HYP_MODE               @ Slot target mode in
	msr     spsr_cxfs, r0                   @ Set full SPSR

	@ Reset CNTVOFF to 0 before leaving monitor mode
	mov	r0, #0
	mcrr	p15, 4, r0, r0, c14		@ Reset CNTVOFF to zero
	movs	pc, lr				@ return to non-secure SVC

jmp_loader:
@ Check which proc we are and run proc 0 only

	mrc p15, 0, r0, c1, c0, 0 @ Read System Control Register
	orr r0, r0, #(1<<2)       @ cache enable
	orr r0, r0, #(1<<12)      @ icache enable
	mcr p15, 0, r0, c1, c0, 0 @ Write System Control Register
.if !BCM2710
	mrc p15, 0, r0, c1, c0, 1 @ Read Auxiliary Control Register
	orr r0, r0, #(1<<6)       @ SMP
	mcr p15, 0, r0, c1, c0, 1 @ Write Auxiliary Control Register
.else
	mrrc p15, 1, r0, r1, c15  @ CPU Extended Control Register
	orr r0, r0, #(1<<6)       @ SMP
	mcrr p15, 1, r0, r1, c15  @ CPU Extended Control Register
.endif

	mov r0, #1
	mcr p15, 0, r0, c14, c3, 1 @ CNTV_CTL (enable=1, imask=0)

@ set to non-sec
	ldr	r1, value			@ value = 0x63fff
	mcr	p15, 0, r1, c1, c1, 2		@ NSACR = all copros to non-sec
@ timer frequency
	ldr	r1, osc				@ osc = 19200000
	mcr	p15, 0, r1, c14, c0, 0		@ write CNTFRQ

	adr	r1, _start
	mcr	p15, 0, r1, c12, c0, 1		@ set MVBAR to secure vectors
	mrc	p15, 0, ip, c12, c0, 0		@ save secure copy of VBAR

	isb
	smc	#0				@ call into MONITOR mode

	mcr	p15, 0, ip, c12, c0, 0		@ write non-secure copy of VBAR

	mov	r4, #0x8000
	mrc     p15, 0, r0, c0, c0, 5
	ubfx    r0, r0, #0, #2
	cmp     r0, #0                          @ primary core
	beq     9f

	mov	r5, #1
	lsl	r5, r0
@ modify the 0xff for force_core mode
	tst	r5, #0xff                       @ enabled cores
	beq	10f

	ldr	r5, =0x4000008C		@ mbox
	mov	r3, #0			@ magic
	str	r3, [r5, r0, lsl #4]

	add	r5, #(0x400000CC-0x4000008C)	@ mbox
1:
	ldr	r4, [r5, r0, lsl #4]
	cmp	r4, r3
	beq	1b
@ clear mailbox
	str	r4, [r5, r0, lsl #4]
9:
	mov	r0, #0
	ldr	r1, =3138		@ BCM2708 machine id
	ldr	r2, atags		@ ATAGS
	bx	r4
10:
	wfi
	b	10b

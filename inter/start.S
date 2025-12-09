/* Extremely simple (and virtually unnecessary)
 * assembly language startup file.
 */

	.global start
start:
# this is for arm64
	msr		DAIFSet, #7		// disable interrupts

	ldr		x0, =vectors
	msr		VBAR_el2, x0

	bl		main

	.globl spin
spin:	b		spin

// unsigned int GetCurrentSMode(void);
.global GetCurrentSMode
GetCurrentSMode:
        mrs             x0, CurrentEL
        lsr             x0, x0, #2
        ret

# ----------------------------------
# Nothing but vectors below here:

    .align  11
    .globl  vectors
vectors:
    .align  7       /* Current EL, SP = SP_ELx */
	/* Synchronous abort */
	mov	w0,#1
	bl handle_bad
    .align  7       /* IRQ */
	mov	w0,#2
	bl handle_bad
    .align  7       /* FIQ */
	mov	w0,#3
	bl handle_bad
    .align  7       /* Current EL syserr */
	mov	w0,#4
	bl handle_bad

    .align  7       /* Current EL, SP = SP_EL0 */
	/* Synchronous abort */
	stp x0, x1, [sp, #-16]!
	stp x2, x3, [sp, #-16]!
	stp x4, x5, [sp, #-16]!
	stp x6, x7, [sp, #-16]!
	stp x8, x9, [sp, #-16]!
	stp x10, x11, [sp, #-16]!
	stp x12, x13, [sp, #-16]!
	stp x14, x15, [sp, #-16]!
	stp x16, x17, [sp, #-16]!
	stp x18, x30, [sp, #-16]!
	bl handle_sync
	ldp x18, x30, [sp], #16
	ldp x16, x17, [sp], #16
	ldp x14, x15, [sp], #16
	ldp x12, x13, [sp], #16
	ldp x10, x11, [sp], #16
	ldp x8, x9, [sp], #16
	ldp x6, x7, [sp], #16
	ldp x4, x5, [sp], #16
	ldp x2, x3, [sp], #16
	ldp x0, x1, [sp], #16
	eret

    .align  7       /* IRQ */
	stp x0, x1, [sp, #-16]!
	stp x2, x3, [sp, #-16]!
	stp x4, x5, [sp, #-16]!
	stp x6, x7, [sp, #-16]!
	stp x8, x9, [sp, #-16]!
	stp x10, x11, [sp, #-16]!
	stp x12, x13, [sp, #-16]!
	stp x14, x15, [sp, #-16]!
	stp x16, x17, [sp, #-16]!
	stp x18, x30, [sp, #-16]!
	bl handle_int
	ldp x18, x30, [sp], #16
	ldp x16, x17, [sp], #16
	ldp x14, x15, [sp], #16
	ldp x12, x13, [sp], #16
	ldp x10, x11, [sp], #16
	ldp x8, x9, [sp], #16
	ldp x6, x7, [sp], #16
	ldp x4, x5, [sp], #16
	ldp x2, x3, [sp], #16
	ldp x0, x1, [sp], #16
	eret

    .align  7       /* FIQ */
	mov	w0,#13
	bl handle_bad
    .align  7       /* Current EL syserr */
	mov	w0,#14
	bl handle_bad

	/* The Gnu assembler pads with nop instructions */
    .align  11

	/* Handle fall through from all other cases */
	mov	w0,#99
	bl handle_bad

// THE END


/* simple assembly language startup file.
 * - clear bss
 * - set up vectors
 *
 * Tom Trebisy  12-9-2025
 *
 * A silly note, but ....
 * If you keep getting synch aborts when U-boot jumps
 * to the code, check if the network cable is plugged in.
 */

# Here is my method, which has never yet worked
.macro to_el1, el1_reg
#ifdef notdef
	mov		w1, #0
	orr     w1, w1, #(1 << 11)  // set ST bit (disable trapping of timer control registers)
	orr     w1, w1, #(1 << 10)  // set RW bit (next lower EL in aarch64)
	orr     w1, w1, #(1 << 3)   // Set EA bit (SError routed to EL3)
#	orr     w1, w1, #(1 << 2)   // Set FIQ bit (FIQs routed to EL3)
#	orr     w1, w1, #(1 << 1)   // Set IRQ bit (IRQs routed to EL3)
#	msr     SCR_EL3, x1
#endif

	# Black magic, what is this?
	# Doing this is no help.
	mrs     x0, cnthctl_el2
    orr     x0, x0, #3
    msr     cnthctl_el2, x0
    msr     cnthp_ctl_el2, xzr

	# More black magic
	mrs     x0, midr_el1
    mrs     x2, mpidr_el1
    msr     vpidr_el2, x0
    msr     vmpidr_el2, x2

	// disable coprocessor traps
    mov     x0, #0x33FF
    msr     cptr_el2, x0
    msr     hstr_el2, xzr
    mov     x0, #(3 << 20)
    msr     cpacr_el1, x0

	// Setup SCTLR access
	// what is this?
    mov     x2, #0x0800
    movk    x2, #0x30d0, lsl #16
    msr     SCTLR_EL1, x2

	# We don't want garbage in this register
	# all zeros will disable caches and mmu
    # msr     SCTLR_EL1, xzr

	# Holy cow, there is a bit in HCR_el2 to
	# enable aarch64 in EL1
	# Without it, we immediately get an
	#  AArch32 exception on the first EL1 instruction
	# These call this "register width" control for 32/64
	# SWIO is cache set/way invalidation override.
	mov		x0, xzr
	orr		w0, w0, #(1<<31)	// RW
	orr		w0, w0, #(1<<1)		// SWIO
	msr     HCR_el2, x0

	mov		x0, sp
    msr		SP_EL1, x0

#define NO_INTS	(0xf<<6)

#define NEW_EL		NO_INTS | 0x4
# #define NEW_ELx	0x5

#	ldr      x0, =el1_entry
#	adr      x0, el1_entry
#   msr      ELR_EL2, x0 // where to branch to when exception completes
    msr      ELR_EL2, \el1_reg

    ldr      x1, =NEW_EL
    msr      SPSR_EL2, x1 // set the program state for this point to a known value

#    msr      ELR_EL3, x0 		// cause synch abort
# uncomment this to get an abort and look at registers
# using the register dump that U-boot does for us.
#    msr      ELR_EL3, x0 		// cause synch abort
#	x0 : 0000000002000058
#   x1 : 0000000000000004

	isb
    eret
.endm

# --------------------------------------------------------------------

	.global start
start:
# this is for arm64
	msr		DAIFSet, #7		// disable interrupts

    // Clear BSS
	ldr     x1, =__bss_start__
	ldr     x2, =__bss_end__

clear_bss_loop:
	str     xzr, [x1], #8
	cmp     x1, x2
	bls     clear_bss_loop  // if x1 <= x2 goto clear_bss_loop

# Do this for EL2
# set up vectors for EL1 now also
#  (no harm doing this while in EL2)
	ldr		x0, =vectors
#	msr		VBAR_el2, x0
	msr		VBAR_el1, x0

# Now switch from EL2 to EL1
# yes, U-boot sets us running at EL2
# We could set the stack, but we will just
# continue using whatever U-boot set up

	adr     x4, el1_entry
	to_el1  x4

# Start here running in EL1
el1_entry:
	isb
	b		flood

	bl		main

	.globl spin
spin:	b		spin

# Heavy handed, but it works, endlessly sending a single character
# over the RK3399 uart.  An unmistakable indication that code has
# reached a certain point.  Relies on the port initialization done
# by U-boot.
flood:
	mov		x0,#0xff1a0000
	mov		w1,#'E'
1:
	str		w1,[x0]
	b		1b

// no longer needed
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
	bl handle_irq
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
	bl handle_fiq
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

    .align  7       /* Current EL syserr */
	mov	w0,#14
	bl handle_bad

	/* The Gnu assembler pads with nop instructions */
    .align  11

	/* Handle fall through from all other cases */
	mov	w0,#99
	bl handle_bad

// THE END


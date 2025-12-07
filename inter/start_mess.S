/* start.S
 * This is a minimal startup file for a
 * bare metal program that runs on the rk3399
 *
 * Tom Trebisky 9-16-2018
 *
 * With the NSIH header configured, this
 * is intended to be loaded by the bl1 loader.
 * Without it (or with it for that matter), that is
 * is intended to be loaded by U-Boot tftp
 */

.globl  start
start:
        adr     x3, vectors_el2
        msr     VBAR_EL2, x3
		bl		main
		b	.

/* ----------------------------------------------------------- */

// Get our cpu ID
// unsigned int GetCPUID(void);
.global GetCPUID
GetCPUID:
        mrs     x1, MPIDR_EL1
        and     x0, x1, #0x3
		tst     x1, 0xff00
		beq     1f
		orr     x0, x0, 4
1:		ret

// unsigned int GetCurrentSMode(void);
.global GetCurrentSMode
GetCurrentSMode:
        mrs             x0, CurrentEL
        lsr             x0, x0, #2
        ret

/* ----------------------------------------------------------- */

.Lpanic:
    stp     x27, x28, [sp, -16]!
    stp     x25, x26, [sp, -16]!
    stp     x23, x24, [sp, -16]!
    stp     x21, x22, [sp, -16]!
    stp     x19, x20, [sp, -16]!
    stp     x17, x18, [sp, -16]!
    stp     x15, x16, [sp, -16]!
    stp     x13, x14, [sp, -16]!
    stp     x11, x12, [sp, -16]!
    stp     x9,  x10, [sp, -16]!
    stp     x7,  x8,  [sp, -16]!
    stp     x5,  x6,  [sp, -16]!
    stp     x3,  x4,  [sp, -16]!
    stp     x1,  x2,  [sp, -16]!
    stp     xzr, x0,  [sp, -16]!

    mrs         x1, currentEl
    ubfx        x1, x1, 2, 2
    cmp         x1, 3
    b.eq    3f
    cmp         x1, 2
    b.eq    2f
    mrs         x1, esr_el1
    b       1f
3:  mrs     x1, esr_el3
    b       1f
2:  mrs     x1, esr_el2

1:  mov     x0, x29
    add     x2, sp, 8
    bl      sync_handler

    ldp     xzr, x0,  [sp], 16
    ldp     x1,  x2,  [sp], 16
    ldp     x3,  x4,  [sp], 16
    ldp     x5,  x6,  [sp], 16
    ldp     x7,  x8,  [sp], 16
    ldp     x9,  x10, [sp], 16
    ldp     x11, x12, [sp], 16
    ldp     x13, x14, [sp], 16
    ldp     x15, x16, [sp], 16
    ldp     x17, x18, [sp], 16
    ldp     x19, x20, [sp], 16
    ldp     x21, x22, [sp], 16
    ldp     x23, x24, [sp], 16
    ldp     x25, x26, [sp], 16
    ldp     x27, x28, [sp], 16
    ldp     x29, x30, [sp], 16
    eret


.macro panic
        .align  7
1:  stp     x29, x30, [sp, -16]!
    mov     x29, 1b - vectors_el2
    b .Lpanic
    ldp     x29, x30, [sp], 16
    eret
.endm

/* Vector table entries are 128 bytes in size, i.e. 32 instructions.
 *  (armv7 had 4 bytes per entry (1 instruction)).
 * There are 16 entries in the table.
 * This is 4 types of exceptions from 4 sources.
 * The 4 sources are:
 *  Same exception level when source SP = SP_EL0
 *  Same exception level when source SP = SP_ELx
 *  Lower exception level running on AArch64
 *  Lower exception level running on AArch32
 */

        .text
        .align 11
vectors_el2:
		panic /* 0x000 Current EL w/ SP0 - Synchronous Thread */

		/* 0x080 Current EL w/ SP0 - IRQ */
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

        panic /* 0x100 Current EL w/ SP0 - FIQ Thread */
        panic /* 0x180 Current EL w/ SP0 - Error Thread */

        panic /* 0x200 Current EL w/ SPx - Synchronous Handler */
        ; panic /* 0x280 Current EL w/ SPx - IRQ Handler */
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
        panic /* 0x300 Current EL w/ SPx - FIQ Handler */
        panic /* 0x380 Current EL w/ SPx - Error Handler */

        panic /* 0x400 Lower EL 64 bit - Synch */
		panic /* 0x480 Lower EL 64 bit - IRQ */
		panic /* 0x500 Lower EL 64 bit - FIQ */
		panic /* 0x580 Lower EL 64 bit - Error */

		panic /* 0x600 Lower EL 32 bit - Synch */
		panic /* 0x680 Lower EL 32 bit - IRQ */
		panic /* 0x700 Lower EL 32 bit - FIQ */
		panic /* 0x780 Lower EL 32 bit - Error */


// THE END

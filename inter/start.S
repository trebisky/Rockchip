/* Extremely simple (and virtually unnecessary)
 * assembly language startup file.
 */

	.global start
start:
# this is for arm64
	msr		DAIFSet, #7		// disable interrupts

	// Clear BSS
        ldr             x1, =__bss_start__
        ldr             x2, =__bss_end__

clear_bss_loop:
        str             xzr, [x1], #8
        cmp             x1, x2
        bls             clear_bss_loop  // if x1 <= x2 goto clear_bss_loop

	bl		main

spin:	b		spin


// THE END

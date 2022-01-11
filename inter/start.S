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

# This will fault unless we are in secure mode
# And indeed we do (we get a synch abort, as usual.
# so we are not running in secure mode, which is good
# and as expected.
.global check_secure
check_secure:
	msr             SCR_EL3, x0
	ret


// THE END

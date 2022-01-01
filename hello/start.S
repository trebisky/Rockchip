/* Extremely simple (and virtually unnecessary)
 * assembly language startup file.
 */

	.global start
start:
# this is for arm64
	msr		DAIFSet, #7		// disable interrupts

	bl		main

spin:	b		spin


// THE END

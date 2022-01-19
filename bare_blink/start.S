/* Extremely simple (and almost unnecessary)
 * assembly language startup file.
 *
 * This is aarch64 (arm64) code
 */

	.global start
start:
	b	next		// overwritten by mkrock
next:
	msr	DAIFSet, #7		// disable interrupts

	bl	main

spin:	b	spin


// THE END

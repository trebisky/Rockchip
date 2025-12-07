/* A bare metal program for the Rockchip RK3399
 *
 * The idea here is to get timer interrupts.
 * It has become a trash-can collection for a
 * variety of different experiments.
 *
 * I first wrote this back in 2021 and gave up on it.
 * After learning about EL2 and EL1 interrupt routing
 * in 11-2025 (working with the RK3328)
 * I came back to this.
 *
 * Tom Trebisky  12-31-2021, 12-6-2025
 */

#include "protos.h"

/* This might give about a 1 second delay */
void
delay ( void )
{
        volatile int count = 100000000;

		/* Adjust for when we change the CPU
		 * from 600 to 100 Mhz.
		 */
		count /= 6;

        while ( count-- )
            ;
}

void
blinker ( void )
{
	/* Setting the bit does turn the LED on */
	for ( ;; ) {
	    led_on ();
	    delay ();
	    led_off ();
	    delay ();
	}
}

void
show_stack ( int recursion )
{
	int dummy;

	show_reg ( "Stack: ", &dummy );
	if ( recursion )
	    show_stack ( 0 );
}

void
examine ( long addr )
{
	int *ip = (int *) addr;
	int val;

	printf ( "Examine %h\n", addr );
	val = *ip;
	printf ( " value = %h\n", val );
}

static inline unsigned int
get_el(void)
{
        unsigned int val;

        asm volatile("mrs %0, CurrentEL" : "=r" (val) : : "cc");
        return val >> 2;
}

// I'm not sure what I was up to here.
static inline void
check_shift(void)
{
        unsigned int x, y;

	x = 0xabc0;
	printf ( "Shift input: %h\n", x );
        // asm volatile("mrs %0, CurrentEL" : "=r" (val) : : "cc");
	// asm volatile ( "lsr     w1, w6, #0" : : : "cc" );
	// asm("mov %0, %1, ror #1" : "=r" (result) : "r" (value));
	asm volatile ( "lsr     %0, %1, #0" : "=r" (y) : "r" (x) : "cc" );
	printf ( "Shift result: %h\n", y );
}

/* These only go inline with gcc -O of some kind */
static inline void
INT_unlock ( void )
{
        asm volatile("msr DAIFClr, #3" : : : "cc");
}

static inline void
INT_lock ( void )
{
        asm volatile("msr DAIFSet, #3" : : : "cc");
}

/* We enable both IRQ and FIQ
 * IRQ = 0x1, RIQ = 0x2
 */
static inline void
enable_irq ( void )
{
    //COMPILER_BARRIER();
    //write_daifclr(DAIF_IRQ_BIT);
    //isb();
    asm volatile ("" ::: "memory");
    asm volatile ( "msr DAIFClr, #3" : : : "cc");
    asm volatile ( "isb" );
}

/* Here is the "fix" to get interrupts to work
 * This was added 12-6-2025 after figuring this out
 * working with the RK3328.
 */
void
inter_fixup ( void )
{
		unsigned long lval;

		asm volatile("mrs %0, hcr_el2" : "=r" (lval) : : "cc");
		/* XXX XXX -- grab FMO, IMO */
        lval |= 0x18;
        asm volatile("msr hcr_el2, %0" : : "r" (lval) : "cc");
}

void
main ( void )
{
	uart_init ();
	gpio_init ();

	printf ( "\n" );
	printf ( "Inter demo for RK3399  1-3-2022\n" );

	/* Drop the CPU clock from 600 to 100 Mhz
	 * to reduce how hot the chip gets.
	 */
	cpu_clock_100 ();

	/* Added 12-6-2025 */
	inter_fixup ();

	gic_init ();
	gic_cpu_init ();

	timer_init ();

	// INT_unlock ();
	enable_irq ();

	/* This will check the stack address */
	show_stack ( 1 );

#ifdef MMU_EXPERIMENTS
	// examine ( 0xfffd0000 );	/* gets synch abort */
	// examine ( 0xffff0000 );	/* gets synch abort */
	// examine ( 0xff930000 );		/* MMU */

	/* We try disabling the MMU to see if that allows us
	 * to access the ROM memory addresses.
	 * It does not.
	 */
	// mmu_init ();

	// examine ( 0xffff0000 );	/* gets synch abort */
	// examine ( 0xfffd0000 );	/* gets synch abort */
#endif

	// This gets a fault, proving we are not in secure mode.
	// check_secure ();

	// This returns 2, which is a bit surprising,
	// but apparently some kind of hypervisor is
	// running at EL3.
	// (Actually BL31 is running as a secure monitor
	//  at EL3).
	printf ( "Current EL: %h\n", get_el() );

	// check_shift ();

#ifdef notdef
	for ( ;; ) {
	    // timer_show ();
	    delay ();
	}
#endif

	pll_test ();

	printf ( "Blinking ...\n" );
	/* This will run the blink demo */
	blinker ();

	/* NOTREACHED */
}

/*
 * -------------------
 * Various exception handlers.
 */

void
handle_int ( void )
{
	int irq;

	irq = intcon_irqwho ();
	printf ( "IRQ Interrupt for IRQ %d\n", irq );

	intcon_irqack ( irq );
}

void
handle_bad ( int who )
{
    printf ( "Bad exception: %d\n", who );
    spin ();
}

void
handle_sync ( int a, int b )
{
    printf ( "Synch exception: %d %d\n", a, b );
    // spin ();
}

/* THE END */

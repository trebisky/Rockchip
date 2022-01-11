/* First bare metal program for the Rockchip RK3399
 *
 * Tom Trebisky  12-31-2021
 */

#include "protos.h"

/* This might give about a 1 second delay */
void
delay ( void )
{
        volatile int count = 100000000;

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

void
main ( void )
{
	uart_init ();
	gpio_init ();
	gic_init ();

	printf ( "Inter demo for RK3399  1-3-2022\n" );

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
	printf ( "Current EL: %h\n", get_el() );

	printf ( "Blinking ...\n" );
	/* This will run the blink demo */
	blinker ();

	/* NOTREACHED */
}

/* THE END */

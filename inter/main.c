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



void
main ( void )
{
	uart_init ();
	gpio_init ();

	printf ( "\n" );
	printf ( "Inter demo for RK3399  1-3-2022\n" );

	gic_init ();
	timer_init ();

	INT_unlock ();

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
	check_shift ();

	for ( ;; ) {
	    // timer_show ();
	    delay ();
	}

	printf ( "Blinking ...\n" );
	/* This will run the blink demo */
	blinker ();

	/* NOTREACHED */
}

/* THE END */

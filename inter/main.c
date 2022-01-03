/* First bare metal program for the Rockchip RK3399
 *
 * Tom Trebisky  12-31-2021
 */

#include "protos.h"

int limit = 200;

void
talker ( void )
{
	if ( ! limit ) {
	    for ( ;; )
		uart_puts ( "hello " );
	} else {
	    for ( ;limit--; )
		uart_puts ( "hello " );
	}
}

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

int bss_test[128];

void
verify_bss ( void )
{
	int i;
	int bss_bad = 0;

	for ( i=0; i<128; i++ ) {
	    if ( bss_test[i] ) {
		printf ( "BSS %d: %h\n", i, bss_test[i] );
		bss_bad = 1;
	    }
	}
	if ( ! bss_bad )
	    printf ( "BSS ok\n" );
}

void
main ( void )
{
	uart_init ();
	gpio_init ();

	/* This will run the hello demo */
	// talker ();

	/* This will check the stack address */
	show_stack ( 1 );
	verify_bss ();

	printf ( "Blinking ...\n" );
	/* This will run the blink demo */
	blinker ();

	/* NOTREACHED */
}

/* THE END */

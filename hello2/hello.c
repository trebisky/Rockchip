/* First bare metal program for the Rockchip RK3399
 *
 * Tom Trebisky  12-31-2021
 */

typedef volatile unsigned int vu32;
typedef unsigned int u32;

#define BIT(x)	(1<<(x))

void uart_init ( void );
void puts ( char * );

int limit = 500;

void
main ( void )
{
	uart_init();

	if ( ! limit ) {
	    for ( ;; )
		puts ( "hello " );
	} else {
	    for ( ;limit--; )
		puts ( "hello " );
	}

	/* NOTREACHED */
}

/* THE END */

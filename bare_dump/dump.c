/* Truly bare metal program for the Rockchip RK3399
 * This is launched directly by the bootrom.
 *
 * Tom Trebisky  1-18-2022
 */

/* When I first ran this, the green LED simply came on,
 * which was lucky for me, otherwise I would have just
 * thought the demo had failed.  With this count, it
 * stays on for 45 seconds!  So the CPU is either just
 * running about 100 times slower, or caches are not
 * enabled or something of the sort.
 */
// #define DELAY_COUNT	100000000

/* This gives something like a 1 hz blink rate.
 */
#define DELAY_COUNT	1000000

typedef volatile unsigned int vu32;
typedef unsigned int u32;

#define BIT(x)	(1<<(x))

void uart_init ( void );
void uart_puts ( char * );

void printf ( char *, ... );
void show_reg ( char *, int * );

/* The TRM describes the GPIO in chapter 20
 *
 * The on chip LED is connected to GPIO0_B3
 */

struct rock_gpio {
	vu32		data;
	vu32		dir;
	u32		_pad0[2];

	u32		_pad1[32];

	vu32		ie;		/* 0x30 */
	vu32		im;		/* 0x34 */
	vu32		il;		/* 0x38 */
	vu32		ip;		/* 0x3c */

	vu32		is;		/* 0x40 */
	vu32		ris;		/* 0x44 */
	vu32		debounce;	/* 0x48 */
	vu32		eoi;		/* 0x4c */

	vu32		ext;		/* 0x50 */
	u32		_pad2[3];

	vu32		sync;		/* 0x60 */
};

#define GPIO0_BASE	((struct rock_gpio *) 0xff720000)
#define GPIO1_BASE	((struct rock_gpio *) 0xff730000)

#define GPIO2_BASE	((struct rock_gpio *) 0xff780000)
#define GPIO3_BASE	((struct rock_gpio *) 0xff788000)
#define GPIO4_BASE	((struct rock_gpio *) 0xff790000)

#define GPIO_BASE	GPIO0_BASE

void
delay ( void )
{
        volatile int count = DELAY_COUNT;

        while ( count-- )
            ;
}

/* The LED is on B3
 */
#define LED_BIT		(8+3)
#define LED_MASK	BIT(LED_BIT)

/* This version is polite and doesn't disturb any bits other
 * than the one of interest.
 */
void
blinker ( void )
{
	struct rock_gpio *gp = GPIO_BASE;

	gp->dir |= LED_MASK;

	/* Setting the bit does turn the LED on */
	for ( ;; ) {
	    gp->data |= LED_MASK;
	    delay ();
	    gp->data &= ~LED_MASK;
	    delay ();
	}
}

void
led_on ( void )
{
	struct rock_gpio *gp = GPIO_BASE;

	gp->dir |= LED_MASK;
	gp->data |= LED_MASK;
}

void
show_stack ( int recursion )
{
        int dummy;

        show_reg ( "Stack: ", &dummy );
        if ( recursion )
            show_stack ( 0 );
}

// int limit = 500;
int limit = 0;

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

	/* NOTREACHED */
}

void
dump_add ( u32 *addr )
{
	unsigned long ia = (unsigned long) addr;
	u32 val = 0;

	printf ( "Dump at %h: %h\n", ia, val );
	val = *addr;
	printf ( "Dump at %h: %h\n", ia, val );
}

void
main ( void )
{
	uart_init();
	led_on ();
	printf ( "\n" );

	// talker ();

	show_stack ( 1 );
	/* This shows the stack at 0xff8c1f3c,
	 * growing to lower addresses.
	 */

	dump_add ( (u32 *) 0xffff0000 );
	dump_add ( (u32 *) 0xfffd0000 );

	printf ( "Blinking ...\n" );
	blinker ();

	/* NOTREACHED */
}

/* THE END */

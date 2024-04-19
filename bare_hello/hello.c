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
void puts ( char * );

/* The TRM describes the GPIO in chapter 20
 *
 * The on chip LED is connected to GPIO0_B3
 */

struct rock_gpio {
	vu32		data;
	vu32		dir;
	u32		_pad0[2];

	u32		_pad1[8];

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

// int limit = 500;
int limit = 0;

void
talker ( void )
{

	if ( ! limit ) {
	    for ( ;; )
		puts ( "hello " );
	} else {
	    for ( ;limit--; )
		puts ( "hello " );
	}

	/* NOTREACHED */
}

void
main ( void )
{
	uart_init();
	led_on ();

	talker ();

	blinker ();

	/* NOTREACHED */
}

/* THE END */

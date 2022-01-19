/* Truly bare metal program for the Rockchip RK3399
 * This is launched directly by the bootrom.
 *
 * Tom Trebisky  1-18-2022
 */

typedef volatile unsigned int vu32;
typedef unsigned int u32;

#define BIT(x)	(1<<(x))

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

/* This might give about a 1 second delay */
void
delay ( void )
{
        volatile int count = 100000000;

        while ( count-- )
            ;
}

/* This works, but manipulates all 32 bits.
 * setting the direction to output (1) is essential
 */
void
blinker_BRUTE ( void )
{
	struct rock_gpio *gp = GPIO_BASE;

	gp->dir = 0xffffffff;

	for ( ;; ) {
	    // puts ( "on\n" );
	    gp->data = 0xffffffff;
	    delay ();
	    // puts ( "off\n" );
	    gp->data = 0;
	    delay ();
	}
}

/* Trial and error to find the bit (since I was lied to and
 *  told it was B5 originally
 */
// #define LED_MASK	0xff<<8	/* yes */
// #define LED_MASK	0xf0<<8	/* no */
// #define LED_MASK	0x0f<<8	/* yes */
// #define LED_MASK	0x03<<8	/* no */
// #define LED_MASK	0x0c<<8	/* yes */
// #define LED_MASK	0x04<<8	/* no */
// #define LED_MASK	0x08<<8	/* yes */

/* This looks more like B3 to me */
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
main ( void )
{
	/* This will run the blink demo */
	blinker ();

	/* NOTREACHED */
}

/* THE END */

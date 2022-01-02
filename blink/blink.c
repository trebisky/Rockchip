/* First bare metal program for the Rockchip RK3399
 *
 * Tom Trebisky  12-31-2021
 */

typedef volatile unsigned int vu32;
typedef unsigned int u32;

#define BIT(x)	(1<<(x))

/* The TRM describes the UART in chapter 19
 */
struct rock_uart {
	vu32	data;
	vu32	ier;
	vu32	iir;
	vu32	lcr;

	vu32	mcr;
	vu32	lsr;
	vu32	msr;
	vu32	scr;

	u32	_pad0[4];

	vu32	srbr;		/* 0x30 */
	u32	_pad1[3];

	u32	_pad2[12];

	vu32	far;		/* 0x70 */
	vu32	tfr;		/* 0x74 */
	vu32	rfw;		/* 0x78 */
	vu32	status;		/* 0x7c */

	vu32	tfl;		/* 0x80 */
	vu32	rfl;		/* 0x84 */
	vu32	srr;		/* 0x88 */
	vu32	srts;		/* 0x8c */

	vu32	sbcr;		/* 0x90 */
	vu32	sdmam;		/* 0x94 */
	vu32	sfe;		/* 0x98 */
	vu32	srt;		/* 0x9c */

	vu32	stet;		/* 0xa0 */
	vu32	htx;		/* 0xa4 */
	vu32	dmasa;		/* 0xa8 */
	u32	_pad3;		/* 0xac */

	u32	_pad4[16];

	u32	_pad5;		/* 0xf0 */
	vu32	cpr;		/* 0xf4 */
	vu32	ucv;		/* 0xf8 */
	vu32	ctr;		/* 0xfc */

};

#define UART0_BASE	((struct rock_uart *) 0xff180000)
#define UART1_BASE	((struct rock_uart *) 0xff190000)
#define UART2_BASE	((struct rock_uart *) 0xff1a0000)
#define UART3_BASE	((struct rock_uart *) 0xff1b0000)

#define UART4_BASE	((struct rock_uart *) 0xff370000)

#define UART_BASE	UART2_BASE

/* Bits in the status register */
#define ST_BUSY		BIT(0)
#define ST_TNF		BIT(1)
#define ST_TE		BIT(2)
#define ST_RNE		BIT(3)
#define ST_RF		BIT(4)

/* The TRM describes the GPIO in chapter 20
 *
 * The on chip LED is connected to a signal GPIO0_B5
 * according to the schematic.
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
uart_init ( void )
{
	/* nothing here, we rely on U-Boot */
}

void
uart_putc ( char c )
{
	struct rock_uart *up = UART_BASE;

	while ( ! (up->status & ST_TNF) )
	    ;

	up->data = c;
}

/* Portable code below here */

void
puts ( char *s )
{
	while ( *s ) {
	    if (*s == '\n')
		uart_putc('\r');
	    uart_putc(*s++);
	}
}

int limit = 500;

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
}

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
	    puts ( "on\n" );
	    gp->data = 0xffffffff;
	    delay ();
	    puts ( "off\n" );
	    gp->data = 0;
	    delay ();
	}
}


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

void
blinker ( void )
{
	struct rock_gpio *gp = GPIO_BASE;

	gp->dir = LED_MASK;

	for ( ;; ) {
	    puts ( "on\n" );
	    gp->data = LED_MASK;
	    delay ();
	    puts ( "off\n" );
	    gp->data = 0;
	    delay ();
	}
}


void
main ( void )
{
	uart_init();

	/* This will run the hello demo */
	// talker ();

	/* This will run the blink demo */
	blinker ();

	/* NOTREACHED */
}

/* THE END */

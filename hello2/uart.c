/* RK3399 - uart.c
 *
 * Tom Trebisky  1-18-2022
 */

typedef volatile unsigned int vu32;
typedef unsigned int u32;
typedef unsigned long u64;

#define BIT(x)	(1<<(x))

/* The TRM keeps register details in Appendix B
 */
struct rock_uart {
	vu32	data;
	vu32	ier;
#define		dll	data
#define		dlh	ier
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

/* Bits in the SRR register */
#define SRR_RESET_UART	0x01
#define SRR_RESET_RFIFO	0x02
#define SRR_RESET_TFIFO	0x04

#define UART_CLOCK	24000000

/* Baud rates possible are:
 * 115.2Kbps, 460.8Kbps, 921.6Kbps, 1.5Mbps, 3Mbps, 4Mbps.
 */

/* now for the pin control (iomux) settings.
 * For UART2, the pins are on GPIO4B
 */
#define GRF_BASE		0xff770000
#define GRF_GPIO4B_IOMUX        0xe024

void
io_write ( u64 base, u64 offset, u32 value )
{
	u32 *addr = (u32 *) (base + offset);

	*addr = value;
}

/* We need to do this for UART2 which is our console.
 * see page 204 in the TRM.
 * Note that GRF is "general register files".
 * There is also a PMUGRF.
 * On page 55 in the datasheet we see:
 * GPIO_B0 is our uart Rx when func 3 is selected
 * GPIO_B1 is our uart Tx when func 3 is selected
 * This particular GRF register is described on TRM page 309.
 * The upper 16 bits are used to enable writes to the
 * lower 16 bits.  Hence the mask of "3".
 * Only 6 pins (GPIO4B[0] to GPIO4B[5]) are controlled by
 * this register.  Each pin has 2 bits to control it.
 * We mess with bits 0 and 1, setting the value of both to 2.
 * Since this comment is becoming a tutorial on this, here are
 * the 4 possible settings for those 2 bits:
 * 0 = gpio
 * 1 = sdmmc
 * 2 = uart2
 * 3 = reserved or JTAG
 */
void
uart_iomux ( void )
{
	// rk_iomux_config(RK_UART0_IOMUX + ch);
	// rk_uart_iomux_config(iomux_id);
	// grf_writel((3 << 18) | (3 << 16) | (2 << 2) | (2 << 0), GRF_GPIO4B_IOMUX);
	// #define grf_writel(v, offset)   do { writel(v, RKIO_GRF_PHYS + offset); } while (0)
	// #define RKIO_GRF_PHYS                   0xFF770000
	// #define GRF_GPIO4B_IOMUX        0xe024
	io_write ( GRF_BASE, GRF_GPIO4B_IOMUX, 
	    (3 << 18) | (3 << 16) | (2 << 2) | (2 << 0) );
}

#define LCR_DLAT	0x80

#define BAUD_MODE_X	16

void
uart_baud ( int baud )
{
	struct rock_uart *up = UART_BASE;
	u32 rate;

	rate = UART_CLOCK / BAUD_MODE_X / baud;

	up->lcr |= LCR_DLAT;

	up->dll = rate & 0xff;
	up->dlh = (rate >> 8 ) & 0xff;

	up->lcr &= ~LCR_DLAT;
}

void
uart_init ( void )
{
	struct rock_uart *up = UART_BASE;

	uart_iomux ();

	/* -- reset the uart and both fifos */
	up->srr = SRR_RESET_UART | SRR_RESET_RFIFO | SRR_RESET_TFIFO;

	/* -- disable interrupts */
	up->ier = 0;

	// val = rk_uart_set_iop(base, IRDA_SIR_DISABLED);
	// writel(irda, base + UART_MCR);
	up->mcr = 0;

#define LCR_DATA8	0x3
	// val = rk_uart_set_lcr(base, UART_BIT8, PARITY_DISABLED, ONE_STOP_BIT);
	/* Parity is disabled and we get one stop bit when zeros are set
	 */
	up->lcr = LCR_DATA8;

	// val = rk_uart_set_baudrate(base, baudrate);
	uart_baud ( 1500000 );

	// rk_uart_set_fifo(base);
	up->sfe = 1;	/* enable fifos via shadow register */
	up->srt = 1;	/* rcvr trigger via shadown register */
	up->stet = 1;	/* tx empty trigger via shadown register */
}

void
uart_putc ( char c )
{
	struct rock_uart *up = UART_BASE;

	while ( ! (up->status & ST_TNF) )
	    ;

	up->data = c;
}

void
puts ( char *s )
{
	while ( *s ) {
	    if (*s == '\n')
		uart_putc('\r');
	    uart_putc(*s++);
	}
}

/* THE END */

/* GPIO driver for the Rockchip RK3399
 *
 * Tom Trebisky  1-3-2022
 */

#include "protos.h"

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

/* The LED is on GPIO0, B3 */
#define LED_BIT		(8+3)
#define LED_MASK	BIT(LED_BIT)

void
gpio_init ( void )
{
	struct rock_gpio *gp = GPIO_BASE;

	/* Make this bit an output */
	gp->dir |= LED_MASK;
}

/* Setting the bit does turn the LED on */
void
led_on ( void )
{
	struct rock_gpio *gp = GPIO_BASE;

	// uart_puts ( "LED on\n" );
	gp->data |= LED_MASK;
}

/* For the LED, we are polite and doesn't disturb any bits other
 * than the one of interest.
 */
void
led_off ( void )
{
	struct rock_gpio *gp = GPIO_BASE;

	// uart_puts ( "LED off\n" );
	gp->data &= ~LED_MASK;
}

/* THE END */

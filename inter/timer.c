/* Simple serial driver for the Rockchip RK3399
 *
 * Tom Trebisky  1-2-2022
 */

#include "protos.h"
#include "rk3399_ints.h"

typedef volatile unsigned int vu32;
typedef unsigned int u32;

#define BIT(x)	(1<<(x))

/* It looks like the RK3399 has 12 timers
 * (and another 12 for secure mode to use).
 * There are also 2 PMU timers.
 *
 * This driver is for the 12 non-secure timers
 *
 * See chapter 10 of the TRM (page 556)
 */
#define TIMER_CLOCK	24000000

struct rk_timer {
	vu32	count0;
	vu32	count1;
	vu32	val0;
	vu32	val1;
	vu32	count2;
	vu32	count3;
	vu32	intstatus;
	vu32	control;
};

#define CTRL_ENA	BIT(0)
#define CTRL_MODE	BIT(1)
#define CTRL_INTENA	BIT(2)

#define TIMER0_BASE	((struct rk_timer *) 0xff850000)
#define TIMER6_BASE	((struct rk_timer *) 0xff858000)

#define SYS_TIMER 	0
#define SYS_TIMER_IRQ	IRQ_TIMER0

/* The TRM is entirely vague on how 6 timers are arranged within
 * the address block given by the base address, but experiment
 * shows that it is a contiguous array at the start of the block.
 */

/* The timer counts up until it reaches a value, then interrupts.
 */

static struct rk_timer *cur_timer;

void
timer_handler ( void )
{
	struct rk_timer *tp = cur_timer;

	/* clear the interrupt */
	tp->intstatus = 1;
	printf ( "Tick\n" );
}

void
timer_setup ( int t, u32 val )
{
	struct rk_timer *tp;

	if ( t > 11 )
	    return;
	if ( t < 6 )
	    tp = &TIMER0_BASE[t];
	else
	    tp = &TIMER6_BASE[t-6];
	cur_timer = tp;

	tp->control &= ~CTRL_ENA;
	tp->control &= ~CTRL_MODE;	/* free run */
	tp->control |= CTRL_INTENA;
	tp->count2 = 0;			/* 2,3 are start value */
	tp->count3 = 0;
	tp->count0 = val;		/* 0,1 are end value */
	tp->count1 = 0;
	tp->control |=  CTRL_ENA;

	intcon_ena ( SYS_TIMER_IRQ );

	printf ( "Timer: %d  %h %h\n", tp->intstatus, tp->count1, tp->count0 );
}

void
timer_show ( void )
{
	struct rk_timer *tp = cur_timer;
	u32 hi, lo;

	hi = tp->val1;
	lo = tp->val0;
	printf ( "Timer: %d  %h %h\n", tp->intstatus, hi, lo );
}

void
timer_init ( void )
{
	u32 val;
	struct rk_timer *tp = TIMER0_BASE;

	val = TIMER_CLOCK / 2;

	timer_setup ( SYS_TIMER, val );
}

/* THE END */

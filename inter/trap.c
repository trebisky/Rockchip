/* trap.c */

//#include "types.h"

#include "protos.h"
#include "rk3399_ints.h"

unsigned int GetCurrentSMode(void);
unsigned int GetCPUID(void);

// void psciHandler(unsigned long*);

void
boardReset ( void )
{
	for ( ;; ) ;
}

static void
handle_irq ( void )
{
	int n;

	n = intcon_irqwho();
	printf ( " !! IRQ: %d\n", n );
	if ( n == IRQ_TIMER0 )
	    timer_handler ();
	else
	    printf ( "IRQ: %d\n", n );
	intcon_irqack ( n );
}

/* exc is offset of the exception
 * esr is the esr (exception syndrome register)
 * regs is the regs
 */
void
sync_handler ( unsigned exc, unsigned esr, unsigned long *regs )
{
	unsigned int class = esr >> 26;		/* 31:26 */
	unsigned int syndrome = esr & 0x1ffffff;	/* 24:0 */

	switch( exc ) {
	    case 0x280:     // IRQ
		// gic_clear ();
		handle_irq ();
		return;
	    case 0x400:
		/* Synch from lower level */
		if( class == 0x17 ) {   // smc
		    if( syndrome == 0 ) {
			// psciHandler(regs);
			return;
		    }
		}
	}

	printf("\n--- PANIC! exception 0x%x on CPU%d EL %d, class=0x%x syndrome=0x%x\n",
            exc, GetCPUID(), GetCurrentSMode(), class, syndrome );

    boardReset();
}

/* THE END */

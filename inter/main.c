/* A bare metal program for the Rockchip RK3399
 *
 * The idea here is to get timer interrupts.
 * It has become a trash-can collection for a
 * variety of different experiments.
 *
 * I first wrote this back in 2021 and gave up on it.
 * After learning about EL2 and EL1 interrupt routing
 * in 11-2025 (working with the RK3328)
 * I came back to this.
 *
 * Tom Trebisky  12-31-2021, 12-6-2025
 */

#include "protos.h"

/* This might give about a 1 second delay */
void
delay ( void )
{
        volatile int count = 100000000;

		/* Adjust for when we change the CPU
		 * from 600 to 100 Mhz.
		 */
		count /= 6;

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

void
examine ( long addr )
{
	int *ip = (int *) addr;
	int val;

	printf ( "Examine %h\n", addr );
	val = *ip;
	printf ( " value = %h\n", val );
}

static inline unsigned int
get_el(void)
{
        unsigned int val;

        asm volatile("mrs %0, CurrentEL" : "=r" (val) : : "cc");
        return val >> 2;
}

// I'm not sure what I was up to here.
static inline void
check_shift(void)
{
        unsigned int x, y;

	x = 0xabc0;
	printf ( "Shift input: %h\n", x );
        // asm volatile("mrs %0, CurrentEL" : "=r" (val) : : "cc");
	// asm volatile ( "lsr     w1, w6, #0" : : : "cc" );
	// asm("mov %0, %1, ror #1" : "=r" (result) : "r" (value));
	asm volatile ( "lsr     %0, %1, #0" : "=r" (y) : "r" (x) : "cc" );
	printf ( "Shift result: %h\n", y );
}

/* These only go inline with gcc -O of some kind */
static inline void
INT_unlock ( void )
{
        asm volatile("msr DAIFClr, #3" : : : "cc");
}

static inline void
INT_lock ( void )
{
        asm volatile("msr DAIFSet, #3" : : : "cc");
}

/* We enable both IRQ and FIQ
 * IRQ = 0x1, RIQ = 0x2
 */
static inline void
enable_irq ( void )
{
    //COMPILER_BARRIER();
    //write_daifclr(DAIF_IRQ_BIT);
    //isb();
    asm volatile ("" ::: "memory");
    asm volatile ( "msr DAIFClr, #3" : : : "cc");
    asm volatile ( "isb" );
}

/* Here is the "fix" to get interrupts to work
 * This was added 12-6-2025 after figuring this out
 * working with the RK3328.
 */
void
inter_fixup ( void )
{
		unsigned long lval;

		asm volatile("mrs %0, hcr_el2" : "=r" (lval) : : "cc");
		/* XXX XXX -- grab FMO, IMO */
        lval |= 0x18;
        asm volatile("msr hcr_el2, %0" : : "r" (lval) : "cc");
}

/* The arguments are in x0, x1 -- so this works.
 * A lot of serendipity though.
 */
void
try_syscall ( int a, int b )
{
        asm volatile("svc #0x123" );
}

#ifdef BSS_TEST
int zero;
int pickle = 12399;
#endif

void
main ( void )
{
	int core;
	unsigned long lval;
	int aff;

	uart_init ();
	gpio_init ();

	printf ( "\n" );
	// printf ( "Inter demo for RK3399  1-3-2022\n" );
	printf ( "Inter demo for RK3399  12-10-2025\n" );

#ifdef BSS_TEST
	/* See if global variables get initialized
	 * as they should.
	 */
	printf ( "zero = %d\n", zero );
	printf ( "pickle = %d\n", pickle );
	pickle = 321;
	printf ( "pickle = %d\n", pickle );
	/* I am pleased and surprised that pickle gets
	 * handled properly, with no effort on my part.
	 */
#endif

	/* Drop the CPU clock from 600 to 100 Mhz
	 * to reduce how hot the chip gets.
	 * This had no noticeable effect.
	 * This chip just runs hot.
	 */
	printf ( "Reduce CPU clock to 100 Mhz\n" );
	cpu_clock_100 ();

	/* Added 12-6-2025 */
	printf ( "Fixing HCR so EL2 gets interrupts\n" );
	inter_fixup ();

	intcon_gic_init ();
	intcon_gic_cpu_init ();

	timer_init ();

	printf ( "Enable interrupts\n" );
	// INT_unlock ();
	enable_irq ();

	/* This will check the stack address */
	// show_stack ( 1 );

#ifdef MMU_EXPERIMENTS
	// examine ( 0xfffd0000 );	/* gets synch abort */
	// examine ( 0xffff0000 );	/* gets synch abort */
	// examine ( 0xff930000 );		/* MMU */

	/* We try disabling the MMU to see if that allows us
	 * to access the ROM memory addresses.
	 * It does not.
	 */
	// mmu_init ();

	// examine ( 0xffff0000 );	/* gets synch abort */
	// examine ( 0xfffd0000 );	/* gets synch abort */
#endif

	// This gets a fault, proving we are not in secure mode.
	// check_secure ();

	// This returns 2, which is a bit surprising,
	// but apparently some kind of hypervisor is
	// running at EL3.
	// (Actually BL31 is running as a secure monitor
	//  at EL3).
	printf ( "Current EL: %h\n", get_el() );

	asm volatile("mrs %0, mpidr_el1" : "=r" (lval) : : "cc");
	printf ( "MPidr_el1 = %Y\n", lval );

	// No such register (there is a vmpidr_el2)
	// asm volatile("mrs %0, mpidr_el2" : "=r" (lval) : : "cc");
	// printf ( "MPidr_el2 = %Y\n", lval );

	core = lval & 0xff;
	printf ( "Core %d\n", core );

	/* See A53 TRM for these */
	/* I see: 00000000_80000000 */
	/* the "8" is strange, this is a reserved bit */
	aff = (lval>>32) & 0xff;
	printf ( "Aff3 %x\n", aff );	/* cluster-3 */
	aff = (lval>>16) & 0xff;
	printf ( "Aff2 %x\n", aff );	/* cluster-2 */
	aff = (lval>>16) & 0xff;
	printf ( "Aff1 %x\n", aff );	/* cluster-1 */
	aff = lval & 0xff;
	printf ( "Aff0 %x\n", aff );	/* core/cpu */

	/* This should produce a synchronous exception */
	try_syscall ( 99, 123 );

	/* Should produce an SGI interrupt */
	intcon_sgi ( 0 );

	// check_shift ();

#ifdef notdef
	for ( ;; ) {
	    // timer_show ();
	    delay ();
	}
#endif

	// pll_test ();

	printf ( "Blinking ...\n" );
	/* This will run the blink demo */
	blinker ();

	/* NOTREACHED */
}

/*
 * -------------------
 * Various exception handlers.
 */

void
handle_irq ( void )
{
	int irq;

	printf ( "IRQ interrupt\n" );
	irq = intcon_irqwho ();
	printf ( "IRQ Interrupt for IRQ %d\n", irq );

	intcon_irqack ( irq );
}

void
handle_fiq ( void )
{
	int irq;

	printf ( "FIQ interrupt\n" );
	irq = intcon_irqwho ();
	printf ( "FIQ Interrupt for IRQ %d\n", irq );

	intcon_irqack ( irq );
}

/* We call this rkpanic() to avoid conflict with
 * the panic() macro in the gic driver setup.
 */
void
rkpanic ( char *msg )
{
	printf ( "Panic (%s)\n", msg );
	printf ( "spinning\n" );
	spin ();
}

void
handle_bad ( int who )
{
    printf ( "Bad exception: %d\n", who );
	rkpanic ( "bad exception" );
}

/* The ESR is the Exception Syndrome Register.
 * it can be decoded to find out exactly what
 * sort of synchronous exception occured.
 * The low 24 bits depend on the cause.
 * For our SVC instruction, this has the
 * immediate value encoded in the SVC.
 * So for our test of SVC I see:
 * ESR = 56000123
 * The upper 5 bits are EC, which is the
 * exception code.
 * The values used here seem to be a well
 * guarded secret.
 */
void
handle_sync ( int a, int b )
{
	int esr;
	int ec;
	int elr;

	// The args only are interesting for SVC
    // printf ( "Synch exception: %d %d\n", a, b );
    printf ( "Synch exceptionn\n", a, b );
	asm volatile("mrs %0, ESR_el2" : "=r" (esr) : : "cc");
	printf ( "ESR = %X\n", esr );
	ec = esr>>26 & 0x3f;
	printf ( "ESR.EC = %X\n", ec );
	asm volatile("mrs %0, ELR_el2" : "=r" (elr) : : "cc");
	printf ( "ELR = %X\n", elr );
	if ( ec == 0x15 ) {
		printf ( " SVC instruction: %X\n", esr & 0xffff );
		printf ( " SVC args: %d %d\n", a, b );
		return;
	}

	if ( ec == 0x20 | ec == 0x21 ) {
		printf ( "Instruction abort: %X\n", ec );
	} else if ( ec == 0x24 | ec == 0x25 ) {
		printf ( "data abort: %X\n", ec );
	} else {
		printf ( "unknown: %X\n", ec );
	}

	rkpanic ( "Synch abort" );
}

/* THE END */

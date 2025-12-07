/*
 * Copyright (C) 2015  Tom Trebisky  <tom@mmto.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. See README and COPYING for
 * more details.
 *
 * pll.c - driver for the various PLL in the Rockchip RK3399
 *
 * Tom Trebisky  1-3-2022, 2-14-2022, 12-6-2025
 */

#include "protos.h"
// #include "rk3399_ints.h"

typedef volatile unsigned int vu32;
typedef unsigned int u32;
typedef unsigned long u64;

#define BIT(x)	(1<<(x))

#define CRU_BASE	0xff760000

#define L_BASE		(CRU_BASE + 0x00)	/* x */
#define B_BASE		(CRU_BASE + 0x20)	/* x */
#define D_BASE		(CRU_BASE + 0x40)	/* x */
#define C_BASE		(CRU_BASE + 0x60)	/* x */
#define G_BASE		(CRU_BASE + 0x80)	/* x */

#define N_BASE		(CRU_BASE + 0xA0)	/* x */
#define V_BASE		(CRU_BASE + 0xC0)	/* x */

/* The P pll is in the PMUCRU */

/* Each PLL has the same 5 registers as the RK3328,
 * but the stuff in the registers is laid out differently.
 */

/* The RK3399 has 8 PLL - 
 * BPLL LPLL DPLL CPLL GPLL NPLL VPLL PPLL
 */

static void
pll_dump ( char *who, u32 base, int extra )
{
		u32 *p;
		unsigned long l;
		int val;
		int fbdiv, refdiv;
		int postdiv1, postdiv2;
		int foutvco, fout;

		/* Avoid warning */
		l = base;
		p = (u32 *) l;
		printf ( "\n" );
		printf ( "%s CON0 = %X\n", who, *p++ );
		printf ( "%s CON1 = %X\n", who, *p++ );
		printf ( "%s CON2 = %X\n", who, *p++ );
		printf ( "%s CON3 = %X\n", who, *p++ );
		printf ( "%s CON4 = %X\n", who, *p++ );

		p = (u32 *) l;
		printf ( "%s bypass = %d\n", who, (p[0] >>15) & 0x1 );

		if ( ! extra ) return;

		/* CON0 */
		// val = (p[0] >> 12) & 0x7;
		// postdiv1 = val;
		// printf ( "%s postdiv1 = %x %d\n", who, val, val );
		val = p[0] & 0xfff;
		fbdiv = val;
		printf ( "%s fbdiv = %x %d\n", who, val, val );

		/* CON1 */
		// printf ( "%s dsmpd = %d\n", who, (p[1] >>12) & 0x1 );
		// printf ( "%s lock = %d\n", who, (p[1] >>10) & 0x1 );
		val = (p[1] >> 12) & 0x7;
		postdiv2 = val;
		printf ( "%s postdiv2 = %x %d\n", who, val, val );

		val = (p[1] >> 8) & 0x7;
		postdiv1 = val;
		printf ( "%s postdiv1 = %x %d\n", who, val, val );

		val = (p[1] >> 0) & 0x3f;
		refdiv = val;
		printf ( "%s refdiv = %x %d\n", who, val, val );

		/* CON2 */
		printf ( "%s lock = %d\n", who, (p[2] >>31) & 0x1 );

		val = p[2] & 0xffffff;
		printf ( "%s fracdiv = %x %d\n", who, val, val );

		foutvco = 24 * fbdiv / refdiv;
		printf ( "%s   foutvco = %d\n", who, foutvco );

		fout = foutvco / postdiv1 / postdiv2;
		printf ( "%s   fout = %d\n", who, fout );
}

static void
pll_cut ( char *who, u32 base, int cut )
{
		u32 *p;
		unsigned long l;
		u32 val;
		u32 mask;

		/* Avoid warning */
		l = base;
		p = (u32 *) l;

		/* change postdiv2 in CON1 */
		val = (0x7 << (16+12)) | cut<<12;
		p[1] = val;

}

#ifdef RK3328
static void
pll_cut ( char *who, u32 base, int cut )
{
		u32 *p;
		unsigned long l;
		u32 val;
		u32 mask;

		/* Avoid warning */
		l = base;
		p = (u32 *) l;

		/* change postdiv2 in CON0 */
		val = *p;
		mask = 0x3 << 12;
		val &= ~mask;
		val |= cut << 12;
		val |= 0xffff0000;
		*p = val;

		pll_dump ( who, base, 1 );
}
#endif

/* Reduce the CPU clock from the original
 * 600 Mhz to 100 so the chip can run cooler.
 * Here is what the PLL looks like after this.
 * This is confirmed by watching the LED blink rate.
 * We guessed and got lucky the first time.
 *  The L pll generates the ARM cpu clock.
 *  12-6-2025
 *
 *  L fbdiv = 32 50
 *  L postdiv2 = 06 6
 *  L postdiv1 = 02 2
 *  L refdiv = 01 1
 *  L lock = 1
 *  L fracdiv = 1F 799
 *  L   foutvco = 1200
 *  L   fout = 100
 */

void
cpu_clock_100 ( void )
{
		pll_cut ( "L", L_BASE, 6 );
}

void
pll_test ( void )
{
		pll_dump ( "L", L_BASE, 1 );
		pll_dump ( "B", B_BASE, 1 );
		pll_dump ( "D", D_BASE, 1 );
		pll_dump ( "C", C_BASE, 1 );

		pll_dump ( "G", G_BASE, 1 );
		pll_dump ( "N", N_BASE, 1 );
		pll_dump ( "V", V_BASE, 1 );
}

/* THE END */

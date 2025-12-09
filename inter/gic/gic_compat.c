/*
 * gic_compat.c
 *
 * Copyright (C) 2025  Tom Trebisky  <tom@mmto.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. See README and COPYING for
 * more details.
 *
 * This has "glue" routines that relay the calls that
 * my gic400 drivers used to the API used by the ATF
 * driver for the gicv3
 *
 * Tom Trebisky  1-3-2022, 2-14-2022, 12-6-2025
 */

void
gic_init ( void )
{
}

void
gic_cpu_init ( void )
{
}

void
intcon_ena ( int irq )
{
}

void
intcon_dis ( int irq )
{
}

int
intcon_irqwho ( void )
{
}

void
intcon_irqack ( int irq )
{
}

/* ====================================================================== */
/* ====================================================================== */

/* While the above entry points are intended for the outside world
 * (i.e. my code).  The entry points below are new things that I am
 * introducing for the ATF gic driver to call.
 */

/* Called by the panic() macro in debug.h */
void
gic_panic ( void )
{
	/* nothing here yet */
}

/* THE END */

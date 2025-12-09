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

#include "gicv3.h"

#define CPU_0	0

/* These are in gicv3_main.c */

void gicv3_driver_init ( const gicv3_driver_data_t *plat_driver_data );

void gicv3_enable_interrupt(unsigned int id, unsigned int proc_num);
void gicv3_disable_interrupt(unsigned int id, unsigned int proc_num);

unsigned int gicv3_get_pending_interrupt_id ( void );
void gicv3_clear_interrupt_pending ( unsigned int id, unsigned int proc_num );

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
		gicv3_enable_interrupt ( irq, CPU_0 );
}

void
intcon_dis ( int irq )
{
		gicv3_disable_interrupt ( irq, CPU_0 );
}

int
intcon_irqwho ( void )
{
		return gicv3_get_pending_interrupt_id ();

}

void
intcon_irqack ( int irq )
{
		gicv3_clear_interrupt_pending ( irq, CPU_0 );
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

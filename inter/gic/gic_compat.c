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

#include "platform_def.h"
#include "platform.h"

#include "../protos.h"

#define CPU_0	0

/* These are in gicv3_main.c */

void gicv3_driver_init ( const gicv3_driver_data_t *plat_driver_data );

void gicv3_enable_interrupt(unsigned int id, unsigned int proc_num);
void gicv3_disable_interrupt(unsigned int id, unsigned int proc_num);

unsigned int gicv3_get_pending_interrupt_id ( void );
void gicv3_clear_interrupt_pending ( unsigned int id, unsigned int proc_num );

/* from plat/rockchip/common/rockchip_gicv3.c */
uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];

/* from plat/rockchip/common/rockchip_gicv3.c */
static const interrupt_prop_t g01s_interrupt_props[] = {
    PLAT_RK_GICV3_G0_IRQS,
    PLAT_RK_GICV3_G1S_IRQS
};

/* from plat/rockchip/common/rockchip_gicv3.c */
static unsigned int plat_rockchip_mpidr_to_core_pos(unsigned long mpidr)
{
    return (unsigned int)plat_core_pos_by_mpidr(mpidr);
}

/* from plat/rockchip/common/rockchip_gicv3.c */
static const gicv3_driver_data_t rockchip_gic_data = {
    .gicd_base = PLAT_RK_GICD_BASE,
    .gicr_base = PLAT_RK_GICR_BASE,
    .interrupt_props = g01s_interrupt_props,
    .interrupt_props_num = ARRAY_SIZE(g01s_interrupt_props),
    .rdistif_num = PLATFORM_CORE_COUNT,
    .rdistif_base_addrs = rdistif_base_addrs,
    .mpidr_to_core_pos = plat_rockchip_mpidr_to_core_pos,
};

/* from plat/rockchip/common/plat_topology.c */
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
    unsigned int cluster_id, cpu_id;

    cpu_id = mpidr & MPIDR_AFFLVL_MASK;
#ifdef PLAT_RK_MPIDR_CLUSTER_MASK
    cluster_id = mpidr & PLAT_RK_MPIDR_CLUSTER_MASK;
#else
    cluster_id = mpidr & MPIDR_CLUSTER_MASK;
#endif

    cpu_id += (cluster_id >> PLAT_RK_CLST_TO_CPUID_SHIFT);

    if (cpu_id >= PLATFORM_CORE_COUNT)
        return -1;

    return cpu_id;
}

/* ================================================================================ */

void
gic_init ( void )
{
		printf ( "gic_init() called\n" );

		gicv3_driver_init ( &rockchip_gic_data );
}

void
gic_cpu_init ( void )
{
}

void
intcon_ena ( int irq )
{
		// just gets a data abort without init
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

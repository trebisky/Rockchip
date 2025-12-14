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

/* This has things like: PLAT_RK_GICR_BASE
 */
#include "platform_def.h"

// #include "platform.h"

#include "stdarg.h"
#include "assert.h"

#define HOLD_PRINTF
#include "../protos.h"

#define CPU_0	0

/* These are in gicv3_main.c */

void gicv3_driver_init ( const gicv3_driver_data_t *plat_driver_data );

void gicv3_enable_interrupt(unsigned int id, unsigned int proc_num);
void gicv3_disable_interrupt(unsigned int id, unsigned int proc_num);

unsigned int gicv3_get_pending_interrupt_id ( void );
void gicv3_clear_interrupt_pending ( unsigned int id, unsigned int proc_num );

/* from plat/rockchip/common/rockchip_gicv3.c */
/* TJT - I declare this static because it conflicts with gicv3_base.c */
static uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];

/* from plat/rockchip/common/rockchip_gicv3.c */
static const interrupt_prop_t g01s_interrupt_props[] = {
    PLAT_RK_GICV3_G0_IRQS,
    PLAT_RK_GICV3_G1S_IRQS
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

/* ================================================================================ */

typedef unsigned int u32;

/* The MMU is in a 64K area
 * This yields 16K words
 * and if we display 4 works per line, 4K lines
 * However the last 3K of these are FFFF_FFFF
 */

// #define MMU_LINES	(4*1024)
#define MMU_LINES	(1*1024)

static void
dump_mmu ( u32 *base )
{
		int i;

		for ( i=0; i<MMU_LINES; i++ ) {
			printf ( "%X:  %X %X %X %X\n",
				base, base[0], base[1], base[2], base[3] );
			base += 4;
		}
}

static void
gic_cpu_checker ( void )
{
		unsigned int reg;
		unsigned long lreg;

		// This only exists on aarch32
		// printf ( "Sctlr = %X\n", read_sctlr() );

		printf ( "Current EL = %d\n", get_current_el() );

		reg = read_sctlr_el1();
		printf ( "Sctlr_el1 = %X\n", reg );
		reg = read_sctlr_el2();
		printf ( "Sctlr_el2 = %X\n", reg );

		if ( reg & SCTLR_M_BIT )
			printf ( "MMU enabled\n" );
		else
			printf ( "MMU -not- enabled\n" );

		if ( reg & SCTLR_C_BIT )
			printf ( "D Cache enabled\n" );
		else
			printf ( "D Cache -not- enabled\n" );

		if ( reg & SCTLR_I_BIT )
			printf ( "I Cache enabled\n" );
		else
			printf ( "I Cache -not- enabled\n" );

		lreg = read_tcr_el2();
		printf ( "Tcr_el2 = %Y\n", lreg );
		lreg = read_ttbr0_el2();
		printf ( "TTbr0_el2 = %Y\n", lreg );

		// This spits out 1024 lines
		// dump_mmu ( (u32 *) lreg );

		// This gives a synch abort
		// lreg = read_ttbr1_el2();
		// printf ( "TTbr1_el2 = %Y\n", lreg );
		/* The TTBRx can be 128 registers if FEAT is active */
}

/* For testing */
static void
all_sgi ( void )
{
		int i;

		for ( i=0; i<16; i++ )
			gicv3_enable_interrupt ( i, CPU_0 );
}

/* Was gic_init(), but that conflicts with gicv3_base.c */
void
intcon_gic_init ( void )
{
		printf ( "gic_init() called\n" );

		gic_cpu_checker ();

		gicv3_driver_init ( &rockchip_gic_data );
		gicv3_distif_init();
		// gicv3_rdistif_init(plat_my_core_pos());
		gicv3_rdistif_init ( CPU_0 );
		// gicv3_cpuif_enable(plat_my_core_pos());
		gicv3_cpuif_enable ( CPU_0 );

		all_sgi ();
}

void
intcon_gic_cpu_init ( void )
{
		/* not used for v3 at present */
}

void
intcon_ena ( int irq )
{
		printf ( "Enable IRQ %d in GIC\n", irq );
		gicv3_enable_interrupt ( irq, CPU_0 );
		/* XXX what are priority values? */
		gicv3_set_interrupt_priority ( irq, CPU_0, 3 );
		gicv3_set_interrupt_group ( irq, CPU_0, INTR_GROUP1NS );
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

/* For testing, poke ourself with an SGI */
void
intcon_sgi ( int sgi )
{
		gicv3_raise_sgi ( sgi, GICV3_G1NS, CPU_0 );
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
	rkpanic ( "gic_panic" );
}

void
__assert ( char *file, unsigned int line )
{
    // printf("ASSERT: %s:%u\n", file, line);
    printf("ASSERT: %s: Line: %d\n", file, line);
	rkpanic ( "assert" );
}

#define LOG_BUF_SIZE	128

void asnprintf (char *abuf, unsigned int size, const char *fmt, va_list args);

/* From common/tf_log.c */
void tf_log(const char *fmt, ...)
{
    uint32_t log_level;
    va_list args;
    const char *prefix_str;
	char log_buf[LOG_BUF_SIZE];

    /* We expect the LOG_MARKER_* macro as the first character */
    log_level = (uint32_t)fmt[0];

    /* Verify that log_level is one of LOG_MARKER_* macro defined in debug.h */
    assert((log_level > 0U) && (log_level <= LOG_LEVEL_VERBOSE));
    assert((log_level % 10U) == 0U);

/* TJT */
#ifdef notdef
    if (log_level > max_log_level) {
        return;
    }

    prefix_str = plat_log_get_prefix(log_level);

    while (*prefix_str != '\0') {
        (void)putchar((int)*prefix_str);
        prefix_str++;
    }
#endif

    va_start(args, fmt);
    // (void)vprintf(fmt + 1, args);
	asnprintf ( log_buf, LOG_BUF_SIZE, fmt+1, args );
    va_end(args);

	uart_puts ( log_buf );
}

/* THE END */

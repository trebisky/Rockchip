/*
 * Copyright (C) 2018  Tom Trebisky  <tom@mmto.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. See README and COPYING for
 * more details.
 *
 * gic500.c - driver for the GIC500 in the Rockchip RK3399
 *
 * This code began with intcon_gic400.c for the Nanopi Fire3 (Samsung S5P6818)
 *  (however the s5p6818 uses a gic400, so this may or may not be a wise
 *   choice).
 *
 * The GIC500 is the interrupt controller in the Rockchip RK3399
 *  This device is arm,gic-v3 and arm,gic-v3-its
 *
 * It is given 2 pages in the TRM (page 560-561)
 *
 * The ARM GIC Architecture Specification (ver 3 and 4)
 *  is a 930 page document.
 * Chapter 12 is the programmers model (page 185)
 *
 * A valuable resource is:
 *  cd /u1/linux/linux-git
 *  arch/arm64/boot/dts/rockchip/rk3399.dtsi
 *  This device is at arm,gic-v3 and arm,gic-v3-its

 * A peek at rk3399.dtsi yields a bonus piece of information
 *  The *  enable method for all 6 cores is "psci" !
 *    (this is also true for the RK3328). 
 *
 * cpu_b0: cpu@100 {
 *            device_type = "cpu";
 *            compatible = "arm,cortex-a72";
 *            reg = <0x0 0x100>;
 *            enable-method = "psci";
 *
 * Tom Trebisky  1-3-2022, 2-14-2022, 12-6-2025
 */

#include "protos.h"
#include "rk3399_ints.h"

typedef volatile unsigned int vu32;
typedef unsigned int u32;
typedef unsigned long u64;

#define BIT(x)	(1<<(x))

/* Base addresses --
 * I learned by working with the RK3328 that you
 *  should assume nothing when the TRM does not
 *  provide information.
 *
 * I figured out the gic400 addresses for the RK3328
 *  as follows:
 * cd /Projects/RK3328/sources/arm-trusted-firmware
 * rgrep GIC | grep BASE | grep rk33
 *
 * For the RK3328 this yields:
 *  ./plat/rockchip/rk3328/rk3328_def.h:#define GIC400_BASE		0xff810000
 *  ./plat/rockchip/rk3328/rk3328_def.h:#define RK3328_GICD_BASE		(GIC400_BASE + 0x1000)
 *  ./plat/rockchip/rk3328/rk3328_def.h:#define RK3328_GICC_BASE		(GIC400_BASE + 0x2000)
 *  ./plat/rockchip/rk3328/rk3328_def.h:#define RK3328_GICR_BASE		0	// no GICR in GIC-400
 *
 * The RK3399 is not so straightforward:
 *
 *  ./plat/rockchip/rk3399/include/shared/addressmap_shared.h:#define GIC500_BASE		(MMIO_BASE + 0x06E00000)
 *  ./plat/rockchip/rk3399/rk3399_def.h:#define BASE_GICD_BASE			(GIC500_BASE)
 *  ./plat/rockchip/rk3399/rk3399_def.h:#define BASE_GICR_BASE			(GIC500_BASE + SIZE_M(1))
 *
 * We need two other pieces of information:
 *  ./plat/rockchip/rk3399/drivers/m0/include/addressmap.h:#define MMIO_BASE			0x40000000
 *  ./plat/rockchip/rk3399/include/addressmap.h:#define MMIO_BASE		0xF8000000
 *  ./plat/rockchip/rk3399/include/shared/addressmap_shared.h:#define SIZE_M(n)		((n) * 1024 * 1024)
 *
 * Also, if we look at the TRM we see this:
 *   FEE0_0000 (2M region)
 * 
 * So f800_0000 + 06e0_0000 = fee0_0000
 * 1024*1024 = 0010_0000
 * And fee0_0000 + 0010_0000 = fef0_0000  (GICR_BASE)
 *
 * A look at plat/rockchip/rk3399/include/platform_def.h
 *    shows us ---
 *  #define PLAT_RK_GICD_BASE   BASE_GICD_BASE
 *  #define PLAT_RK_GICR_BASE   BASE_GICR_BASE
 *  #define PLAT_RK_GICC_BASE   0
 *
 * So, no "C" base at all -- but we get an R base.
 * Hmmm - the GIC500 is indeed different from the GIC400.
 */

#define GIC_BASE	0xfee00000

#define GICD_BASE	0xfee00000
#define GICR_BASE	0xfef00000

/* The address map in the TRM shows
 * a 2M window allocated for the GIC500, i.e.
 * 0xfee00000 to 0xfeffffff
 */

/* XXX crazy experiment
 * Which did not work.
 */
#define GICC_BASE	GICR_BASE

#ifdef notdef
/* At this point, I have no idea where I got these values from.
 * Note that 3 of the areas here are outside of the 2M GIC area.
 */
#define GICC_BASE	0xfff00000	/* 0x10000 */
#define GICR_BASE	0xfef00000	/* 0xC0000 */
#define GICH_BASE	0xfff10000	/* 0x10000 */
#define GICV_BASE	0xfff20000	/* 0x10000 */

#define GITS_BASE	0xfee20000	/* 0x20000 */
#endif

/* The Rockchip RK3399 TRM gives only 2 pages to the GIC in chapter 11 (page 560).
 * It leaves the rest to the 930 page manual for GICv3.
 * Those pages do give some important details, as the GIC can be configured
 * in various ways for different silicon.
 * In the RK3399 we have:
 *   2 clusters
 *   cluster 0 has 4 cpu, cluster 1 has 2 cpu
 *   4 RID bits
 *   256 spis
 *   And more
 */

/* ================================================================================ */
/* ================================================================================ */

/* XXX - check and think about these */
// #define NUM_CONFIG	10
#define NUM_CONFIG	32
// #define NUM_TARGET	40
#define NUM_TARGET	128
// #define NUM_PRIO	40
#define NUM_PRIO	128
// #define NUM_MASK	5
#define NUM_MASK	16

/* See page 519 in the GICv3 manual */
struct gic500_dist {
	vu32 ctl;				/* 0x00 */
	vu32 type;				/* 0x04 */
	vu32 iid;				/* 0x08 */

	vu32 type2;				/* 0x0c - v4 only */
	vu32 status;			/* 0x10 - v4 only */

	int __pad0[27];

	vu32	group[NUM_MASK];
	/* XXX ok to here */
	int __pad00[16];
	vu32	eset[NUM_MASK];		/* BG - 0x100 */
	int __pad1[16];
	vu32	eclear[NUM_MASK];	/* BG - 0x180 */
	int __pad2[16];
	vu32 pset[NUM_MASK];		/* BG - 0x200 */
	int __pad3[16];
	vu32 pclear[NUM_MASK];		/* BG - 0x280 */
	int __pad4[16];
	vu32 aset[NUM_MASK];		/* BG - 0x300 */
	int __pad5[16];
	vu32 aclear[NUM_MASK];		/* BG - 0x380 */
	int __pad55[16];
	vu32 prio[NUM_PRIO];		/* BG - 0x400 */
	int __pad6[128];
	vu32 target[NUM_TARGET];	/* 0x800 */
	int __pad7[128];
	vu32 config[NUM_CONFIG];	/* 0xc00 */
	int __pad8[32];
	vu32 ppi_stat;			/* 0xd00 */
	vu32 spi_stat[15];		/* 0xd04 */
	int __pad9[112];

	vu32 sgi;			/* 0xf00 */
	int __pad10[3];
	vu32	sgi_pclear[4];		/* 0xf10 */
	vu32	sgi_pset[4];		/* 0xf20 */
	int __pad11[40];

	vu32 pid4;			/* 0xfd0 */
	vu32 pid5;			/* 0xfd4 */
	vu32 pid6;			/* 0xfd8 */
	vu32 pid7;			/* 0xfdc */
	vu32 pid0;			/* 0xfe0 */
	vu32 pid1;			/* 0xfe4 */
	vu32 pid2;			/* 0xfe8 */
	vu32 pid3;			/* 0xfec */

	vu32 cid0;			/* 0xff0 */
	vu32 cid1;			/* 0xff4 */
	vu32 cid2;			/* 0xff8 */
	vu32 cid3;			/* 0xffc */
};

/* Temporary, to verify structure offsets */
void
gic_check ( void )
{
		struct gic500_dist *gp;

		gp = (struct gic500_dist *) 0;

		printf ( "GIC groupR = %X\n", &gp->group );
}

void
gic_init ( void )
{
		struct gic500_dist *gp = (struct gic500_dist *) GICD_BASE;

		printf ( "GIC distributor type = %X\n", gp->type );
		printf ( "GIC distributor type2 = %X\n", gp->type2 );
		printf ( "GIC distributor iid = %X\n", gp->type2 );

		gic_check ();
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

// ++++++++++++++++++++++++++++++++++++++++++++++++
// ++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef OLD_GIC_400
/* Below here is code from the gic-400 */

void gic_show ( void );

/* The GIC-400 is GIC version 2.0
 */

// #define NUM_CONFIG	10
#define NUM_CONFIG	32
// #define NUM_TARGET	40
#define NUM_TARGET	128
// #define NUM_PRIO	40
#define NUM_PRIO	128
// #define NUM_MASK	5
#define NUM_MASK	16

/* How many config registers?
 * 64 they say for v4 with 2 bits per int.
 ( so 64 * 16 = 1024 sources !! )
 */

struct gic400_redist {
	vu32 ctrl;			/* 0x00 */
	vu32 iidr;			/* 0x04 */
	vu32 typer;			/* 0x08 */
	int __pad0;
	vu32 statusr;			/* 0x10 */
	vu32 waker;			/* 0x14 */
	vu32 mpamidr;			/* 0x18 */
	vu32 partidr;			/* 0x1c */
	int __pad1[8];
	vu32 setlpir;			/* 0x40 */
	int __pad2;
	vu32 clrlpir;			/* 0x48 */
	int __pad3[10];
	vu32 propbaser;			/* 0x70 */
	int __pad4;
	vu32 pendbaser;			/* 0x78 */
	/* And more */
};

struct gic400_dist {
	vu32 ctrl;			/* 0x00 */
	vu32 type;			/* 0x04 */
	vu32 iidr;			/* 0x08 */

	vu32 type2;			/* 0x0c - v4 only */
	vu32 statusr;			/* 0x10 - v4 only */

	int __pad0[27];

	vu32	group[NUM_MASK];
	int __pad00[16];
	vu32	eset[NUM_MASK];		/* BG - 0x100 */
	int __pad1[16];
	vu32	eclear[NUM_MASK];	/* BG - 0x180 */
	int __pad2[16];
	vu32 pset[NUM_MASK];		/* BG - 0x200 */
	int __pad3[16];
	vu32 pclear[NUM_MASK];		/* BG - 0x280 */
	int __pad4[16];
	vu32 aset[NUM_MASK];		/* BG - 0x300 */
	int __pad5[16];
	vu32 aclear[NUM_MASK];		/* BG - 0x380 */
	int __pad55[16];
	vu32 prio[NUM_PRIO];		/* BG - 0x400 */
	int __pad6[128];
	vu32 target[NUM_TARGET];	/* 0x800 */
	int __pad7[128];
	vu32 config[NUM_CONFIG];	/* 0xc00 */
	int __pad8[32];
	vu32 ppi_stat;			/* 0xd00 */
	vu32 spi_stat[15];		/* 0xd04 */
	int __pad9[112];

	vu32 sgi;			/* 0xf00 */
	int __pad10[3];
	vu32	sgi_pclear[4];		/* 0xf10 */
	vu32	sgi_pset[4];		/* 0xf20 */
	int __pad11[40];

	vu32 pid4;			/* 0xfd0 */
	vu32 pid5;			/* 0xfd4 */
	vu32 pid6;			/* 0xfd8 */
	vu32 pid7;			/* 0xfdc */
	vu32 pid0;			/* 0xfe0 */
	vu32 pid1;			/* 0xfe4 */
	vu32 pid2;			/* 0xfe8 */
	vu32 pid3;			/* 0xfec */

	vu32 cid0;			/* 0xff0 */
	vu32 cid1;			/* 0xff4 */
	vu32 cid2;			/* 0xff8 */
	vu32 cid3;			/* 0xffc */
};

struct gic400_cpu {
	vu32 ctrl;			/* 0x00 */
	vu32 primask;			/* 0x04 */

	vu32 bpr;			/* 0x08 */
	vu32 iack;			/* 0x0c */
	vu32 eoi;			/* 0x10 */
	vu32 run_pri;			/* 0x14 */
	vu32 high_pri;			/* 0x18 */

	vu32 abpr;			/* 0x1c */
	vu32 aiack;			/* 0x20 */
	vu32 aeoi;			/* 0x24 */
	vu32 ahigh_pri;			/* 0x28 */
	int _pad0[41];
	vu32 apr0;			/* 0xd0 */
	int _pad1[3];
	vu32 nsapr0;			/* 0xe0 */
	int _pad2[6];
	vu32 iid;			/* 0xfc */
	int _pad3[960];
	vu32 dir;			/* 0x1000 */
};

#ifdef notdef
#define GIC_DIST_BASE	((struct gic400_dist *) 0xc0009000)
#define GIC_CPU_BASE	((struct gic400_cpu *) 0xc000a000)
#endif

#define GIC_DIST_BASE	((struct gic400_dist *) GICD_BASE)
#define GIC_CPU_BASE	((struct gic400_cpu *) GICC_BASE)

#define	G0_ENABLE	0x01
#define	G1_ENABLE	0x02

void
intcon_ena ( int irq )
{
	struct gic400_dist *gp = GIC_DIST_BASE;
	int x = irq / 32;
	unsigned long mask = 1 << (irq%32);

	gp->eset[x] = mask;
}

void
intcon_dis ( int irq )
{
	struct gic400_dist *gp = GIC_DIST_BASE;
	int x = irq / 32;
	unsigned long mask = 1 << (irq%32);

	gp->eclear[x] = mask;
}

static void
gic_unpend ( int irq )
{
	struct gic400_dist *gp = GIC_DIST_BASE;
	int x = irq / 32;
	unsigned long mask = 1 << (irq%32);

	gp->pclear[x] = mask;
}

#define SGI_LIST	0
#define SGI_ALL		(1<<24)
#define SGI_SELF	(2<<24)

/* Trigger a software generated interrupt (SGI)
 * "cpu" is the target core.
 */
void
gic_soft ( int sgi_num, int cpu )
{
	struct gic400_dist *gp = GIC_DIST_BASE;

	gp->sgi = SGI_LIST | (1<<(16+cpu)) | sgi_num;
}

/* Trigger a software generated interrupt (SGI)
 *  to ourself
 */
void
gic_soft_self ( int sgi_num )
{
	struct gic400_dist *gp = GIC_DIST_BASE;

	gp->sgi = SGI_SELF | sgi_num;
}

#ifdef notdef
/*-*/ void
/*-*/ gic_handler ( void )
/*-*/ {
/*-*/ 	struct gic400_cpu *cp = GIC_CPU_BASE;
/*-*/ 	int irq;
/*-*/ 
/*-*/ 	irq = cp->iack;
/*-*/ 
/*-*/ 	/* Do we need to EOI a spurious interrupt ? */
/*-*/ 	if ( irq == 1023 ) {
/*-*/ 	    return;
/*-*/ 	}
/*-*/ 
/*-*/ 	if ( irq == IRQ_TIMER0 )
/*-*/ 	    timer_handler ( 0 );
/*-*/ 
/*-*/ 	cp->eoi = irq;
/*-*/ 	gic_unpend ( IRQ_TIMER0 );
/*-*/ }
#endif

int
intcon_irqwho ( void )
{
	struct gic400_cpu *cp = GIC_CPU_BASE;

	return cp->iack;
}

void
intcon_irqack ( int irq )
{
	struct gic400_cpu *cp = GIC_CPU_BASE;

	cp->eoi = irq;
	gic_unpend ( irq );
}

/* Initialize the "banked" registers that are unique to each core
 * This needs to be called by each core when it starts up.
 */
void
gic_cpu_init ( void )
{
	struct gic400_dist *gp = GIC_DIST_BASE;
	struct gic400_cpu *cp = GIC_CPU_BASE;
	int i;

	/* enable all SGI, disable all PPI */
	gp->eclear[0] = 0xffff0000;
	gp->eset[0]   = 0x0000ffff;

	/* priority for PPI and SGI */
	for ( i=0; i<8; i++ )
	    gp->prio[i] = 0xa0a0a0a0;

	cp->primask = 0xf0;
	cp->ctrl = 1;
}

void
gic_init ( void )
{
	struct gic400_dist *gp = GIC_DIST_BASE;
	int i;

	/* Initialize the distributor */
	// gp->ctrl = 0;
	// Forward group 0 and 1
	gp->ctrl = 3;

	/* make all SPI level triggered */
	for ( i=2; i<NUM_CONFIG; i++ )
	    gp->config[i] = 0;

	for ( i=8; i<NUM_TARGET; i++ )
	    gp->target[i] = 0x01010101;

	for ( i=8; i<NUM_PRIO; i++ )
	    gp->prio[i] = 0xa0a0a0a0;

	/* Put everything in group 1 */
	for ( i=0; i<NUM_MASK; i++ )
	    gp->group[i] = 0xffffffff;

	for ( i=1; i<NUM_MASK; i++ )
	    gp->eclear[i] = 0xffffffff;

	for ( i=0; i<NUM_MASK; i++ )
	    gp->pclear[i] = 0xffffffff;

	gp->ctrl = G0_ENABLE;

	/* Initialize banked registers for core 0 */
	gic_cpu_init ();

	gic_show ();
}

static void
hex_show ( char *msg, u64 val )
{
	// printf ( "%s = %08x\n", msg, val );
	printf ( "%s = %h\n", msg, val );
}

void
gic_show ( void )
{
	struct gic400_dist *gp = GIC_DIST_BASE;
	struct gic400_cpu *cp = GIC_CPU_BASE;

	// int is 4 bytes, long is 8
	// printf ( "int is %d bytes\n", sizeof(int) );

	// printf ( "GIC sgi at = %08x\n", &gp->sgi );
	hex_show ( "GIC sgi at", (u64) &gp->sgi );

	/*
	printf ( "GIC t0 at = %08x\n", &gp->target[0] );
	printf ( "GIC s0 at = %08x\n", &gp->spi_stat[0] );
	printf ( "GIC cid0 at = %08x\n", &gp->cid0 );
	*/

	// printf ( "GIC dir at = %08x\n", &cp->dir );
	hex_show ( "GIC dir at", (u64) &cp->dir );

	// printf ( "GIC type  = %08x\n", gp->type );
	// printf ( "GIC type2 = %08x\n", gp->type2 );
	// printf ( "GIC iid   = %08x\n", gp->iidr );

	/*
	GIC type  = 007B04A8
	GIC type2 = 00000000
	GIC iid   = 0001143B
	*/

	hex_show ( "GIC type ", gp->type );
	hex_show ( "GIC type2", gp->type2 );
	hex_show ( "GIC iid  ", gp->iidr );
}

#ifdef notdef
void
gic_test ( void )
{
	int i;

	for ( i=0; i<16; i++ ) {
	    intcon_ena ( IRQ_SGI_0 + i );
	}

	for ( i=0; i<16; i++ ) {
	    printf ( "SGI %d\n", i );
	    gic_soft_self ( SGI_0 + i );
	}
}
#endif

#endif /* OLD_GIC_400 */

/* THE END */

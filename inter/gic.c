/* driver for the GIC500 in the Rockchip RK3399
 *
 * The GIC500 is the interrupt controller in the RK3399
 *
 * It is given 2 pages in the TRM (page 560-561)
 *
 * The ARM GIC Architecture Specification (ver 3 and 4)
 *  is a 930 page document.
 * Chapter 12 is the programmers model (page 185)
 *
 * A valuable resource is:
 *  linux/arch/arm64/boot/dts/rockchip/rk3399.dtsi
 *  This device is arm,gic-v3 and arm,gic-v3-its

 * A bonus piece of information is that the
 *  enable method for all 6 cores is "psci"
 *
 * Tom Trebisky  1-3-2022, 2-14-2022
 */

#include "protos.h"
#include "rk3399_ints.h"

typedef volatile unsigned int vu32;
typedef unsigned int u32;

#define BIT(x)	(1<<(x))

#define GIC_BASE	0xfee00000

#define GICD_BASE	0xfee00000	/* 0x10000 */
#define GICR_BASE	0xfef00000	/* 0xC0000 */
#define GICC_BASE	0xfff00000	/* 0x10000 */
#define GICH_BASE	0xfff10000	/* 0x10000 */
#define GICV_BASE	0xfff20000	/* 0x10000 */

#define GITS_BASE	0xfee20000	/* 0x20000 */

/* a 2M window is allocated for the GIC500, i.e.
 * 0xfee00000 to 0xfeffffff
 * Note that 3 of the areas above are outside of this.
 */

void
gic_init ( void )
{
}

/* THE END */

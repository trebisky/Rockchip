/* driver for the GIC500 in the Rockchip RK3399
 *
 * The GIC500 is the interrupt controller in the RK3399
 *
 * Tom Trebisky  1-3-2022
 */

#include "protos.h"

typedef volatile unsigned int vu32;
typedef unsigned int u32;

#define BIT(x)	(1<<(x))

#define GIC_BASE	0xfee00000

/* a 2M window is allocated for the GIC500, i.e.
 * 0xfee00000 to 0xfeffffff
 */


void
gic_init ( void )
{
}

/* THE END */

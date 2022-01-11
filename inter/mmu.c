/* MMU driver for the Rockchip RK3399
 *
 * This is really just a simple experiment as follows --
 * I was attempting using U-boot to examine memory where
 * the SoC bootrom image should live.
 * I discover that I get Synchronous Aborts when
 * trying to access ffff0000 where the bootrom should be.
 * I also get SA at many other places, including
 * addresses in the first 1M of the RAM address space.
 * This makes me wonder if something somewhere has
 * mapped access to certain addresses as invalid.
 * And I thought perhaps that the SoC mmu might be
 *  the culprit.  Hence these experiments.
 *
 * Apparently this MMU is not at the root of my trouble.
 *
 * This doesn't really belong as part of the "inter" demo,
 * but it was what I was working on at the time, so feel
 * free to ignore this.
 *
 * Tom Trebisky  1-3-2022
 */

#include "protos.h"

typedef volatile unsigned int vu32;
typedef unsigned int u32;

#define BIT(x)	(1<<(x))

/* The TRM describes the GPIO in chapter 20
 *
 * The on chip LED is connected to GPIO0_B3
 */

struct rock_mmu {
	vu32		addr;
	vu32		status;
	vu32		cmd;
	vu32		fault;

	vu32		zap;
	vu32		int_rawstat;
	vu32		int_clear;
	vu32		int_mask;

	vu32		int_status;
	vu32		gating;
};

/* HDCP mmu base addr
 * the TRM section 9.4 defines several other addresses
 */
#define MMU_BASE	((struct rock_mmu *) 0xff930000)

/* Only 3 bits are used in the CMD register.
 * The MMU only accepts commands when stalled
 */
#define CMD_ENABLE		0
#define CMD_DISABLE		1
#define CMD_ENABLE_STALL	2
#define CMD_DISABLE_STALL	3
#define CMD_ZAP_CACHE		4
#define CMD_PF_DONE		5
#define CMD_FORCE_RESET		6

void
mmu_init ( void )
{
	struct rock_mmu *mp = MMU_BASE;

	printf ( "Switching off the MMU\n" );
	mp->cmd = CMD_ENABLE_STALL;
	mp->cmd = CMD_DISABLE;
	// mp->cmd = CMD_FORCE_RESET;

	printf ( "MMU addr: %h\n", mp->addr );
	printf ( "MMU status: %h\n", mp->status );
	printf ( "MMU cmd: %h\n", mp->cmd );
	printf ( "MMU fault: %h\n", mp->fault );
	printf ( "MMU zap: %h\n", mp->zap );
}

/* THE END */

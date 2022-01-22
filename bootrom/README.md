Study of the Rockchip RK3399 bootrom

I wrote bare_dump to be able to extract this
(which seems to be read protected by the time U-Boot
is running).  I used bare_dump to dump the addresses
ffff0000 to ffffffff (64K) and then verified that
this yields two copies of the same thing.
So, as the TRM says, the bootrom is 32K in size.


"Arm_wrap" has proven itself to be a handy tool when disassembling
binary files.  It allows objdump to produce listings with
proper addresses and references, and other niceties.
It was last used in my Fire3 project and was copied from
there to here, then adjusted as necessary.

Tom Trebisky  1-20-2022

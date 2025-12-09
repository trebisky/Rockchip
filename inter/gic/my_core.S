/*
 * from plat_helpers.S
 *
 * tjt  12-8-2025
 */

#include <asm_macros.S>

#include <arch.h>
#include <platform_def.h>

.globl plat_my_core_pos

func plat_my_core_pos
    mrs x0, mpidr_el1
    and x1, x0, #MPIDR_CPU_MASK
    and x0, x0, #MPIDR_CLUSTER_MASK
    add x0, x1, x0, LSR #PLAT_RK_CLST_TO_CPUID_SHIFT
    ret
endfunc plat_my_core_pos

/* THE END */

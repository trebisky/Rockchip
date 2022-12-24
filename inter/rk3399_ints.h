/* List of interrupt sources for the Rockchip RK3399
 *
 * pages 15-22 of the TRM describe the interrupt sources.
 *
 * Tom Trebisky  2-14-2022
 */

#define IRQ_SGI_0       0
#define IRQ_SGI_1       1
#define IRQ_SGI_2       2
#define IRQ_SGI_3       3
#define IRQ_SGI_4       4
#define IRQ_SGI_5       5
#define IRQ_SGI_6       6
#define IRQ_SGI_7       7
#define IRQ_SGI_8       8
#define IRQ_SGI_9       9
#define IRQ_SGI_10      10
#define IRQ_SGI_11      11
#define IRQ_SGI_12      12
#define IRQ_SGI_13      13
#define IRQ_SGI_14      14
#define IRQ_SGI_15      15

/* PPI follow, all low level */
/* 8 are NA, so there are 8 "in use" and this is what the TRM quotes
 */
#define IRQ_PPI_0       16
#define IRQ_PPI_1       17
#define IRQ_PPI_2       18
#define IRQ_PPI_3       19
#define IRQ_PPI_4       20
#define IRQ_PPI_5       21
#define IRQ_PPI_6       22
#define IRQ_PPI_7       23
#define IRQ_PPI_8       24
#define IRQ_PPI_9       25
#define IRQ_PPI_10      26
#define IRQ_PPI_11      27
#define IRQ_PPI_12      28
#define IRQ_PPI_13      29
#define IRQ_PPI_14      30
#define IRQ_PPI_15      31

/* SPI follow -- all high level */
/* We have 150 SPI (32 to 181)
 * 2 are NA so 148 are "in use" and this is what the TRM quotes
 */
#define IRQ_CRYPTO0	32
// ...
#define IRQ_GPIO0	46
#define IRQ_GPIO1	47
#define IRQ_GPIO2	48
#define IRQ_GPIO3	49
#define IRQ_GPIO4	50

#define IRQ_TIMER0	113
#define IRQ_TIMER1	114
#define IRQ_TIMER2	115
#define IRQ_TIMER3	116
#define IRQ_TIMER4	117
#define IRQ_TIMER5	118
#define IRQ_TIMER6	119
#define IRQ_TIMER7	120
#define IRQ_TIMER8	121
#define IRQ_TIMER9	122
#define IRQ_TIMER10	123
#define IRQ_TIMER11	124

#define IRQ_UART1	130
#define IRQ_UART0	131
#define IRQ_UART2	132
#define IRQ_UART3	133
#define IRQ_UART4	134

#define IRQ_USB0_BVALID	135
#define IRQ_USB0_ID	136
#define IRQ_USB0	137
#define IRQ_USB0_LINE	138
#define IRQ_USB0_RXDET	139

#define IRQ_USB1_BVALID	140
#define IRQ_USB1_ID	141
#define IRQ_USB1	142
#define IRQ_USB1_LINE	143
#define IRQ_USB1_RXDET	144

#define IRQ_USB0_PME	155
#define IRQ_USB0_H_LEG	156
#define IRQ_USB0_H_SYS	157

#define IRQ_USB1_PME	158
#define IRQ_USB1_H_LEG	159
#define IRQ_USB1_H_SYS	160

#define IRQ_MBOX1_0	172
#define IRQ_MBOX1_1	173
#define IRQ_MBOX1_2	174
#define IRQ_MBOX1_3	175

#define IRQ_MBOX0_0	176
#define IRQ_MBOX0_1	177
#define IRQ_MBOX0_2	178
#define IRQ_MBOX0_3	179

#define IRQ_EXTERRIRQ_PD_CORE_L	180
#define IRQ_EXTERRIRQ_PD_CORE_B	181

/* THE END */

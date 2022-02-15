/* List of interrupt sources for the Rockchip RK3399
 *
 * These are "SPI" for the GIC500
 *
 * pages 15-22 of the TRM describe the interrupt sources.
 * There are 148 SPI (shared peripheral interrupts)
 * and 8 PPI (private peripheral interrupts)
 * 
 * So they say, but ...
 *  we see 16-31 as low level PPI in the source list
 *  we see 32-181 as high level spi in the source list
 *
 * Tom Trebisky  2-14-2022
 */

/* 0-15 are not SPI */
/* 16-31 are PPI */
#define INT_GPIO0	46
#define INT_GPIO1	47
#define INT_GPIO2	48
#define INT_GPIO3	49
#define INT_GPIO4	50

#define INT_TIMER0	113
#define INT_TIMER1	114
#define INT_TIMER2	115
#define INT_TIMER3	116
#define INT_TIMER4	117
#define INT_TIMER5	118
#define INT_TIMER6	119
#define INT_TIMER7	120
#define INT_TIMER8	121
#define INT_TIMER9	122
#define INT_TIMER10	123
#define INT_TIMER11	124

#define INT_UART1	130
#define INT_UART0	131
#define INT_UART2	132
#define INT_UART3	133
#define INT_UART4	134

#define INT_USB0_BVALID	135
#define INT_USB0_ID	136
#define INT_USB0	137
#define INT_USB0_LINE	138
#define INT_USB0_RXDET	139

#define INT_USB1_BVALID	140
#define INT_USB1_ID	141
#define INT_USB1	142
#define INT_USB1_LINE	143
#define INT_USB1_RXDET	144

#define INT_USB0_PME	155
#define INT_USB0_H_LEG	156
#define INT_USB0_H_SYS	157

#define INT_USB1_PME	158
#define INT_USB1_H_LEG	159
#define INT_USB1_H_SYS	160

#define INT_MBOX1_0	172
#define INT_MBOX1_1	173
#define INT_MBOX1_2	174
#define INT_MBOX1_3	175

#define INT_MBOX0_0	176
#define INT_MBOX0_1	177
#define INT_MBOX0_2	178
#define INT_MBOX0_3	179

/* THE END */

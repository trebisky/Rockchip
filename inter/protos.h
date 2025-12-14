/* protos.h for my Rockchip demos
 *
 * Tom Trebisky  1-3-2022
 */

void spin ();
void rkpanic ( char * );

void uart_init ( void );
void uart_puts ( char * );

void timer_handler ( void );
void timer_init ( void );
void timer_show ( void );

void gpio_init ( void );
void led_on ( void );
void led_off ( void );

/* This is for gic/gic_compat.c */
#ifndef HOLD_PRINTF
void printf ( char *, ... );
#endif

void show_reg ( char *, int * );

void mmu_init ( void );

void intcon_gic_init ( void );
void intcon_gic_cpu_init ( void );
int intcon_irqwho ( void );
void intcon_ena ( int );
void intcon_irqack ( int );
void intcon_sgi ( int );

void pll_test ( void );
void cpu_clock_100 ( void );

/* THE END */

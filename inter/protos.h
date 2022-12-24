/* protos.h for my Rockchip demos
 *
 * Tom Trebisky  1-3-2022
 */

void uart_init ( void );
void uart_puts ( char * );

void timer_handler ( void );
void timer_init ( void );
void timer_show ( void );

void gpio_init ( void );
void led_on ( void );
void led_off ( void );

void printf ( char *, ... );
void show_reg ( char *, int * );

void mmu_init ( void );

void gic_init ( void );
int intcon_irqwho ( void );
void intcon_ena ( int );
void intcon_irqack ( int );

/* THE END */

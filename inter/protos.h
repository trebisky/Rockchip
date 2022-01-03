/* protos.h for my Rockchip demos
 *
 * Tom Trebisky  1-3-2022
 */

void uart_init ( void );
void uart_puts ( char * );

void gpio_init ( void );
void led_on ( void );
void led_off ( void );

void printf ( char *, ... );
void show_reg ( char *, int * );

/* THE END */

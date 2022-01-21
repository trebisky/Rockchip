/* serial.c
 * (c) Tom Trebisky  7-2-2017
 *
 * Serial (uart) driver for the F411
 * For the 411 this is section 19 of RM0383
 *
 * This began (2017) as a simple polled output driver for
 *  console messages on port 1
 * In 2020, I decided to extend it to listen to a GPS receiver
 *  on port 2.
 *
 * Notice that I number these 1,2,3.
 * However my "3" is what they call "6" in the manual.
 * 
 * On the F411, USART1 and USART6 are on the APB2 bus.
 * On the F411, USART2 is on the APB1 bus.
 *
 * On the F411, after reset, with no fiddling with RCC
 *  settings, both are running at 16 Mhz.
 *  Apparently on the F411 both APB1 and APB2
 *   always run at the same rate.
 *
 * NOTE: On my black pill boards, pins C6 and C7 are not available,
 *  Meaning that UART3 (aka UART6) is not available.
 *  The code is in here, but not of much use if you can't
 *  get to the pins!!
 */

#include <stdarg.h>

void uart_puts ( char * );

#define PRINTF_BUF_SIZE 128
static void asnprintf (char *abuf, unsigned int size, const char *fmt, va_list args);

void
printf ( char *fmt, ... )
{
	char buf[PRINTF_BUF_SIZE];
        va_list args;

        va_start ( args, fmt );
        asnprintf ( buf, PRINTF_BUF_SIZE, fmt, args );
        va_end ( args );

        uart_puts ( buf );
}

/* The limit is absurd, so take care */
void
sprintf ( char *buf, char *fmt, ... )
{
        va_list args;

        va_start ( args, fmt );
        asnprintf ( buf, 256, fmt, args );
        va_end ( args );
}

/* ========================================================================= */

/* Here I develop a simple printf.
 * It only has 3 triggers:
 *  %s to inject a string
 *  %d to inject a decimal number
 *  %h to inject a 32 bit hex value as xxxxyyyy
 */

#define PUTCHAR(x)      if ( buf <= end ) *buf++ = (x)

static const char hex_table[] = "0123456789ABCDEF";

// #define HEX(x)  ((x)<10 ? '0'+(x) : 'A'+(x)-10)
#define HEX(x)  hex_table[(x)]

#ifdef notdef
static char *
sprintnb ( char *buf, char *end, int n, int b)
{
        char prbuf[16];
        register char *cp;

        if (b == 10 && n < 0) {
            PUTCHAR('-');
            n = -n;
        }
        cp = prbuf;

        do {
            // *cp++ = "0123456789ABCDEF"[n%b];
            *cp++ = hex_table[n%b];
            n /= b;
        } while (n);

        do {
            PUTCHAR(*--cp);
        } while (cp > prbuf);

        return buf;
}
#endif

static char *
sprintn ( char *buf, char *end, int n )
{
        char prbuf[16];
        char *cp;

        if ( n < 0 ) {
            PUTCHAR('-');
            n = -n;
        }
        cp = prbuf;

        do {
            // *cp++ = "0123456789"[n%10];
            *cp++ = hex_table[n%10];
            n /= 10;
        } while (n);

        do {
            PUTCHAR(*--cp);
        } while (cp > prbuf);

        return buf;
}

static char *
shex2( char *buf, char *end, int val )
{
        PUTCHAR( HEX((val>>4)&0xf) );
        PUTCHAR( HEX(val&0xf) );
        return buf;
}

#ifdef notdef
static char *
shex3( char *buf, char *end, int val )
{
        PUTCHAR( HEX((val>>8)&0xf) );
        return shex2(buf,end,val);
}

static char *
shex4( char *buf, char *end, int val )
{
        buf = shex2(buf,end,val>>8);
        return shex2(buf,end,val);
}
#endif

static char *
shex8( char *buf, char *end, int val )
{
        buf = shex2(buf,end,val>>24);
        buf = shex2(buf,end,val>>16);
        buf = shex2(buf,end,val>>8);
        return shex2(buf,end,val);
}

static void
asnprintf (char *abuf, unsigned int size, const char *fmt, va_list args)
{
    char *buf, *end;
    int c;
    char *p;

    buf = abuf;
    end = buf + size - 1;
    if (end < buf - 1) {
        end = ((void *) -1);
        size = end - buf + 1;
    }

    while ( c = *fmt++ ) {
	if ( c != '%' ) {
            PUTCHAR(c);
            continue;
        }
	c = *fmt++;
	if ( c == 'd' ) {
	    buf = sprintn ( buf, end, va_arg(args,int) );
	    continue;
	}
	if ( c == 'x' ) {
	    buf = shex2 ( buf, end, va_arg(args,int) & 0xff );
	    continue;
	}
	if ( c == 'h' || c == 'X' ) {
	    buf = shex8 ( buf, end, va_arg(args,int) );
	    continue;
	}
	if ( c == 'c' ) {
            PUTCHAR( va_arg(args,int) );
	    continue;
	}
	if ( c == 's' ) {
	    p = va_arg(args,char *);
	    // printf ( "Got: %s\n", p );
	    while ( c = *p++ )
		PUTCHAR(c);
	    continue;
	}
    }
    if ( buf > end )
	buf = end;
    PUTCHAR('\0');
}

#ifdef notdef
void
serial_printf ( int fd, char *fmt, ... )
{
	char buf[PRINTF_BUF_SIZE];
        va_list args;

        va_start ( args, fmt );
        asnprintf ( buf, PRINTF_BUF_SIZE, fmt, args );
        va_end ( args );

        uart_puts ( fd, buf );
}
#endif

/* Handy now and then */
void
show_reg ( char *msg, int *addr )
{
	printf ( "%s %h %h\n", msg, (long) addr, *addr );

	/*
	console_puts ( msg );
	console_putc ( ' ' );
	print32 ( (int) addr );
	console_putc ( ' ' );
	print32 ( *addr );
	console_putc ( '\n' );
	*/
}

/* THE END */

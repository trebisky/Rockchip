/* mkrock - make an idb format boot file for the Rockchip RK3399
 *
 * Tom Trebisky 1-16-2022
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void rock_encode ( char *, int );

#define BUF_SIZE	64*1024

char buffer[BUF_SIZE];

void
error ( char *msg )
{
	fprintf ( stderr, "%s\n", msg );
	exit ( 1 );
}

/* Quick and dirty, no error messages */
int
main ( int argc, char **argv )
{
	int fd, n;

	--argc;
	++argv;
	if ( argc < 2 ) {
	    error ( "usage: mkrock infile outfile" );
	}

	fd = open ( *argv, O_RDONLY );
	if ( fd < 0 )
	    return 1;
	n = read ( fd, buffer, BUF_SIZE );
	if ( n <= 0 )
	    return 1;

	rock_encode ( buffer, n );
	write ( 1, buffer, n );

	return 0;
}

/* THE END */

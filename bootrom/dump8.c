/* dump8 - display 8 byte (64 bit) constants
 *   in a civilized way.
 *
 *  Tom Trebisky  1-22-2022
 */

#include <stdio.h>
#include <stdlib.h>
// #include <sys/types.h>
// #include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


char *binfile = "bootrom.bin";

#define IMSIZE	32768

#define ADDR_BASE	0xffff0000

char image[IMSIZE];

void
error ( char *msg )
{
	fprintf ( stderr, "%s\n", msg );
	exit ( 1 );
}

void
dumpit ( unsigned int addr, int count )
{
	unsigned int oaddr;
	unsigned long *base;
	int offset;
	int i;

	offset = addr / sizeof (unsigned long);
	oaddr = ADDR_BASE + addr;

	base = (unsigned long *) image;

	for ( i=0; i<count; i++ ) {
	    printf ( "%08x:\t%016lx\n", oaddr, base[offset] );
	    oaddr += sizeof(unsigned long);
	    offset++;
	}
}

int
main ( int argc, char **argv )
{
	unsigned int start;
	int count;
	int fd;

	--argc;
	++argv;

	if ( argc != 2 )
	    error ( "Usage: dump8 start, count" );

	start = strtol ( argv[0], NULL, 16 );
	count = atoi ( argv[1] );

	printf ( "Dump %08x %d\n", start, count );

	fd = open ( binfile, O_RDONLY );
	if ( fd < 0 )
	    error ( "bin file open fails" );
	if ( read ( fd, image, IMSIZE ) != IMSIZE )
	    error ( "read fails" );
	close ( fd );

	dumpit ( start, count );

	return 0;
}

/* THE END */


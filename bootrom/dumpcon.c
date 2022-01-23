/* dumpcon -
 *  display blocks of constants in a civilized way.
 *  this began as "dump8" to just dump 64 bit constants
 *  then I added the "-4" option to dump 32 bit constants
 *
 *  Tom Trebisky  1-22-2022
 *
 * A typical invocation looks like this:
 * ./dumpcon -4 4e70 100 >zzz
 * The value "100" is a guess, I trim the end of the
 * file zzz and then splice it into the disassembly
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
dump8 ( unsigned int addr, int count )
{
	unsigned int oaddr;
	unsigned long *base;
	int offset;
	int i;
	unsigned int hi, lo;

	offset = addr / sizeof (unsigned long);
	oaddr = ADDR_BASE + addr;

	base = (unsigned long *) image;

	for ( i=0; i<count; i++ ) {
	    hi = base[offset] >> 32;
	    lo = base[offset] & 0xffffffff;
	    printf ( "%08x: %08x %08x\n", oaddr, hi, lo );
	    oaddr += sizeof(unsigned long);
	    offset++;
	}
}

void
dump4 ( unsigned int addr, int count )
{
	unsigned int oaddr;
	unsigned int *base;
	int offset;
	int i;

	offset = addr / sizeof (unsigned int);
	oaddr = ADDR_BASE + addr;

	base = (unsigned int *) image;

	for ( i=0; i<count; i++ ) {
	    printf ( "%08x: %08x\n", oaddr, base[offset] );
	    oaddr += sizeof(unsigned int);
	    offset++;
	}
}

int
main ( int argc, char **argv )
{
	unsigned int start;
	int count;
	int fd;
	int type = 8;

	--argc;
	++argv;

	while ( argc && **argv == '-' ) {
	    if ( argv[0][1] == '4' ) {
		type = 4;
		// printf ( "32 bit mode selected\n" );
	    }
	    --argc;
	    ++argv;
	}

	if ( argc != 2 )
	    error ( "Usage: dumpcon [-4] start, count" );

	start = strtol ( argv[0], NULL, 16 );
	start &= 0xffff;
	count = atoi ( argv[1] );

	// printf ( "Dump %08x %d\n", start, count );

	fd = open ( binfile, O_RDONLY );
	if ( fd < 0 )
	    error ( "bin file open fails" );
	if ( read ( fd, image, IMSIZE ) != IMSIZE )
	    error ( "read fails" );
	close ( fd );

	if ( type == 8 )
	    dump8 ( start, count );
	else
	    dump4 ( start, count );

	return 0;
}

/* THE END */


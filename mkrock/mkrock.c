/* mkrock - make an idb format boot file for the Rockchip RK3399
 *
 * Tom Trebisky 1-16-2022
 *
 * All of this is done by u-boot/tools/mkimage
 * This program is based on a careful study of that, and some code
 * is indeed derived especially from rkcommon.c
 * Why write a new program if mkimage does the job?
 * Mostly because I want to learn and understand how all this works.
 *
 * The file generated by this should be written to
 *  and SD card at offset 64, or at the start of SPI flash (NAND).
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void rock_encode ( char *, int );

/* This would have been header0_info in u-boot/mkimage.
 * This is a 512 byte header that starts the file.
 */
struct rock_header {
	unsigned int magic;
	int _pad0;
	unsigned int disable_rc4;
	unsigned short init_offset;
	char _pad1[492];
	unsigned short init_size;
	unsigned short init_boot_size;
	short _pad2;
};

/* Some things are specified in units of 512 byte blocks.
 * Things we load must be padded to multiples of 2048 bytes.
 */
#define ROCK_MAGIC	0x0ff0aa55
#define	SIZE_ALIGN	2048
#define	BLOCK_SIZE	512
#define	EXTRA_BOOT	524288		/* 512 << 10 i.e. 1024 blocks */

#define ROUNDUP(x, y)	(((x) + ((y) - 1)) & ~((y) - 1))

void
error ( char *msg )
{
	fprintf ( stderr, "%s\n", msg );
	exit ( 1 );
}

/* count is in bytes */
void
write_zero_pad ( int fd, int count )
{
	char *zeros;

	// printf ( "zero pad: %d\n", count );

	zeros = (char *) malloc ( count );
	memset ( zeros, '\0', count );
	write ( fd, zeros, count );
	free ( zeros );
}

struct image {
	int size;
	int pad_size;
} image_info;


#define INIT_OFFSET	4	/* 512 blocks from start of file */

/* I run this on an Intel machine, so I don't worry about byte order routines
 * as both my host and the target are both little endian.
 *
 * The following link discusses the size values in this header.
 *   https://lists.denx.de/pipermail/u-boot/2017-May/293267.html
 *
 * Adding size to the init_boot_size value, allows a feature this
 * describes as BACK_TO_BROM to cause a second loader to be executed.
 *
 * Also note that init_size must be a multiple of 4*512 (i.e. 2K)
 *  or the bootrom will not load it.  So the image must be padded
 *  to a 2K multiple.
 */
void
make_header ( struct rock_header *rh )
{
	int extra_size;

	memset ( (char *) rh, '\0', sizeof ( struct rock_header) );

	rh->magic = ROCK_MAGIC;
	rh->disable_rc4 = 1;	/* The SPL is not encrypted */
	rh->init_offset = INIT_OFFSET;

	rh->init_size = image_info.pad_size / BLOCK_SIZE;
	// printf ( "init size = %d %x %d\n", rh->init_size, rh->init_size, image_info.pad_size );

	/* We add a huge (512K) region to the end of the actual boot image we provide.
	 * This certainly allows all manner of things (such as a dtb) to be appended to
	 * the file, and this is exactly what U-boot does and we can only assume that it
	 * works.  It would be nice to see bootrom code or documentation and know exactly
	 * how it handles this, especially with only 192K of static ram to load into.
	 * This adds 1024 blocks to the init_size.
	 *
	 * I also examined the idbloader.bin file that came with the debian distribution
	 * and it adds exactly this same 1024 blocks to the init_size.
	 * It already has something appended to the file, and we learn elsewhere that
	 * it has something called "ddr.bin" as the primary payload, but has the tpl
	 * appended as "extra stuff" that is not described by the header in any way.
	 *
	 * Once all this gets on the SD card (or NAND) there is of course no vestige
	 * of the original file size.  The loader can keep reading sectors from the
	 * SD card (or NAND) until it fills memory, and presumably that is exactly
	 * what it does.
	 */
	extra_size = image_info.pad_size + EXTRA_BOOT;

	rh->init_boot_size = extra_size / BLOCK_SIZE;
	// printf ( "extra size = %d %x %d\n", rh->init_boot_size, rh->init_boot_size, extra_size );

	rock_encode ( (char *) rh, sizeof(struct rock_header) );
}

void
setup_image_sizes ( int ifd )
{
	struct stat stbuf;
	int size;
	int pad_size;

	if ( fstat(ifd, &stbuf) < 0)
	    error ( "Cannot get input image size" );

	image_info.size = stbuf.st_size;

	image_info.pad_size = ROUNDUP ( image_info.size, SIZE_ALIGN );

	// printf ( "size, aligned size = %d %d\n", image_info.size, image_info.pad_size );
}

void
write_image ( int ifd, int ofd )
{
	char *imp;

	/* Here is a clever way to copy a file,
	 * stolen from u-boot.  mmap the input file,
	 * then write from that mapped address.
	 */
	// imp = mmap(0, image_info.size, PROT_READ | PROT_WRITE, MAP_SHARED, ifd, 0);
	imp = mmap(0, image_info.size, PROT_READ | PROT_WRITE, MAP_PRIVATE, ifd, 0);
        if ( imp == MAP_FAILED )
	    error ( "Cannot map/read image" );

	/* Overwrite the first 4 bytes with this "header"
	 * This seems like a suspicious thing to do, but the files I have
	 * examined just have a branch to the next instruction in this place,
	 * which can safely be overwritten.
	 */
	memcpy ( imp, "RK33", 4 );

	write ( ofd, imp, image_info.size );
	munmap ( (void *)imp, image_info.size);
        close ( ifd );

	write_zero_pad ( ofd, image_info.pad_size - image_info.size );
}

/* Quick and dirty, no error messages */
int
main ( int argc, char **argv )
{
	int in_fd, out_fd;
	struct rock_header hdr;

	--argc;
	++argv;
	if ( argc < 2 ) {
	    error ( "usage: mkrock infile outfile" );
	}

	in_fd = open ( argv[0], O_RDONLY );
	if ( in_fd < 0 )
	    error ( "Cannot open input file" );

	out_fd = open ( argv[1], O_WRONLY | O_CREAT, 0664 );
	if ( out_fd < 0 )
	    error ( "Cannot open output file" );

	setup_image_sizes ( in_fd );

	make_header ( &hdr );
	write ( out_fd, (char *) &hdr, sizeof(hdr) );
	write_zero_pad ( out_fd, 512 * (INIT_OFFSET - 1) );

	write_image ( in_fd, out_fd );

	close ( out_fd );
	close ( in_fd );

	return 0;
}

/* THE END */

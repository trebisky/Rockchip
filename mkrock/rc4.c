// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 *
 * (C) Copyright 2008-2014 Rockchip Electronics
 *
 * Rivest Cipher 4 (RC4) implementation
 */

/* Taken from u-boot/lib/rc4.c
 * the buffer is overwritten as it is encoded.
 */

void
rc4_encode( unsigned char *buf, unsigned int len, unsigned char key[16] )
{
	unsigned char s[256], k[256], temp;
	unsigned short i, j, t;
	int ptr;

	j = 0;
	for (i = 0; i < 256; i++) {
		s[i] = (unsigned char)i;
		j &= 0x0f;
		k[i] = key[j];
		j++;
	}

	j = 0;
	for (i = 0; i < 256; i++) {
		j = (j + s[i] + k[i]) % 256;
		temp = s[i];
		s[i] = s[j];
		s[j] = temp;
	}

	i = 0;
	j = 0;
	for (ptr = 0; ptr < len; ptr++) {
		i = (i + 1) % 256;
		j = (j + s[i]) % 256;
		temp = s[i];
		s[i] = s[j];
		s[j] = temp;
		t = (s[i] + (s[j] % 256)) % 256;
		buf[ptr] = buf[ptr] ^ s[t];
	}
}

/* I add my simple front end to play with it.
 * Note that this decrypts itself, you just run
 * an encrypted file through and you get back what
 * you started with.  And it is byte by byte, the
 * encrypted file size is the same as the original.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE	64*1024

char buffer[BUF_SIZE];

/* Here is the Rockchip key */
static unsigned char rc4_key[16] = {
        124, 78, 3, 4, 85, 5, 9, 7,
        45, 44, 123, 56, 23, 13, 23, 17
};

/* Quick and dirty, no error messages */
int
main ( int argc, char **argv )
{
	int fd, n;

	--argc;
	++argv;
	if ( argc < 1 )
	    return 1;

	fd = open ( *argv, O_RDONLY );
	if ( fd < 0 )
	    return 1;
	n = read ( fd, buffer, BUF_SIZE );
	if ( n <= 0 )
	    return 1;

	rc4_encode ( buffer, n, rc4_key );
	write ( 1, buffer, n );

	return 0;
}

/* THE END */

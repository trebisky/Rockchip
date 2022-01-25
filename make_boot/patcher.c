/* patcher.c
 *
 * Patch some env settings in a U-boot executable
 * 
 * Taken from the EBAZ4205 "setup" project
 *
 * Tom Trebisky  1-7-2021 for ebaz4205
 * Tom Trebisky  1-23-2022 for RK3399
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

// char *infile = "b19.img";
// char *outfile = "b19p.img";

// char *infile = "u1";
char *infile = "u1p";
char *outfile = "u1p";

// #define IM_SIZE		128*1024
// #define ENV_OFFSET	0xbb5a
#define IM_SIZE		1024*1024
#define ENV_OFFSET	0x8c78a

char image[IM_SIZE];

/* 128K image is 0 to 0x1ffff */

/* ------------ */

void
read_file ( char *buf )
{
    int f;
    int n;

    f = open ( infile, O_RDONLY );
    n = read ( f, buf, IM_SIZE );
    close ( f );
}

void
write_file ( char *buf )
{
    int f;
    int n;

    f = creat ( outfile, 0664 );
    n = write ( f, buf, IM_SIZE );
    close ( f );
}

void
dump_env ( char *env )
{
    char *p;

    p = env;
    while ( *p ) {
	printf ( "%s\n", p );
	p += strlen(p) + 1;
    }

}

static char *ip;

void
init_env ( void )
{
    ip = &image[ENV_OFFSET];
}

void
add_env ( char *s )
{
    strcpy ( ip, s );
    ip += strlen(s) + 1;
}

int
main ( int argc, char **argv )
{
    read_file ( image );
    dump_env ( &image[ENV_OFFSET] );
    exit (1);

    /* This overwrites the entire env area!!
     */
    init_env ();
    add_env ( "ipaddr=192.168.0.35" );
    add_env ( "serverip=192.168.0.5" );
    // add_env ( "ethaddr=00:0a:35:00:01:22" );
    add_env ( "ethaddr=52:b0:37:c3:9b:ec" );
    add_env ( "bootdelay=3" );
    add_env ( "bootaddr=0x20000000" );
    add_env ( "boot_kyu=echo Booting Kyu via dhcp ; dhcp ${bootaddr}; go ${bootaddr}" );
    add_env ( "boot_tftp=echo Booting Kyu via tftp ; tftpboot ${bootaddr} rock.bin; go ${bootaddr}" );
    // Go with tftp, dhcp gets weird ideas about the filename (C0A80050.img)
    // This is the IP address converted to hex, but it should be using bitcoin.bin
    add_env ( "bootcmd=run boot_tftp" );
    // add_env ( "bootcmd=run boot_kyu" );
    add_env ( "" );

    /* XXX ??? Does U-Boot do a CRC check on the env information ? */
    /* And what about our boot address ? */

    write_file ( image );

    printf ( "\n" );
    dump_env ( &image[ENV_OFFSET] );
}

/* THE END */

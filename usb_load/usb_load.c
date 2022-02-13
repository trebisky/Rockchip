/* usb_load.c
 *
 * Tom Trebisky  2-8-2022
 *
 * Use a USB connection to the RK3399 to
 * talk to the bootrom and load and run an image.
 *
 * usb_load with no arguments -  scans USB for the rockchip and exits
 * usb_load -d - will download the DDR loader to sram
 * usb_load path - will download your gadget to sram
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

void load_image_sram ( char * );
void load_image_ddr ( char * );
void load_image ( char *, int );
int send_image ( unsigned char *, int, int );

int usb_find_rk ( void );
void usb_open_rk ( void );
void usb_close_rk ( void );
int usb_send_rk ( int, char *, int );

int crc_calc ( unsigned char *, int );
void rc4 ( unsigned char*, int );

#define	DDR	"ddr.img"

/* Notes --
 *
 * Trying to "chain" two things to SRAM just gets the message "Soft reset" on
 *  the second load (at least when the first was our ddr.img.
 */

void
error ( char *msg )
{
	fprintf ( stderr, "%s\n", msg );
	exit ( 1 );
}

int 
main ( int argc, char **argv )
{
	int n;
	char *path = NULL;
	int ddr_load = 0;

	argc--;
	argv++;

	while ( argc-- ) {
	    if ( argv[0][0] == '-' ) {
		ddr_load = 1;
	    } else {
		path = argv[0];
	    }
	    argv++;
	}

	n = usb_find_rk ();
	if ( n < 1 )
	    error ( "Cannot find any RK3399 devices" );

	usb_open_rk ();

	if ( ddr_load )
	    load_image_sram ( DDR );

	/* A second is really too long, but we do
	 * need some delay.
	 */
	if ( ddr_load && path )
	    sleep ( 1.0 );

	if ( path )
	    load_image_ddr ( path );

	usb_close_rk ();

	return 0;
}

#define MAX_IMAGE_SIZE	128*1024

unsigned char buffer[MAX_IMAGE_SIZE];

void
load_image ( char *path, int type )
{
	int fd;
	int n;

	fd = open ( path, O_RDONLY );
	if ( fd < 0 )
            error ( "File open failed" );

	n = read ( fd, buffer, MAX_IMAGE_SIZE );
	printf ( "Image read: %d bytes\n", n );

	/* We probably read the first part of an image
	 * that is bigger than our buffer.
	 */
	if ( n == MAX_IMAGE_SIZE )
	    error ( "Image too big" );

	close ( fd );

	if ( send_image ( buffer, n, type ) )
	    error ( "Error sending image" );
}

void
load_image_sram ( char *path )
{
	load_image ( path, 0x471 );
}

void
load_image_ddr ( char *path )
{
	load_image ( path, 0x472 );
}

#define CHUNK_SIZE	4096
// #define CHUNK_SIZE	10000	// fails
// #define CHUNK_SIZE	8192	// fails
// #define CHUNK_SIZE	2048	// ok

/* We can send either to SRAM (at ff8c2000) or DDR ram (at 0)
 */
int
send_image ( unsigned char *buf, int size, int type )
{
	int crc;
	int len;
	int sent;
	int nio;
	int n;
	int rem;
	// int tail_packet;
	// char extra = 0;

#ifdef notdef
	/* This is unnecessary given our rounding up to
	 * full multiples of CHUNK_SIZE below.
	 */
	if ( size & 0x1 )
	    size++;

	/* Some of my executables are quite small and trying to
	 * send a single small buffer got an error return.
	 * (although it actually worked just fine).
	 * This does no harm and avoids getting any error
	 * that might frighten the users (namely me).
	 */
	if ( size < 4096 )
	    size = 4096;
#endif

	/* This is my way of bypassing whatever this "tail packet"
	 * rubbish is all about.  I just ensure that we never
	 * send any packet that isn't 4096 bytes.  Sending padding
	 * does no harm and certainly doesn't slow things down.
	 * This also covers the above case.
	 */
	rem = (size) % CHUNK_SIZE;
	if ( rem )
	    size += (CHUNK_SIZE-rem);

	rc4 ( buf, size );

#ifdef notdef
	/* I have no idea what this is all about */
	tail_packet = 0;
	if ( (size % CHUNK_SIZE) == CHUNK_SIZE-2 )
	    tail_packet = 1;
#endif

	crc = crc_calc ( buf, size );

	/* Given the rounding above, this will always be sent
	 * as a final 2 byte write.
	 *
	 * I tried inluding it in the last tidy CHUNK_SIZE
	 * packet, but this actually causes the download
	 * to fail (which is what the old tail_packet
	 * logic was designed to avoid.) Doing this works fine
	 * and seems to handle all the cases.
	 *
	 * Here is what I have concluded about this and the "tail_packet"
	 * business.  It could be a firmware bug, but more likely I think
	 * is that the bootrom just thinks there is more coming as long
	 * as we keep sending full CHUNKs.  A partial chunk is needed to
	 * tell it the end has arrived.
	 */
	buf[size] = (crc >> 8) & 0xff;
	buf[size+1] = crc & 0xff;
	len = size +2;

	sent = 0;

	while ( sent < len ) {
	    nio = len - sent;
	    if ( nio > CHUNK_SIZE ) nio = CHUNK_SIZE;
	    n = usb_send_rk ( type, buf+sent, nio );
	    if ( n != nio ) {
		fprintf ( stderr, "Write error: %d\n", n );
		return 1;
	    }
	    sent += n;
	    // printf ( "Wrote (0x%x): %d --> %d\n", type, n, sent );
	}

#ifdef notdef
	if ( tail_packet ) {
	    n = usb_send_rk ( type, &extra, 1 );
	    if ( n != 1 )
		return 1;
	}
#endif

	return 0;
}

/* ---------------------------------- */

/* lsusb shows:
Bus 003 Device 027: ID 2207:330c Fuzhou Rockchip Electronics Company RK3399 in Mask ROM mode
 *
 * Here is a note on USB permissions on linux.
 * You could just run this as root.
 * Or you could add some special udev rules just for this device.
 * Or you can make you life easy and set up "usbusers" as follows:
 * 1 - add usbusers to /etc/group
 *     I added this line: usbusers:x:1101:tom
 * 2 - add a rule to /etc/udev/rules.d as follows:
 *     filename:  99-usbusers.rules
 *     SUBSYSTEM=="usb", MODE="0666", GROUP="usbusers"
 * 3 - to avoid needing to reboot:
 *     udevadm control --reload
 *     udevadm trigger
 * This worked just fine for me on my Fedora 34 system.
 */

#include <libusb.h>

#define ROCK_VENDOR	0x2207
#define ROCK_RK3399	0x330c

#ifdef notdef
typedef enum{
        RKUSB_NONE = 0x0,
        RKUSB_MASKROM = 0x01,
        RKUSB_LOADER = 0x02,
        RKUSB_MSC = 0x04
} ENUM_RKUSB_TYPE;
#endif

/* I am new to libusb, and there seem to be few if any introductory tutorials,
 *  so I am going to collect my own notes here.
 * 
 * A key decision is whether to use a synchronous or asynchronous interface.
 *  libusb will allow either.  It would probably be better to call these
 *  blocking versus non-blocking.  I will use "synchronous" here, i.e. the
 *  simple blocking interface.  This means that the libusb_control_transfer()
 *  call will block, which is fine by me, at least to get started.
 *
 * Once you have opened a device, you need to "claim an interface".
 * This is because devices can have multiple interfaces and you have
 * to specify which one your device handle is working with.
 * You get an error if you try to claim a non-existant interface.
 */

struct libusb_device_handle *devh = NULL;

int
usb_find_rk ( void )
{
	libusb_device **list;
	libusb_device *dev;
	struct libusb_device_descriptor desc;
	int s;
	int n;
	int i;
	int bcd;
	int rv;

	s = libusb_init ( NULL );
	if ( s < 0 )
	    error ( "libusb init failed" );

	n = libusb_get_device_list ( NULL, &list );
	if ( n < 0 )
	    error ( "libusb failed to get device list" );
	if ( n == 0 )
	    error ( "libusb device list empty" );

	/* I see bcd == 0x200 in maskrom mode.
	 */
	rv = 0;
	for ( i=0; i<n; i++ ) {
	    dev = list[i];
	    s = libusb_get_device_descriptor ( dev, &desc );
	    // printf ( "usb %d: %x:%x\n", i, desc.idVendor, desc.idProduct );
	    if ( desc.idVendor == ROCK_VENDOR ) {
		bcd = desc.bcdUSB;
		printf ( "Rockchip device: %x:%x %x\n", desc.idVendor, desc.idProduct, bcd );
		rv++;
	    }
	}

	libusb_free_device_list(list, 1);
	return rv;
}

#define OUR_INTERFACE	0

void
usb_open_rk ( void )
{
	int s;
	struct libusb_config_descriptor *pconf=NULL;
	libusb_device *dev;

	devh = libusb_open_device_with_vid_pid ( NULL, ROCK_VENDOR, ROCK_RK3399 );
	if ( ! devh )
	    error ( "Cannot open rockchip device" );

	dev = libusb_get_device ( devh );
	if ( ! devh )
	    error ( "Cannot get libusb device" );

#ifdef notdef
	s = libusb_get_active_config_descriptor ( dev, &pconf );
	if ( s < 0 )
	    error ( "libusb cannot get config" );

	printf ( "We have %d interfaces\n", pconf->bNumInterfaces );
#endif

	s = libusb_claim_interface ( devh, OUR_INTERFACE );
	if ( s < 0 )
	    error ( "libusb cannot claim interface" );
}

void
usb_close_rk ( void )
{
	libusb_release_interface ( devh, OUR_INTERFACE );
	libusb_exit ( NULL );
}

#define OUR_TIMEOUT	0	/* in milliseconds, 0 = unlimited */

/* type is 0x471 or 0x472
 *
 * We do everything with SETUP packets.
 * Don't blame me, I didn't write the bootrom code, but it works so why worry.
 */
int
usb_send_rk ( int type, char *buf, int count )
{
	int len;

	len = libusb_control_transfer ( devh, 0x40, 0xC, 0, type, buf, count, OUR_TIMEOUT );

	/* above,
	 * 0x40 is the "request type" (indicates direction)
	 * 0xC is the "request"
	 * 0 is the "value"
	 * our type is the "index"
	 */
	 return len;
}

/* Taken from rkdeveloptool */

#define poly16_CCITT    0x1021          /* crc-ccitt mask */

unsigned short
CRC_Calculate(unsigned short crc, unsigned char ch)
{
        int i;

        for ( i=0x80; i != 0; i >>= 1 ) {
                if ( (crc & 0x8000) != 0 ) {
                        crc <<= 1;
                        crc ^= poly16_CCITT;
                } else
                        crc <<= 1;

                if ( (ch & i)!=0 )
                        crc ^= poly16_CCITT;
        }
        return crc;
}

int
crc_calc ( unsigned char *buf, int len )
{
	unsigned short crc = 0xffff;

	while ( len-- )
	    crc = CRC_Calculate ( crc, *buf++ );

	return crc;
}

void
rc4 ( unsigned char* buf, int len )
{
        unsigned char S[256],K[256];
	/* Rockchip key */
        unsigned char key[16]={124,78,3,4,85,5,9,7,45,44,123,56,23,13,23,17};
        int i,j,t,x;
	int temp;

        j = 0;
        for(i=0; i<256; i++){
                S[i] = (unsigned char)i;
                j&=0x0f;
                K[i] = key[j];
                j++;
        }

        j = 0;
        for(i=0; i<256; i++){
                j = (j + S[i] + K[i]) % 256;
                temp = S[i];
                S[i] = S[j];
                S[j] = temp;
        }

        i = j = 0;
        for(x=0; x<len; x++){
                i = (i+1) % 256;
                j = (j + S[i]) % 256;
                temp = S[i];
                S[i] = S[j];
                S[j] = temp;
                t = (S[i] + (S[j] % 256)) % 256;
                buf[x] = buf[x] ^ S[t];
        }
}


/* THE END */

/* usb_load.c
 *
 * Tom Trebisky  2-8-2022
 *
 * Use a USB connection to the RK3399 to
 * talk to the bootrom and load and run an image.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

int usb_find_rk ( void );
void usb_open_rk ( void );
void usb_close_rk ( void );

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

	n = usb_find_rk ();
	if ( n < 1 )
	    error ( "Cannot find any RK3399 devices" );
	usb_open_rk ();
	usb_close_rk ();

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
 *  simple blocking interface.  This means that the libusb_bulk_transfer()
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

/* THE END */

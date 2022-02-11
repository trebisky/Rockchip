# usb_load
#
# A tool to load binary images to the RK3399
# via a USB connection to the bootrom
#
# Tom Trebisky  2-8-2022

all:	usb_load unpack

CFLAGS_USB = -I/usr/include/libusb-1.0
# CFLAGS = -g -O2
# CFLAGS = -O2
CF = $(CFLAGS_USB)

usb_load:	usb_load.c
	cc $(CF) -o usb_load usb_load.c -lusb-1.0

unpack:	unpack.c
	cc -o unpack unpack.c

get1:
	cp ../bare_hello/bare.bin ./hello_sram.bin

clean:
	rm -f usb_load

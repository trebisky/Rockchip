# Create a bootable SD card for the RK3399
# Tom Trebisky  1-11-2022

# Remember than "dd" records are 512 bytes by default
#
# Coincidentally the uboot and trust images are
#  both 8192 records in size (4M).
# The idbloader is # 404 records (202k)
#
# I plug in a new out of the box SanDisk 16 G card
# and see one partition:
# /dev/sdh1        8192 31116287 31108096 14.8G  c W95 FAT32 (LBA)

# WARNING!  Be very careful about this device name.
# since you might run this as root, you could make a mistake
# and give say the device for your system disk and wipe out
# the install on your system drive!!!

#CARD = /dev/null
CARD = /dev/sdh
#CARD = /dev/mmcblk1
NULL = /dev/null

all:	sdcard

sdnull:
	dd if=idbloader.bin of=$(NULL) seek=64 conv=notrunc
	dd if=uboot.img of=$(NULL) seek=16384 conv=notrunc
	dd if=trust.bin of=$(NULL) seek=24576 conv=notrunc

sdcard:
	dd if=/dev/zero of=$(CARD) count=64
	dd if=idbloader.bin of=$(CARD) seek=64 conv=notrunc
	dd if=uboot.img of=$(CARD) seek=16384 conv=notrunc
	dd if=trust.bin of=$(CARD) seek=24576 conv=notrunc
	sync

idb:
	dd if=/dev/zero of=$(CARD) count=62000
	dd if=idbloader.bin of=$(CARD) seek=64 conv=notrunc
	sync

# Running fdisk on my bootable Debian image SD card shows:
#  Disk /dev/sdh: 14.84 GiB, 15931539456 bytes, 31116288 sectors
#  /dev/sdh1       61440 30805119 30743680 14.7G 83 Linux

sdgrab:
	dd if=$(CARD) of=myboot.img count=61440
	dd if=$(CARD) of=zi.img skip=64 count=404
	dd if=$(CARD) of=zu.img skip=16384 count=8192
	dd if=$(CARD) of=zt.img skip=24576 count=8192

install:
	dd if=myboot.img of=$(CARD)
	sync

setup:
	parted -s $(CARD) -- mklabel msdos


# The card I was experimenting with apparently failed.
# It now shows a 31M capacity rather than 16G
# This is a commonly reported problem that can sometimes
# be recovered by "formatting software" that runs under windows.

size:
	parted $(CARD) unit s print -sm

check:
	fdisk -l $(CARD)

zap:
	dd if=/dev/zero of=$(CARD) bs=8M count=5

# This is from the nand-sata-install script
# it is claimed to delete all partitions on the eMMC drive
# Note that the count is in bytest
nukepart:
	dd bs=1 seek=446 count=64 if=/dev/zero of=$(CARD)


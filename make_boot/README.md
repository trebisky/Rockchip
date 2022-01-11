This collection of files allows you to create a microSD (TF) card
that will launch U-Boot for the Orange Pi 4 with the RK3399.

1-11-2022

The idea is to start with 3 binary files that I found in
the /usr/lib/linux-u-boot-legacy-orangepi4_2.1.0_arm64
directory on my RK3399 debian distribution and then to use
appropriate "dd" commands to put them on the SD card.

I fetched them from the debian system running on the RK3399 board
via:

* scp root@rock:/usr/lib/linux-u-boot-legacy-orangepi4_2.1.0_arm64/*.bin .
* scp root@rock:/usr/lib/linux-u-boot-legacy-orangepi4_2.1.0_arm64/*.img .

* ls -l
* -rw-rw-r-- 1 tom tom  206844 Jan 11 13:14 idbloader.bin
* -rw-rw-r-- 1 tom tom 4194304 Jan 11 13:14 trust.bin
* -rw-rw-r-- 1 tom tom 4194304 Jan 11 13:14 uboot.img


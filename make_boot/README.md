This collection of files allows you to create a microSD (TF) card
that will launch U-Boot for the Orange Pi 4 with the RK3399.

This project is unfinished, but seems to work.
I'll get back to it someday (I hope)

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

I had trouble with one SD card, and now for the record, here is how a second
card behaved.  Fresh from the package, I used "dd" to copy a 4G debian image
onto it as follows:

dd if=Orangepi4_2.1.0_debian_buster_desktop_linux4.4.179.img of=/dev/sdh bs=16M
222+1 records in
222+1 records out
3732930560 bytes (3.7 GB, 3.5 GiB) copied, 192.317 s, 19.4 MB/s

The copy takes about 3.5 minutes with this 16M blocksize.

After this, I removed and replugged the card, the read back the image in order
to verify it.  It is not entirely surprising that read speed is over 3 times
faster than writing.  The verify was satisfactory.
The image is exactly 3732930560 bytes in size

dd if=/dev/sdh of=check.img bs=16M count=223
223+0 records in
223+0 records out
3741319168 bytes (3.7 GB, 3.5 GiB) copied, 55.129 s, 67.9 MB/s
cmp Orangepi4_2.1.0_debian_buster_desktop_linux4.4.179.img check.img
cmp: EOF on Orangepi4_2.1.0_debian_buster_desktop_linux4.4.179.img after byte 3732930560, in line 23759814

After all of this, fdisk shows:
Disk /dev/sdh: 14.84 GiB, 15931539456 bytes, 31116288 sectors
/dev/sdh1       61440 7290879 7229440  3.4G 83 Linux

With this new and verified card, simply putting the idbloader on the card yields plenty of
messages on the serial console.  Clearly my failure with the first card was due to a bad SD card.




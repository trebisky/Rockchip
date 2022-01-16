mkrock - make a bootable idbloader file

1-11-2022

You can just user the mkimage program that ships with U-Boot to do this.
You will find this in the tools directory and you could run it as:

mkimage -n "rk3399" -T rksd -d myfile.bin myidb.img

But I want to study mkimage (which is a swiss army style tool that handles
a multitude of image types) and fully understand how to prepare an image
that the RK3399 bootloader will load and run.



# Create a bootable SD card for the RK3399
# Tom Trebisky  1-11-2022

all:	rc4 mkrock

rc4:	rc4.c
	cc -DWITH_MAIN -o rc4 rc4.c

ROBJS = mkrock.o rc4.o

mkrock:	$(ROBJS)
	cc -o mkrock $(ROBJS)

install:	mkrock
	cp mkrock /usr/local/bin

# The two test files, one an input and the other
#  the expected output were obtained from the u-boot/tpl directory
test:
	./mkrock u-boot-tpl.bin mkrock.bin
	-cmp mkrock.bin u-boot-tpl-rockchip.bin
	odx mkrock.bin >mkrock.odx
	odx u-boot-tpl-rockchip.bin >rockchip.odx
	diff mkrock.odx rockchip.odx



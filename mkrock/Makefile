# Create a bootable SD card for the RK3399
# Tom Trebisky  1-11-2022

all:	rc4 mkrock

rc4:	rc4.c
	cc -DWITH_MAIN -o rc4 rc4.c

ROBJS = mkrock.o rc4.o

mkrock:	$(ROBJS)
	cc -o mkrock $(ROBJS)


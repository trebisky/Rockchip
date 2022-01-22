# arm_wrap - convert a binary file into
# an elf file to allow improved disassembly
#
# Tom Trebisky  12-14-2016  8-21-2018 1-21-2022

DUMP32 = arm-linux-gnu-objdump
MACH32 = -marm
DUMP = aarch64-linux-gnu-objdump
MACH = -maarch64

all:	arm_wrap dump8 bootrom.dis

arm_wrap:	arm_wrap.c
	cc -o arm_wrap arm_wrap.c

dump8:	dump8.c
	cc -o dump8 dump8.c

#install: arm_wrap
#	cp arm_wrap /home/tom/bin

.PHONY:	naive.dis

# This yields something reasonable
naive.dis: bootrom.bin
	$(DUMP) -D -b binary $(MACH) bootrom.bin >naive.dis

# This yields nonsense
naive32.dis: bootrom.bin
	$(DUMP32) -D -b binary $(MACH32) bootrom.bin >naive32.dis

# objdump will suppress zones with zero bytes, displaying
# simply "..." (3 dots) for any number of zeros.
# The -z option tells it to show all those zeros.
bootrom.dis: bootrom.elf
	$(DUMP) $(MACH) -d bootrom.elf -z >bootrom.dis

bootrom.elf:    arm_wrap bootrom.bin
	./arm_wrap -bffff0000 bootrom.bin bootrom.elf

bootrom.bin:    rom.txt
	./tobin

clean:
	rm -f arm_wrap
	rm -f naive.dis
	rm -f bootrom.elf
	rm -f bootrom.dis
	rm -f bootrom.bin


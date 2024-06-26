# Makefile for Rockchip RK3399 first hello demo
# Adaped from my h5 and Zynq projects
# Tom Trebisky  1-1-2022 (Happy New Year!)
# tom@mmto.org

BOARD = bare
CROSS_COMPILE = aarch64-linux-gnu-

# -------------------------------------

OBJS = start.o hello.o uart.o

TARGET = $(BOARD).bin

# CFLAGS		:=	-g -Wall -Wextra -ffreestanding -fno-builtin -mlittle-endian
CFLAGS		:=	-g -Wall -ffreestanding -fno-builtin -mlittle-endian
CFLAGS		+= -march=armv8-a+crc
CFLAGS		+= -mtune=cortex-a53
CFLAGS		+= -I.

LDFLAGS		:=	-Bstatic \
			-Tbare.lds \
			-Wl,--start-group \
			-Wl,--end-group \
			-Wl,--build-id=none \
			-nostdlib

LD_DDR_FLAGS	:=	-Bstatic \
			-Tbare_ddr.lds \
			-Wl,--start-group \
			-Wl,--end-group \
			-Wl,--build-id=none \
			-nostdlib

CC			=	$(CROSS_COMPILE)gcc $(CFLAGS)
LD 			=	$(CROSS_COMPILE)gcc $(LDFLAGS)
LD_DDR 			=	$(CROSS_COMPILE)gcc $(LD_DDR_FLAGS)
OBJCOPY			=	$(CROSS_COMPILE)objcopy
DUMP			=	$(CROSS_COMPILE)objdump

# This gives us dependencies in .d files.
# CFLAGS		+= -MMD
# This gives us a map file.
# CFLAGS		+= -Wl,-Map=$(BOARD).map,--cref \

.c.o:
	@echo " [CC]   $<"
	@$(CC) $< -c -o $@

.S.o:
	@echo " [CC]   $<"
	@$(CC) $< -c -o $@

# -------------------------------------

all: install bare_ddr.bin
#all: $(TARGET)

install: $(TARGET)
	#cp $(TARGET) /var/lib/tftpboot
	mkrock bare.bin bare.img

CARD = /dev/sdh

# You must be root to do this on my system --
# People say that notruc is pointless for a block device,
#  but we retain it for old times sake.
card:	bare.bin
	mkrock bare.bin bare.img
	dd if=/dev/zero of=$(CARD) count=64
	dd if=bare.img of=$(CARD) seek=64 conv=notrunc
	sync

$(BOARD).elf: $(OBJS)
	@echo " [LD]   $(BOARD).elf"
	@$(LD) $(OBJS) -o $(BOARD).elf

bare_ddr.elf:	$(OBJS)
	@echo " [LD]   bare_ddr.elf"
	@$(LD_DDR) $(OBJS) -o bare_ddr.elf

bare_ddr.bin: bare_ddr.elf
	@echo " [IMG]  bare_ddr.bin"
	@$(OBJCOPY) -O binary bare_ddr.elf bare_ddr.bin

$(TARGET): $(BOARD).elf
	@echo " [IMG]  $(TARGET)"
	@$(OBJCOPY) -O binary $(BOARD).elf $(TARGET)

dis: $(BOARD).elf
	$(DUMP) -d $(BOARD).elf -z >$(BOARD).dis

fetch:
	cp ../USB_loader/loader tools

.PHONY: clean
clean:
	rm -f *.o
	rm -f *.img
	rm -f *.elf
	rm -f *.bin
	rm -f *.map
	rm -f *.dis
#	rm -f *.d

# THE END

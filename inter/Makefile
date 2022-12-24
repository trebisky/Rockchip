# Makefile for Rockchip RK3399 first hello demo
# Adaped from my h5 and Zynq projects
# Tom Trebisky  1-1-2022 (Happy New Year!)
# tom@mmto.org

BOARD = rock
CROSS_COMPILE = aarch64-linux-gnu-

# -------------------------------------

OBJS = start.o prf.o uart.o gpio.o timer.o gic500.o trap.o mmu.o main.o

TARGET = $(BOARD).bin

# CFLAGS		:=	-g -Wall -Wextra -ffreestanding -fno-builtin -mlittle-endian
# CFLAGS		:=	-g -Wall -ffreestanding -fno-builtin -mlittle-endian
CFLAGS		:=	-g -ffreestanding -fno-builtin -mlittle-endian
CFLAGS		+= -march=armv8-a+crc
CFLAGS		+= -mtune=cortex-a53
CFLAGS		+= -I.

LDFLAGS		:=	-Bstatic \
			-Tspl.lds \
			-Wl,--start-group \
			-Wl,--end-group \
			-Wl,--build-id=none \
			-nostdlib

CC			=	$(CROSS_COMPILE)gcc $(CFLAGS)
LD 			=	$(CROSS_COMPILE)gcc $(LDFLAGS)
OBJCOPY			=	$(CROSS_COMPILE)objcopy
DUMP			=	$(CROSS_COMPILE)objdump -d

LOAD			=	tools/loader -h64

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

#rock.dump:
#	$(DUMP) rock.elf rock.dump

# -------------------------------------

all: install
#all: $(TARGET)

install: $(TARGET)
	cp $(TARGET) /var/lib/tftpboot

gic500.o: gic500.c rk3399_ints.h

$(BOARD).elf: $(OBJS)
	@echo " [LD]   $(BOARD).elf"
	@$(LD) $(OBJS) -o $(BOARD).elf

$(TARGET): $(BOARD).elf
	@#echo " [IMG]  $(TARGET)
	@$(OBJCOPY) -O binary $(BOARD).elf $(TARGET)

dis: $(BOARD).elf
	$(DUMP) -d $(BOARD).elf -z >$(BOARD).dis

fetch:
	cp ../USB_loader/loader tools

borrow::
	cp /u1/Projects/Fire3/Github/Timer/trap.c .
	cp /u1/Projects/Fire3/Github/Timer/start.S .

# -------------------
# You will need to be root for either of these to work.

usb:  $(TARGET)
	$(LOAD) $(TARGET)

sdcard:  $(TARGET)
	$(LOAD) -o $(TARGET) | dd of=/dev/sdf seek=1 conv=fdatasync

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

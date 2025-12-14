# Makefile for the ARM gic driver subdirectory.
#  part of my Rockchip RK3399 inter demo
# Tom Trebisky  12-8-2025

CROSS_COMPILE = aarch64-linux-gnu-

# -------------------------------------

OBJS = gic_compat.o gicv3_main.o gicdv3_helpers.o gicrv3_helpers.o gicv3_helpers.o \
	gic-x00.o arm_gicv3_common.o \
	my_core.o cache_helpers.o spinlock.o

# We don't use this (see the base directory)
NOT_OBJS = gicv3_base.o
#CFLAGS		+= -DLOG_LEVEL=30 -DUSE_GIC_DRIVER=3
# The file gicv3_main.c complains without the DRIVER=3

# CFLAGS		:=	-g -Wall -Wextra -ffreestanding -fno-builtin -mlittle-endian
# CFLAGS		:=	-g -Wall -ffreestanding -fno-builtin -mlittle-endian
CFLAGS		:=	-g -ffreestanding -fno-builtin -mlittle-endian
CFLAGS		+= -march=armv8-a+crc
CFLAGS		+= -mtune=cortex-a53
CFLAGS		+= -nostdinc
CFLAGS		+= -I. -I./include
CFLAGS		+= -DLOG_LEVEL=30 -DUSE_GIC_DRIVER=3
# see debug.h for LOG_LEVEL
# it should be a multiple of 10.

CC			=	$(CROSS_COMPILE)gcc $(CFLAGS)
LD 			=	$(CROSS_COMPILE)gcc $(LDFLAGS)
OBJCOPY			=	$(CROSS_COMPILE)objcopy
DUMP			=	$(CROSS_COMPILE)objdump -d

.c.o:
	@echo " [CC]   $<"
	@$(CC) $< -c -o $@

.S.o:
	@echo " [AS]   $<"
	@$(CC) $< -c -o $@

# -------------------------------------

#all: install
#all: ../gic.o
all: $(OBJS)
	$(LD) -r -o ../gic.o $(OBJS)

../gic.o: $(OBJS)
	$(LD) -r -o ../gic.o $(OBJS)

.PHONY: clean
clean:
	rm -f *.o

# THE END

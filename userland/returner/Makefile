SRCS := $(wildcard src/*.c) $(wildcard src/*.s)
CFLAGS := -ffreestanding -fPIE -static -nostdlib -nostdinc

CC ?= cc

compile:
	$(CC) $(CFLAGS) -T link.ld $(SRCS) -o returner

install:
	doas cp returner /mnt/misc/elf


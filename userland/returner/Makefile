SRCS := $(wildcard src/*.c) $(wildcard src/*.s)
CFLAGS := -ffreestanding -fPIE -static -nostdlib -nostdinc

CC ?= cc

todisk: compile
	cp returner ../../disk.img

compile:
	$(CC) $(CFLAGS) -T link.ld $(SRCS) -o returner

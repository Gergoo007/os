SRCS := $(wildcard *.c) $(wildcard *.s)
CFLAGS := -ffreestanding -fPIE -static -nostdlib -nostdinc

CC ?= cc

install: compile
	tar uf ../../bootstuff/initrd init

compile:
	$(CC) $(CFLAGS) -T link.ld $(SRCS) -o init

SRCS := $(wildcard src/*.c) $(wildcard src/*.s)
CFLAGS := -ffreestanding -fPIE -static -nostdlib -nostdinc

CC ?= gcc

compile:
	$(CC) $(CFLAGS) -T link.ld $(SRCS) -o returner

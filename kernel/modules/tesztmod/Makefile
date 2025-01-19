# Disable idiotic invisible default variables like CC
MAKEFLAGS += --no-builtin-variables

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CSRCS := $(subst ,,$(call rwildcard,src,*.c))
SSRCS := $(subst ,,$(call rwildcard,src,*.s))

# no-red-zone: painful addition
_CFLAGS := $(CFLAGS) -mgeneral-regs-only -ffreestanding -nostdlib -nostdinc \
	-I../../src -xc -std=gnu23 -Wall -Wextra -Wshadow -Wno-address-of-packed-member \
	-Wno-unused-parameter -fno-stack-protector -mno-red-zone -O0 -g -Wno-multichar \
	-Wno-packed-bitfield-compat -msse -msse2

ASFLAGS :=
LDFLAGS := -T module.ld -z max-page-size=0x1000 --just-symbols=kernel.syms -pie

CC := gcc
LD ?= ld.gold
AS ?= as

MOD_NAME := $(notdir $(CURDIR))

module: clean
	$(CC) $(_CFLAGS) $(CSRCS) $(SSRCS) -c -o out/module.o
	strip -R .eh_frame -R .rela.eh_frame --strip-debug out/module.o

install: module
	mcopy -n -o -i ../../../disk.img@@1048576 out/module.o ::modules/$(MOD_NAME).km

clean:
	rm -rf out/*
	find src -type d -exec mkdir -p "out/{}" \;
	cp -r out/src/* out/ 2> /dev/null || :
	rm -rf out/src

$(V).SILENT:

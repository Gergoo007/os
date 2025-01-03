# Disable idiotic invisible default variables like CC
MAKEFLAGS += --no-builtin-variables

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CSRCS := $(subst src/,,$(call rwildcard,src,*.c))
COBJS := $(patsubst %.c,out/%.c.o,$(CSRCS))

SSRCS := $(subst src/,,$(call rwildcard,src,*.s))
SOBJS := $(patsubst %.s,out/%.s.o,$(SSRCS))

# no-red-zone: painful addition
_CFLAGS := $(CFLAGS) -mgeneral-regs-only -ffreestanding -nostdlib -nostdinc \
	-Isrc -xc -std=gnu23 -Wall -Wextra -Wshadow -Wno-address-of-packed-member \
	-Wno-unused-parameter -fno-stack-protector -mno-red-zone -O0 -g -Wno-multichar \
	-Wno-packed-bitfield-compat -msse -msse2
ASFLAGS :=
LDFLAGS := -T module.ld -z max-page-size=0x1000 -pie

CC := gcc
LD ?= ld.gold
AS ?= as

MOD_NAME := $(notdir $(CURDIR))

module: clean
	@$(MAKE) --no-print-directory link

link: $(SOBJS) $(COBJS)
	@echo "  > [$(MOD_NAME)] $(LD) $@"
	@$(LD) $(LDFLAGS) $^ -o out/module

out/%.c.o: src/%.c
	@echo "  > [$(MOD_NAME)] $(CC) $^"
	@$(CC) $(_CFLAGS) -c $^ -o $@

out/%.s.o: src/%.s
	@echo "  > [$(MOD_NAME)] $(AS) $^"
	@$(AS) $(ASFLAGS) -c $^ -o $@

regen_ldscript:


clean:
	@rm -rf out/*
	@find src -type d -exec mkdir -p "out/{}" \;
	@cp -r out/src/* out/ 2> /dev/null || :
	@rm -rf out/src

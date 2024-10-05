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
	-Wno-packed-bitfield-compat
ASFLAGS :=
LDFLAGS := -T ldscript -z max-page-size=0x1000

CC := gcc
LD ?= ld.gold
AS ?= as

kernel: clean
	@$(MAKE) --no-print-directory link

link: $(SOBJS) $(COBJS) out/font.o
	@echo "  > $(LD) $@"
	@$(LD) $(LDFLAGS) $^ -o out/kernel

# out/%.c.o: src/lai/%.c
# 	@echo LAI
# 	@echo "  > $(CC) $^"
# 	@$(CC) $(_CFLAGS) -c $^ -o out/$@

out/%.c.o: src/%.c
	@echo "  > $(CC) $^"
	@$(CC) $(_CFLAGS) -c $^ -o $@

out/%.s.o: src/%.s
	@echo "  > $(AS) $^"
	@$(AS) $(ASFLAGS) -c $^ -o $@

out/font.o: src/font.psf
	@echo "  > $(LD) $^"
	@$(LD) -r -b binary $^ -o $@

clean:
	@rm -rf out/*
	@find src -type d -exec mkdir -p "out/{}" \;
	@mv out/src/* out/
	@rmdir out/src

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
LDFLAGS := -T ldscript -z max-page-size=0x1000

CC := gcc
LD ?= ld.gold
AS ?= as

RECOMPILE_ACPI ?= 0

kernel: clean
	@$(MAKE) --no-print-directory link

link: $(SOBJS) $(COBJS) out/font.o
	@echo "  > $(LD) $@"
	@$(LD) $(LDFLAGS) $^ -o out/kernel

out/acpi/lai/%.c.o: src/acpi/lai/%.c
	@echo LAI
	@echo "  > $(CC) $^"
	@$(CC) $(_CFLAGS) -c $^ -o $@

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
ifeq ($(RECOMPILE_ACPI),1)
	@rm -rf out/*
else
	@find out/ -mindepth 1 -maxdepth 1 -not -path 'out/acpi*' -exec rm -rf {} +
endif
	@find src -type d -exec mkdir -p "out/{}" \;
	@cp -r out/src/* out/
	@rm -rf out/src

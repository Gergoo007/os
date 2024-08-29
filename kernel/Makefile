rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CSRCS := $(subst src/,,$(call rwildcard,src,*.c))
COBJS := $(patsubst %.c,out/%.c.o,$(CSRCS))

SSRCS := $(subst src/,,$(call rwildcard,src,*.s))
SOBJS := $(patsubst %.s,out/%.s.o,$(SSRCS))

_CFLAGS := $(CFLAGS) -mgeneral-regs-only -ffreestanding -nostdlib -nostdinc \
	-Isrc -xc -std=gnu17 -Wall -Wextra -Wshadow -Wno-address-of-packed-member \
	-Wno-unused-parameter -fno-stack-protector -O0 -g \
	-Wno-multichar
ASFLAGS :=
LDFLAGS := -T ldscript -z max-page-size=0x1000

kernel: clean
	@$(MAKE) --no-print-directory link

link: $(SOBJS) $(COBJS) out/font.o
	@echo "  > $(LD) $@"
	@ld.gold $(LDFLAGS) out/*.o -o out/kernel

out/%.c.o: src/%.c
	@echo "  > $(CC) $^"
	@gcc $(_CFLAGS) -c $^ -o out/$(notdir $@)

out/%.s.o: src/%.s
	@echo "  > $(AS) $^"
	@as $(ASFLAGS) -c $^ -o out/$(notdir $@)

out/font.o: src/font.psf
	@echo "  > $(LD) $^"
	@ld.gold -r -b binary $^ -o $@

clean:
	@rm -rf out
	@mkdir out

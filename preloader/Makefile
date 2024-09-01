rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CSRCS := $(subst src/,,$(call rwildcard,src,*.c))
COBJS := $(patsubst %.c,out/%.c.o,$(CSRCS))

SSRCS := $(subst src/,,$(call rwildcard,src,*.s))
SOBJS := $(patsubst %.s,out/%.s.o,$(SSRCS))

CFLAGS := -mgeneral-regs-only -ffreestanding -nostdlib -nostdinc \
	-Isrc -xc -std=gnu17 -Wall -Wextra -Wshadow -Wno-address-of-packed-member \
	-Wno-incompatible-library-redeclaration -Wno-unused-parameter -fno-stack-protector -O0 -g \
	-Wno-multichar
ASFLAGS :=
LDFLAGS := -T ldscript -z max-page-size=0x1000 -static

kernel: clean
	@$(MAKE) --no-print-directory link

link: $(SOBJS) $(COBJS) out/kernel.o
	@echo "  > $(LD) $@"
	@ld.gold $(LDFLAGS) out/*.o -o out/preloader

out/%.c.o: src/%.c
	@echo "  > $(CC) $^"
	@gcc $(CFLAGS) -c $^ -o out/$(notdir $@)

out/%.s.o: src/%.s
	@echo "  > $(AS) $^"
	@as $(ASFLAGS) -c $^ -o out/$(notdir $@)

out/kernel.o: ../kernel/out/kernel
	@echo "  > $(LD) $^"
	@cp $^ kernel
	@strip --strip-all kernel
	@ld.gold -r -b binary kernel -o $@
	@rm kernel

clean:
	@rm -rf out
	@mkdir out

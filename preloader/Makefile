rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

ASFLAGS :=
LDFLAGS := -T ldscript -z max-page-size=0x1000 -static

kernel: clean
	@$(MAKE) --no-print-directory link

link: out/boot.s.o out/kernel.o syms
	@echo "  > $(LD) preloader"
	@ld.gold $(LDFLAGS) out/boot.s.o out/kernel.o out/ksymtab.o out/kstrtab.o out/kshstrtab.o out/kdebug_line.o -o out/preloader

out/%.s.o: src/%.s
	@echo "  > $(AS) $^"
	@as $(ASFLAGS) -c $^ -o out/$(notdir $@)

out/kernel.o: ../kernel/out/kernel
	@echo "  > $(LD) $^"
	@cp $^ kernelelf
	@strip --strip-all kernelelf
	@../tools/kernelsegextractor/segextr kernelelf kernel
	@ld.gold -r -b binary kernel -o $@
	@rm kernel kernelelf

syms:
	@echo "  > tools/extractor ../kernel/out/kernel"
	@../tools/symextractor/extractor ../kernel/out/kernel out/ksymtab out/kstrtab out/kshstrtab out/kdebug_line
	@ld.gold -r -b binary out/ksymtab -o out/ksymtab.o
	@ld.gold -r -b binary out/kstrtab -o out/kstrtab.o
	@ld.gold -r -b binary out/kshstrtab -o out/kshstrtab.o
	@ld.gold -r -b binary out/kdebug_line -o out/kdebug_line.o

clean:
	@rm -rf out
	@mkdir out

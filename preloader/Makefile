rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

ASFLAGS :=
LDFLAGS := -T ldscript -z max-page-size=0x1000 -static

kernel: clean
	@$(MAKE) --no-print-directory link

link: out/boot.s.o out/kernel.o
	@echo "  > $(LD) $@"
	@ld.gold $(LDFLAGS) out/*.o -o out/preloader

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

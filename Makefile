RAMSIZE ?= 4G

_QEMU_FLAGS := -cdrom arzene.iso -no-reboot -no-shutdown -m $(RAMSIZE) -M q35 $(QEMU_FLAGS) \
		-smp 8 -drive id=disk,file=disk.img,if=none \
		-device ich9-usb-uhci6,id=uhci -device qemu-xhci,id=xhci \
		-device ahci,id=ahci \
		-device ide-hd,drive=disk,bus=ahci.0 \
		-device usb-mouse,bus=uhci.0 -device usb-tablet,bus=xhci.0 \
		-boot d -cpu host

_QEMU_FLAGS_UEFI := -drive if=pflash,format=raw,unit=0,file="emustuff/OVMF/OVMF_CODE.fd",readonly=on \
		-drive if=pflash,format=raw,unit=1,file="emustuff/OVMF/OVMF_VARS.fd",readonly=on \
		$(_QEMU_FLAGS) $(QEMU_FLAGS_UEFI)

run: prepare_img
	qemu-system-x86_64 $(_QEMU_FLAGS) -enable-kvm

uefi: prepare_img
	qemu-system-x86_64 $(_QEMU_FLAGS_UEFI) -enable-kvm

test:
	qemu-system-x86_64 $(_QEMU_FLAGS) -cpu qemu64 -d int

debug:
	qemu-system-x86_64 $(_QEMU_FLAGS) -cpu qemu64 \
		-S -s > /dev/null & gdb kernel/out/kernel --eval-command="target remote :1234"

bochs: prepare_img
	bochs -qf emustuff/.bochsrc -rc emustuff/bochscmd

prepare_img:
	@$(MAKE) -C kernel
	@$(MAKE) -C preloader
	@cp preloader/out/preloader bootstuff/kernel
	@grub-mkrescue bootstuff -o arzene.iso --quiet --locales= --themes=

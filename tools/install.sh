make -C kernel
make -C preloader
doas cp preloader/out/preloader /boot/kernel


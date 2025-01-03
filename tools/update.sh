if [[ -b "$1" ]]; then
	make prepare_img
	doas mount $1 /mnt/misc

    doas cp kernel/out/kernel /mnt/kernel
    doas strip /mnt/kernel

	doas umount $1
	sync
	exit 0
else
	echo "Az eszköz nem létezik!"
	exit 1
fi


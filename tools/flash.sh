if [[ -b "$1" ]]; then
	make prepare_img
	doas dd if=arzene.iso of=$1 bs=4M conv=fsync
	exit 0
else
	echo "Az eszköz nem létezik!"
	exit 1
fi


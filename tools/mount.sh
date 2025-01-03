dev=$(doas losetup --find --show --partscan  disk.img)
mntpoint=${1:-/mnt/misc}
doas mount ${dev}p1 $mntpoint


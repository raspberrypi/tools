#!/bin/bash

dd if=/dev/zero of=rpi.img count=0 bs=1 seek=2021654528

sudo sh -c 'cat << EOF | sfdisk --force rpi.img
unit: sectors
1 : start=     2048, size=   204800, Id= c
2 : start=   206848, size=  3741696, Id=83
EOF
'
sudo losetup /dev/loop0 rpi.img -o $((2048*512))
sudo mkfs.vfat -F 32 -n boot /dev/loop0
sudo losetup -d /dev/loop0
sudo losetup /dev/loop0 rpi.img -o $((206848*512))
sudo mkfs.ext4 -L root /dev/loop0
sudo losetup -d /dev/loop0

mkdir -p mnt/{boot,root}
sudo mount -o loop,offset=$((2048*512)) rpi.img mnt/boot
sudo mount -o loop,offset=$((206848*512)) rpi.img mnt/root

sudo rsync -a rootfs/ mnt/root/
#sudo cp -a rootfs/boot/* mnt/boot/
sudo cp -a ../firmware/hardfp/opt/vc mnt/root/opt/
sudo cp -a ../linux/build/dist/lib/modules mnt/root/lib/
sudo cp -a ../linux/build/dist/include/* mnt/root/usr/include
sudo cp ../linux/build/.config mnt/boot/config-3.18.9-preempt-rt5
sudo cp ../linux/build/arch/arm/boot/zImage mnt/boot/kernel.img
sudo cp ../firmware/boot/{*bin,*dat,*elf} mnt/boot/
sudo sh -c 'cat >mnt/boot/config.txt << EOF
kernel=kernel.img
arm_freq=800
core_freq=250
sdram_freq=400
over_voltage=0
gpu_mem=16
EOF
'
sudo sh -c 'cat > mnt/boot/cmdline.txt << EOF
dwc_otg.lpm_enable=0 console=ttyAMA0,115200 kgdboc=ttyAMA0,115200 root=/dev/mmcblk0p2 rootfstype=ext4 elevator=deadline rootwait
EOF
'
sudo umount mnt/{boot,root}

bzip2 -9 rpi.img

# sudo sh -c 'bzcat rpi.img.bz2 > /dev/mmcblk0'

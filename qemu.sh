#!/bin/bash
exec qemu-system-aarch64 -nographic -M virt -m 2048 -cpu cortex-a53 -smp cores=4 -kernel ./arch/arm64/boot/Image -initrd ../image/ramdisk.img -append "bootmode=ramdisk rw root=/dev/ram0 kgdboc=ttyS0,115200 kgdbwait" -serial mon:stdio \
  -drive file=../image/rootfs.img,format=raw,if=sd,id=rootfs -device virtio-blk-device,drive=rootfs \
  -drive file=../image/boot.img,format=raw,if=sd,id=boot -device virtio-blk-device,drive=boot \
  -drive file=../image/modules.img,format=raw,if=sd,id=modules -device virtio-blk-device,drive=modules \
  -drive file=../image/system-data.img,format=raw,if=sd,id=system-data -device virtio-blk-device,drive=system-data

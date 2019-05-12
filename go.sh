./build-rpi3-arm64.sh
sudo ./scripts/mkbootimg_rpi3.sh
rm -rf ../image/
mkdir ../image/
cp boot.img ../image/
cp modules.img ../image/
cp tizen-unified_20181024.1_iot-headless-2parts-armv7l-rpi3.tar.gz ../image/tizen.tar.gz
cp ./copy.sh ../image/
cd ../image/
tar -xvzf tizen.tar.gz
rm tizen.tar.gz
mkdir mntdir
sudo mount rootfs.img ./mntdir/
sudo -s
sudo umount ~/image/mntdir
./qemu.sh

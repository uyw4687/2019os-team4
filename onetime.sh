./build-rpi3-arm64.sh
sudo ./scripts/mkbootimg_rpi3.sh
tar -zcvf tizen-unified_20181024.1_iot-boot-arm64-rpi3.tar.gz boot.img modules.img

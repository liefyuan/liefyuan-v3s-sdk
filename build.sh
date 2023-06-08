#!/bin/bash

TOP_DIR=$(pwd)

# configure Cross compiler

export PATH=$PATH:$TOP_DIR/prebuilts/gcc-linaro-6.3.1-2017.02-x86_64_arm-linux-gnueabihf/bin

export ARCH=arm

export CROSS_COMPILE=arm-linux-gnueabihf-

export gcc=arm-linux-gnueabihf-gcc

check_cross_compiler() {
   log=$(arm-linux-gnueabihf-gcc -v)

   echo $log
}

build_bootcmd() {
    echo "Start build boot cmd!"

    cd $TOP_DIR/u-boot/u-boot-v3s-current
    echo "$TOP_DIR/u-boot/u-boot-v3s-current"

    rm boot.scr 
    rm bootcmd

    #touch "boot.cmd"
    #echo "touch "boot.cmd""

    #echo "setenv bootargs console=ttyS0,115200 root=/dev/mmcblk0p2 rootwait panic=10 earlyprintk rw" > boot.cmd
    #echo "setenv video-mode sunxi:800x480-18@60,monitor=lcd" >> boot.cmd
    #echo "setenv lcd-mode x:800,y:480,depth:18,pclk_khz:10000,le:42,ri:8,up:11,lo:4,hs:1,vs:1,sync:3,vmode:0" >> boot.cmd
    #echo "setenv stderr serial,lcd" >> boot.cmd
    #echo "setenv stdout serial,lcd" >> boot.cmd
    #echo "load mmc 0:1 0x41000000 zImage" >> boot.cmd
    #echo "load mmc 0:1 0x41800000 sun8i-v3s-bingpi-uboot.dtb" >> boot.cmd
    #echo "bootz 0x41000000 - 0x41800000" >> boot.cmd

    #sudo cp $TOP_DIR/tools/mkimage /usr/local/bin/mkimage
    #echo "sudo cp ./tools/mkimage /usr/local/bin/mkimage"
    cp $TOP_DIR/board/BingPi/script/bootcmd $TOP_DIR/u-boot/u-boot-v3s-current/
    echo "cp bootcmd to u-boot workspace"

    $TOP_DIR/u-boot/u-boot-v3s-current/tools/mkimage -C none -A arm -T script -d bootcmd boot.scr
    
    #ehco "$TOP_DIR/tools/mkimage -C none -A arm -T script -d bootcmd boot.scr"

    echo "Bootcmd build OK!"
}

build_uboot() {
    echo "Start Compile U-Boot!"

    cd $TOP_DIR/u-boot/u-boot-v3s-current
    echo "cd $TOP_DIR/u-boot/u-boot-v3s-current"

    make clean
    echo "make clean"

    cp $TOP_DIR/board/BingPi/dts/uboot/sun8i-v3s-bingpi-uboot.dts $TOP_DIR/u-boot/u-boot-v3s-current/arch/arm/dts/
    echo "cp u-boot dts"

    cp $TOP_DIR/board/BingPi/config/uboot/uboot_bingpi_800x480LCD_defconfig $TOP_DIR/u-boot/u-boot-v3s-current/configs/
    echo "cp u-boot defconfig"
    
    make uboot_bingpi_800x480LCD_defconfig
    echo "make uboot_bingpi_800x480LCD_defconfig"

    time make 2>&1 | tee build.log 
    echo "time make 2>&1 | tee build.log"

    build_bootcmd

    echo "U-Boot Compiled!"
}

build_kernel() {
    echo "Start Compile Linux Kernel!"

    cd $TOP_DIR/linux/linux-5.2.y
    echo "cd $TOP_DIR/linux/linux-5.2.y"

    make clean
    echo "make clean"

    cp $TOP_DIR/board/BingPi/dts/linux/sun8i-v3s-bingpi-linux.dts $TOP_DIR/linux/linux-5.2.y/arch/arm/boot/dts/
    echo "cp linux dts"

    cp $TOP_DIR/board/BingPi/config/linux/linux_bingpi_v3s_defconfig $TOP_DIR/linux/linux-5.2.y/arch/arm/configs/
    echo "cp linux defconfig"

    make linux_bingpi_v3s_defconfig 
    echo "make linux_bingpi_v3s_defconfig"

    time make -j16
    echo "time make -j16"

    make dtbs
    echo "make dtbs"

    echo "Linux Kernel Compiled!"
}

build_rootfs() {
    echo "Start Compile rootfs!"
    cd $TOP_DIR/buildroot 
    echo "cd $TOP_DIR/buildroot"

    #sudo make clean
    #echo "make clean"

    cp $TOP_DIR/board/BingPi/buildroot/bingpi-v3s-defconfig $TOP_DIR/buildroot/.config
    echo "cp $TOP_DIR/board/BingPi/buildroot/bingpi-v3s-defconfig $TOP_DIR/buildroot/.config"

    time sudo make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE -j16
    echo "time sudo make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE  -j16"

    echo "Buildroot rootfs Compiled!"
}


copy_bin() {
    echo "Copy bin file..."

    cp $TOP_DIR/linux/linux-5.2.y/arch/arm/boot/zImage $TOP_DIR/output/
    echo "cp $TOP_DIR/linux/linux-5.2.y/arch/arm/boot/zImage $TOP_DIR/output/"

    cp $TOP_DIR/linux/linux-5.2.y/arch/arm/boot/dts/sun8i-v3s-licheepi-zero-dock.dtb $TOP_DIR/output/
    echo "cp $TOP_DIR/linux/linux-5.2.y/arch/arm/boot/dts/sun8i-v3s-licheepi-zero-dock.dtb $TOP_DIR/output/"

    cp $TOP_DIR/u-boot/u-boot-v3s-current/u-boot-sunxi-with-spl.bin $TOP_DIR/output/
    echo "cp $TOP_DIR/u-boot/u-boot-v3s-current/u-boot-sunxi-with-spl.bin $TOP_DIR/output/"

    cp $TOP_DIR/u-boot/u-boot-v3s-current/boot.scr $TOP_DIR/output/
    echo "cp $TOP_DIR/u-boot/u-boot-v3s-current/boot.scr $TOP_DIR/output/"

    cp $TOP_DIR/buildroot/output/images/rootfs.tar $TOP_DIR/output/
    echo "cp $TOP_DIR/buildroot/output/images/rootfs.tar $TOP_DIR/output/"
}

build_image() {
   echo "build sd image"

   copy_bin

   cp $TOP_DIR/board/BingPi/script/pack.sh $TOP_DIR/output

   cd $TOP_DIR/output

   rm sdcard.img

   ./pack.sh
   
}

clean_all() {
  echo "clean all"

  cd $TOP_DIR/u-boot/u-boot-v3s-current
  make clean

  cd $TOP_DIR/linux/linux-5.2.y
  make clean

  cd $TOP_DIR/buildroot
  make clean

  cd $TOP_DIR/output
  rm *.tar
  rm *.img
}


if [ $1 == "uboot" ]; then 
    build_uboot
elif [ $1 == "kernel" ]; then
    build_kernel
elif [ $1 == "rootfs" ]; then
    build_rootfs
elif [ $1 == "image" ]; then
    build_image
elif [ $1 == "clean" ]; then
    clean_all
elif [ $1 == "all" ]; then
    build_uboot
    cd ../
    build_kernel
    cd ../
    build_rootfs
    cd ../
    build_image
fi


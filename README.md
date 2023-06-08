版本：buildroot: buildroot-2020.02.4
linux: linux-zero-5.2.y
uboot: u-boot-3s-current

# 编译命令
- 编译uboot：./build.sh uboot
- 编译kernel：./build.sh kernel
- 编译rootfs：./build.sh rootfs
- 打包命令：./build.sh image
- 编译所有：./build.sh all
- 清理所有：./build.sh clean

# 输出文件
输出img文件在：./output/sdcard.img


# 需要注意的地方
- Linux的设备树的名称
- uboot的设备树的名称
- uboot的配置文件里面有uboot指定的设备树名字
- Buildroot里面指定的交叉编译器的路径需要是完整的名称
 - 在buildroot的xxxx_defconfig里面有指定的交叉编译器的路径，不能是相对路径（目前没有办法解决，只能写死）
 
 
# 打包注意的地方
- 文件名字

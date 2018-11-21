#!/bin/bash
ARCH=arm
KERNEL=kernel7
CROSS_COMPILE=arm-linux-gnueabihf-
cd $1
make ARCH=$ARCH KERNEL=$KERNEL CROSS_COMPILE=$CROSS_COMPILE
bcm2709_defconfig


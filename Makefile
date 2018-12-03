# Build all drivers in this folder
obj-m += timer.o
obj-m += pdriver.o
obj-m += barometerDriver.o
CROSS_COMPILE=~/Projects/rpi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-
SOURCE=~/Projects/rpi/linux

host:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

rpi: 
	make \
	-C $(SOURCE) \
	ARCH=arm \
	CROSS_COMPILE=$(CROSS_COMPILE) \
	M=$(PWD) \
	modules

clean: 
	$(RM) *.ko
	$(RM) *.o




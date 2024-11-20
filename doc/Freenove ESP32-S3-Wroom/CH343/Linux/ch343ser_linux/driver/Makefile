ifeq ($(KERNELRELEASE), )
KERNELDIR := /lib/modules/$(shell uname -r)/build
PWD :=$(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR)  M=$(PWD)  
clean:
	rm -rf *.mk .tmp_versions Module.symvers *.mod.c *.o *.ko .*.cmd Module.markers modules.order *.a *.mod
load:
	insmod ch343.ko
unload:
	rmmod ch343
install: default
	mkdir -p /lib/modules/$(shell uname -r)/kernel/drivers/usb/serial
	cp -f ./ch343.ko /lib/modules/$(shell uname -r)/kernel/drivers/usb/serial
	echo "ch343\n" >> /etc/modules
	depmod -a
uninstall:
	rm -rf /lib/modules/$(shell uname -r)/kernel/drivers/usb/serial/ch343.ko
	depmod -a
else
	obj-m := ch343.o
endif

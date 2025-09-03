obj-m += espcam.o

espcam-objs := espcam_main.o espcam_sysfs.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

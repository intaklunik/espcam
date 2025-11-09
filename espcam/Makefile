obj-m += espcam.o

espcam-objs := espcam_main.o espcam_sysfs.o espcam_spi.o /video/espcam_video.o /video/espcam_ctrl.o /video/espcam_vb.o /video/espcam_ioctl.o

ccflags-y += -I$(src)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

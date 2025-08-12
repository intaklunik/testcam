obj-m += testcam.o

testcam-objs := testcam_main.o testcam_ioctl.o testcam_vb.o testcam_draw.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

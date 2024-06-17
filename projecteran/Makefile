obj-m := proj.o
ROOTDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	make -C $(ROOTDIR) M=$(PWD) modules
	
clean:
	make -C $(ROOTDIR) M=$(PWD) clean

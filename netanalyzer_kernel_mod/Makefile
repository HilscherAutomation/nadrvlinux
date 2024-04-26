ifeq ($(KDIR),)
	KDIR := /lib/modules/$(shell uname -r)/build
endif

ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m     := netanalyzer.o
ccflags-y += -I$(PWD)/toolkit/Include/ -I$(PWD)/toolkit/ -I$(PWD)/

netanalyzer-objs := netana_enum.o netana.o netana_sysfs.o netana_tk_interface.o \
	OS_Linux.o USER_Linux.o toolkit/netana_functions.o toolkit/netana_toolkit.o \
	toolkit/netana_interrupt.o toolkit/netana_capture.o

else
# normal makefile
all:    modules

clean-files := *.order *.ko *.o

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install

clean:
	rm -f *.o *.ko *.order *.mod* Module.symvers

endif

# Comment/uncomment the following line to disable/enable debugging
DEBUG=y
# Add your debugging flag (or not) to CFLAGS
ifeq ($(DEBUG),y)
  DEBFLAGS = -O -g -DDEBUG # "-O" is needed to expand inlines
else
  DEBFLAGS = -Wall
endif

CFLAGS += -I$(LDDINC) ${DEBFLAGS} ${INCLUDES} ./

ifneq ($(KERNELRELEASE),)
# call from kernel build system
obj-m := my-enc28j60.o
my-enc28j60-objs := enc28j60_main.o spi_cmds.o
else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)


.PHONY: default
default: all ;

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install
	depmod -A

endif

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c *.order *.mod *.symvers .tmp_versions

depend .depend dep:
	$(CC) $(CFLAGS) -M *.c > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif

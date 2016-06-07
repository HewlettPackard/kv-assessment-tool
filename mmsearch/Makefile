obj-m := mmsearch.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rm *.ko *.mod.* *.o *.order *.symvers



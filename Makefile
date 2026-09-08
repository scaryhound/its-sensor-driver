obj-m := its_sensor.o

# If KERNELDIR is not defined (like when building natively on your host), point it to the local kernel headers.
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

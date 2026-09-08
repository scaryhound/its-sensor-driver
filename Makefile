# Cross-compilation variables (Yocto will override these)
CC ?= gcc
CFLAGS ?= -Wall -Wextra -g

# Kernel Module variables
obj-m := its_sensor.o
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# Targets
APP_NAME := its_daemon
SRC := its_daemon.c

.PHONY: all clean

all: $(APP_NAME) module

# Compile the user-space daemon
$(APP_NAME): $(SRC)
	$(CC) $(CFLAGS) -o $(APP_NAME) $(SRC)

# Compile the kernel module
module:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -f $(APP_NAME)
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2018 IBM Corporation
#

KDIR ?= /lib/modules/$(shell uname -r)/build

default: modules

modules clean:
	$(MAKE) -C $(KDIR) M=$(PWD) $@

modules_install:
	$(MAKE) -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(DESTDIR) $@

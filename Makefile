#
# $Id: Makefile 1235 2014-02-21 16:44:10Z ales.bardorfer $
#
# Red Pitaya specific application Makefile.
#

APP=$(notdir $(CURDIR:%/=%))
FPGA=fpga.bit

# Versioning system
BUILD_NUMBER ?= 0
REVISION ?= devbuild
VER:=$(shell cat info/info.json | grep version | sed -e 's/.*:\ *\"//' | sed -e 's/-.*//')

INSTALL_DIR ?= ../

CONTROLLER=controller.so
ARTIFACTS=$(CONTROLLER)

CFLAGS += -DVERSION=$(VER)-$(BUILD_NUMBER) -DREVISION=$(REVISION)
export CFLAGS

all: $(CONTROLLER) $(CLI)

$(CLI):
	$(MAKE) -C src

$(CONTROLLER):
	$(MAKE) -C ./Application/remote_acquire_utility/src

clean:
	$(MAKE) -C ./Application/remote_acquire_utility/src clean
	$(MAKE) -C ./src clean

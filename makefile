ROOTDIR = $(CURDIR)
SOURCEDIR = $(ROOTDIR)/Source
DEBUGDIR = $(ROOTDIR)/Debug

# set compiler defaults for OSX versus *nix
OS := $(shell uname)
ifeq ($(OS), Darwin)
ifndef CC
export CC = $(if $(shell which clang), clang, gcc)
endif
else
# linux defaults
ifndef CC
export CC = gcc
endif
endif


export CFLAGS = -Wall -Wno-unknown-pragmas -g
CFLAGS += -I$(SOURCEDIR) -IBuild

export DFLAGS = -DDEBUG=1

MAKE := $(shell which make 2> /dev/null)

subsystem_debug: subsystem_source
	$(MAKE) -C $(DEBUGDIR)

subsystem_source:
	$(MAKE) -C $(SOURCEDIR)
	@echo "HHH"

clean:
	$(MAKE) clean -C $(SOURCEDIR)
	$(MAKE) clean -C $(DEBUGDIR)




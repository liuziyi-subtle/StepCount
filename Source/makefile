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

export CFLAGS = -Wall -g3 -pedantic -fPIC
export DFLAGS = -DDEBUG=1
export LDLIBS = -lm

OBJS := $(patsubst %.c, %.o, $(wildcard *.c))

libstep.a: $(OBJS)
	ar -r libstep.a $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(DFLAGS)

clean:
	rm -f $(CURDIR)/*.o $(CURDIR)/*.a

SOURCEDIR = ../Source

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

export CFLAGS = $(shell python3-config --cflags)
CFLAGS += -I$(SOURCEDIR)

export LDFLAGS = $(shell python3-config --ldflags)
LDFLAGS += -lpython3.9
LDFLAGS += -L$(SOURCEDIR)

export DFLAGS = -DDEBUG=1

ETOBJS = c2py_wrap.o debug.o

_c2py.so: $(ETOBJS)
	$(CC) -bundle `python3-config --ldflags` -lpython3.9 -lm $(ETOBJS) -L$(SOURCEDIR) -lstep -o _c2py.so
	#ld -shared $(PY_LIB_DIR) $(PY_LIB) $(ETOBJS) -lm -L/Users/liuziyi/Documents/GitHub/StepCount/Source -lstep -lpython -o steppy.so

c2py_wrap.o: c2py_wrap.c
	$(CC) -c $(CFLAGS) c2py_wrap.c

c2py_wrap.c: c2py.i
	swig -python -py3 c2py.i

debug.o: debug.c
	$(CC) -c $(CFLAGS) debug.c

clean:
	rm -f $(CURDIR)/*_wrap.c $(CURDIR)/c2py.py $(CURDIR)/*.so $(CURDIR)/*.o
	rm -rf __pycache__

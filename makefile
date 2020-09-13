CC = $(Compiler) -g
CFLAGS = -Wall -Wextra

# 0 - 单条数据  1 - 多个文件
DFLAGS = -DALGO_DEBUG=1

DIR_DEBUG = debug

DIR_RESULTS = results

ETOBJS = main.o nonwalk_check.o nonwalk_model.o debug.o

all:	$(DIR_RESULTS) executable

$(DIR_RESULTS):
	mkdir $@

executable: $(ETOBJS)
	$(CC) $(CFLAGS) -o $@ $(ETOBJS) -Wall -lm

main.o:
	$(CC) $(CFLAGS) -c $(DIR_DEBUG)/main.c -o $@ -I$(DIR_DEBUG) $(DFLAGS)

nonwalk_check.o:
	$(CC) $(CFLAGS) -c nonwalk_check.c $(DFLAGS)

nonwalk_model.o:
	$(CC) $(CFLAGS) -c nonwalk_model.c $(DFLAGS)

debug.o:
	$(CC) $(CFLAGS) -c $(DIR_DEBUG)/debug.c -o $@ -I$(DIR_DEBUG) $(DFLAGS)

clean:
	rm -rf *.o *.out *~ executable $(DIR_RESULTS)

#IDIR =../include
CC=gcc
#CFLAGS=-I$(IDIR)
src = $(wildcard *.c)
obj = $(src:.c=.o)

LDFLAGS = 

FileProcess: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) myprog

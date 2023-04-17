CC=gcc
AR=ar
LIB=libgmalloc.a
LIBFLAGS=-rcs
OFLAGS=-c -fPIC -Wall
CFLAGS=-Wall

mergesort: mergesort.c $(LIB)
	$(CC) $(CFLAGS) $< -o $@ -L. -lgmalloc -I.

$(LIB): goodmalloc.o
	$(AR) $(LIBFLAGS) $(LIB) $<

goodmalloc.o: goodmalloc.c goodmalloc.h
	$(CC) $(OFLAGS) $^

clean:
	rm *.o *.a *.gch

.PHONY: clean

mergesort.c:

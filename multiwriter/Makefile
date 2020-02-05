CC=gcc
CFLAGS=-Wall -Werror -I -lrt.
DEPS = Multiwriter.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

massivereader: main.o Multiwriter.o
	$(CC) -o multiwriter main.o Multiwriter.o -lrt

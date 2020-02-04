CC=gcc
CFLAGS=-Wall -Werror -I.
DEPS = Massivereader.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

massivereader: main.o Massivereader.o
	$(CC) -o massivereader main.o Massivereader.o

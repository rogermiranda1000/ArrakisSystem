CC = gcc
CFLAGS = -Wall -Wextra -g

fremen: fremen.c fremen.h RegExSearcher.o
	$(CC) fremen.c RegExSearcher.o -o fremen $(CFLAGS)

regex: RegExSearcher.c RegExSearcher.h
	$(CC) RegExSearcher.c -o RegExSearcher.o $(CFLAGS)

clean:
	rm -f fremen *.o

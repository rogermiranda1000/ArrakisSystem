CC = gcc
# -std=gnu1 és necesari per WCONTINUED [https://stackoverflow.com/a/60436595/9178470]
CFLAGS = -Wall -Wextra -g -std=gnu11

fremen: atreides.c atreides.h Users.o
	$(CC) atreides.c Users.o Atreides $(CFLAGS)

config: Users.c Users.h
	$(CC) Users.c -o Users.o $(CFLAGS)

clean:
	rm -f Atreides *.o

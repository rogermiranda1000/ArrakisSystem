CC = gcc
# -std=gnu1 és necesari per WCONTINUED [https://stackoverflow.com/a/60436595/9178470]
CFLAGS = -Wall -Wextra -g -std=gnu11

fremen: atreides.c atreides.h
	$(CC) atreides.c Atreides $(CFLAGS)

clean:
	rm -f fremen.exe *.o

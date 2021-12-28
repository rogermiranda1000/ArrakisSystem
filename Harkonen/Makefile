CC = gcc
# -std=gnu1 és necesari per WCONTINUED [https://stackoverflow.com/a/60436595/9178470]
CFLAGS = -Wall -Wextra -g -std=gnu11

harkonen: harkonen.c harkonen.h ProgramLauncher.o ConfigReader.o
	$(CC) harkonen.c ProgramLauncher.o ConfigReader.o -o Harkonen $(CFLAGS)

program: ProgramLauncher.c ProgramLauncher.h
	$(CC) ProgramLauncher.c -o ProgramLauncher.o $(CFLAGS)

config: ConfigReader.c ConfigReader.h
	$(CC) ConfigReader.c -o ConfigReader.o $(CFLAGS)

clean:
	rm -f Harkonen *.o

CC = gcc
# -std=gnu1 és necesari per WCONTINUED [https://stackoverflow.com/a/60436595/9178470]
CFLAGS = -Wall -Wextra -g -std=gnu11 -lpthread

atreides: atreides.c atreides.h Users.o ConfigReader.o RegExSearcher.o Threads.o Comunication.o ProgramLauncher.o
	$(CC) atreides.c Users.o ConfigReader.o RegExSearcher.o Threads.o Comunication.o ProgramLauncher.o -o Atreides $(CFLAGS)

users: Users.c Users.h
	$(CC) Users.c -o Users.o $(CFLAGS)

config: ConfigReader.c ConfigReader.h
	$(CC) ConfigReader.c -o ConfigReader.o $(CFLAGS)

regex: RegExSearcher.c RegExSearcher.h
	$(CC) RegExSearcher.c -o RegExSearcher.o $(CFLAGS)

threads: Threads.c Threads.h
	$(CC) Threads.c -o Threads.o $(CFLAGS)

comunication: Comunication.c Comunication.h
	$(CC) Comunication.c -o Comunication.o $(CFLAGS)

program: ProgramLauncher.c ProgramLauncher.h
	$(CC) ProgramLauncher.c -o ProgramLauncher.o $(CFLAGS)

clean:
	rm -f Atreides *.o

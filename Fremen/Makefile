CC = gcc
# -std=gnu1 és necesari per WCONTINUED [https://stackoverflow.com/a/60436595/9178470]
CFLAGS = -Wall -Wextra -g -std=gnu11

fremen: fremen.c fremen.h RegExSearcher.o ProgramLauncher.o ConfigReader.o
	$(CC) fremen.c RegExSearcher.o ProgramLauncher.o ConfigReader.o -o fremen $(CFLAGS)
	
config: ConfigReader.c ConfigReader.h RegExSearcher.o ProgramLauncher.o
	$(CC) ConfigReader.c -o ConfigReader.o $(CFLAGS)

regex: RegExSearcher.c RegExSearcher.h
	$(CC) RegExSearcher.c -o RegExSearcher.o $(CFLAGS)

program: ProgramLauncher.c ProgramLauncher.h
	$(CC) ProgramLauncher.c -o ProgramLauncher.o $(CFLAGS)

clean:
	rm -f fremen *.o

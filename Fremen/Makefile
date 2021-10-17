CC = gcc
# -std=gnu1 és necesari per WCONTINUED [https://stackoverflow.com/a/60436595/9178470]
CFLAGS = -Wall -Wextra -g -std=gnu11

fremen: fremen.c fremen.h RegExSearcher.o ProgramLauncher.o ConfigReader.o Commands.o
	$(CC) fremen.c ConfigReader.o RegExSearcher.o ProgramLauncher.o Commands.o -o fremen.exe $(CFLAGS)
	
config: ConfigReader.c ConfigReader.h
	$(CC) ConfigReader.c -o ConfigReader.o $(CFLAGS)

regex: RegExSearcher.c RegExSearcher.h
	$(CC) RegExSearcher.c -o RegExSearcher.o $(CFLAGS)

program: ProgramLauncher.c ProgramLauncher.h
	$(CC) ProgramLauncher.c -o ProgramLauncher.o $(CFLAGS)

commands: Commands.c Commands.h
	$(CC) Commands.c -o Commands.o $(CFLAGS)

clean:
	rm -f fremen.exe *.o

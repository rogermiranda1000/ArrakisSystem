#include "FileCleaner.h"

char *clean_directory;
unsigned int clean_delay;
char **execute_cmd_envp;
void (*execute_cmd_freeMallocs)();

void cleanInterrupt() {
	write(1, CLEANING_FILES, sizeof(CLEANING_FILES)/sizeof(char));
	
	// neteja els fitxers
	char *cmd;
	concat(&cmd, "rm -f %s/*", clean_directory);
	executeProgramLine(&cmd, execute_cmd_envp, execute_cmd_freeMallocs);
	
	// reprograma
	signal(SIGALRM, cleanInterrupt);
	alarm(clean_delay);
}

void startCleaner(char *directory, unsigned int delay, char *envp[], void (*freeMallocs)()) {
	clean_directory = directory;
	clean_delay = delay*60;
	
	execute_cmd_envp = envp;
	execute_cmd_freeMallocs = freeMallocs;
	
	signal(SIGALRM, cleanInterrupt);
	alarm(clean_delay);
}
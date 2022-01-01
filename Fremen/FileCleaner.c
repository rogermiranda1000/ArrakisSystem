#include "FileCleaner.h"

char *clean_directory;
unsigned int clean_delay;
char **execute_cmd_envp;
void (*execute_cmd_freeMallocs)();

ForkedPipeInfo remove_files_fork_pipe;
char *remove_file;
void freeMallocsPlusPipe() {
	execute_cmd_freeMallocs();
	free(remove_file);
	freeForkedPipeInfo(&remove_files_fork_pipe);
}

/**
 * Neteja els fitxers
 */
void cleanInterrupt() {
	write(1, CLEANING_FILES, sizeof(CLEANING_FILES)/sizeof(char));
	
	// obtè els fitxers a eliminar
	char *cmd;
	concat(&cmd, "find %s -name *.jpg -o -name *.JPG", clean_directory);
	executeProgramLineWithPipe(&remove_files_fork_pipe, &cmd, execute_cmd_envp, execute_cmd_freeMallocs);
	
	readUntil(fdPipeInfo(remove_files_fork_pipe, 0), &remove_file, '\n');
	while (*remove_file) {
		concat(&cmd, "rm %s", remove_file);
		executeProgramLine(&cmd, execute_cmd_envp, freeMallocsPlusPipe);
		
		free(remove_file);
		readUntil(fdPipeInfo(remove_files_fork_pipe, 0), &remove_file, '\n');
	}
	free(remove_file);
	freeForkedPipeInfo(&remove_files_fork_pipe);
	
	// reprograma
	signal(SIGALRM, cleanInterrupt);
	alarm(clean_delay);
}

void startCleaner(char *directory, unsigned int delay, char *envp[], void (*freeMallocs)()) {
	clean_directory = directory;
	clean_delay = delay * 60;
	
	execute_cmd_envp = envp;
	execute_cmd_freeMallocs = freeMallocs;
	
	signal(SIGALRM, cleanInterrupt);
	alarm(clean_delay);
}
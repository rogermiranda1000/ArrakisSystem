#include "harkonen.h"

/**
 * Donat un format i els paràmetres (de la mateixa forma que es pasen a sprintf), retorna la string
 * /!\ Cal fer el free del buffer /!\
 * @param buffer	On es guardarà el resultat (char**)
 * @param format	Format (com a sprintf)
 * @param ...		Paràmetres del format (com a sprintf)
 * @return			Mida
 */
#define concat(buffer, format, ...) ({													\
	size_t size = snprintf(NULL, 0, format, __VA_ARGS__); /* obtè la mida resultant */	\
	*buffer = (char*)malloc(size+1);													\
	sprintf(*buffer, format, __VA_ARGS__); /* retorna la mida */						\
})

char *command = NULL;

void ctrlCHandler() {
    free(command);
}

int main(int argc, char *argv[], char *envp[]) {

    signal(SIGINT, ctrlCHandler);

    if (argc != 2) {
        write(DESCRIPTOR_ERROR, ERROR_ARGS, STATIC_STRING_LEN(ERROR_ARGS));
		exit(EXIT_FAILURE);
    }

    write(DESCRIPTOR_SCREEN, MSG_INIT, STATIC_STRING_LEN(MSG_INIT));

    int seconds = atoi(argv[1]);

    ForkedPipeInfo fork_pipe;
    char *command1 = (char *)malloc(sizeof(char) * (STATIC_STRING_LEN(CMD_WHO) + 1));
    strcpy(command1, CMD_WHO);
    executeProgramLineWithPipe(&fork_pipe, &command1, envp, ctrlCHandler);
    char* who;
    readUntil(fdPipeInfo(fork_pipe, 0), &who, '\n');
	freeForkedPipeInfo(&fork_pipe);
    
    concat(&command, "ps -u %s --no-headers", who);
    free(who);

    while (true) {
        char *command2 = (char *)malloc(sizeof(char) * (strlen(command) + 1));
        strcpy(command2, command);
        executeProgramLineWithPipe(&fork_pipe, &command2, envp, ctrlCHandler);
        // Coses
        freeForkedPipeInfo(&fork_pipe);
        sleep(seconds);
    }
}
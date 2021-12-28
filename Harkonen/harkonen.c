#include "harkonen.h"

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
    command1 = (char*)malloc(sizeof(char) * (STATIC_STRING_LEN(CMD_WHO) + 1));
    strcpy(command1, CMD_WHO);
    executeProgramLineWithPipe(&fork_pipe, &command1, envp, freeMallocs);
    readUntil(fdPipeInfo(fork_pipe, 0), &md5sum, ' '); // md5sum retorna '<md5> *<nom fitxer>'
	freeForkedPipeInfo(&fork_pipe);
    free(command1);


    while (true) {
        executeProgramLineWithPipe(&fork_pipe, &command, envp, freeMallocs);
        sleep(seconds);
    }
}
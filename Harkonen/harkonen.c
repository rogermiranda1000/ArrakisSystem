#include "harkonen.h"

int main(int argc, char *argv[], char *envp[]) {
    if (argc != 2) {
        write(DESCRIPTOR_ERROR, ERROR_ARGS, STATIC_STRING_LEN(ERROR_ARGS));
		exit(EXIT_FAILURE);
    }

    write(DESCRIPTOR_SCREEN, MSG_INIT, STATIC_STRING_LEN(MSG_INIT));

    int seconds = atoi(argv[1]);

    while (true) {
        
        sleep(seconds); 
    }
}
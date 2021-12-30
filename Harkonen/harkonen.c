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

/**
 * Donat un format i els paràmetres (de la mateixa forma que es pasen a sprintf), imprimeix la string
 * @param fd		FileDescriptor on imprimir la string
 * @param format	Format (com a sprintf)
 * @param ...		Paràmetres del format (com a sprintf)
 */
#define susPrintF(fd, format, ...) ({							\
	char *buffer;												\
	write(fd, buffer, concat(&buffer, format, __VA_ARGS__));	\
	free(buffer);												\
})

/**
 * Llegeix del FileDescriptor fins trobar el delimitador i decideix si continuar o no
 * Útil per utilitzar junt amb '\n' com a stop_delimiter
 * /!\ El delimitador no es guarda al buffer, però sí s'elimina del FileDescriptor /!\
 * /!\ Cal fer el free del buffer /!\
 * @param fd					FileDescriptor on agafar les dades
 * @param buffer				Array on guardar la informació (amb '\0')
 * @param size					Caracters obtninguts (sense contar '\0'; NULL si no es vol guardar)
 * @param continue_delimiter	Caracter que marca quan parar la lectura
 * @param stop_delimiter		Caracter que marca quan parar la lectura
 * @retval true					S'ha detingut a continue_delimiter
 * @retval false				S'ha detingut a stop_delimiter
 */
bool readUntilAndContinue(int fd, char **buffer, size_t *size, char continue_delimiter, char stop_delimiter) {
	char aux;
	size_t s = 0;
	*buffer = NULL;
	
	while(read(fd, &aux, sizeof(char)) == sizeof(char) && aux != continue_delimiter && aux != stop_delimiter) {
		*buffer = (char*)realloc(*buffer, (s+1)*sizeof(char));
		(*buffer)[s++] = aux;
	}
	*buffer = (char*)realloc(*buffer, (s+1)*sizeof(char));
	(*buffer)[s] = '\0';
	
	if (size != NULL) *size = s;
	
	return (aux == continue_delimiter);
}

/**
 * Llegeix del FileDescriptor fins trobar el delimitador
 * /!\ El delimitador no es guarda al buffer, però sí s'elimina del FileDescriptor /!\
 * /!\ Cal fer el free del buffer /!\
 * @param fd		FileDescriptor on agafar les dades
 * @param buffer	Array on guardar la informació (amb '\0')
 * @param delimiter	Caracter que marca quan parar
 * @return			Caracters obtninguts (sense contar '\0')
 */
size_t readUntil(int fd, char **buffer, char delimiter) {
	size_t size;
	
	readUntilAndContinue(fd, buffer, &size, delimiter, delimiter);
	
	return size;
}

char *command = NULL;

void ctrlCHandler() {
    free(command);
}

void quit() {
	write(DESCRIPTOR_SCREEN, MSG_EXIT, STATIC_STRING_LEN(MSG_EXIT));
	ctrlCHandler();
}

/**
 * Donat un FD amb PIDs retorna un aleatori
 * @param fd	File Descriptor on llegir els PIDs
 * @return		PID aleatori; -1 si el FD no conté cap
 */
int getRandomPID(int fd) {
	int *pids = NULL;
	size_t pid_size = 0;
	
	// el primer, per alguna raó, és el PID 1; aixó passa si posem '--format=%P'
	char *info;
	readUntil(fd, &info, '\n');
	free(info);
	
	readUntil(fd, &info, '\n');
	while (*info) {
		pids = (int*)realloc(pids, sizeof(int)*(++pid_size));
		pids[pid_size-1] = atoi(info);
		
		free(info);
		readUntil(fd, &info, '\n');
	}
	free(info);
	
	if (pid_size == 0) return -1;
	return pids[rand() % pid_size];
}

int main(int argc, char *argv[], char *envp[]) {
    signal(SIGINT, quit);

    if (argc != 2) {
        write(DESCRIPTOR_ERROR, ERROR_ARGS, STATIC_STRING_LEN(ERROR_ARGS));
		exit(EXIT_FAILURE);
    }

    write(DESCRIPTOR_SCREEN, MSG_INIT, STATIC_STRING_LEN(MSG_INIT));
	
	srand(time(NULL)); // pel random

    int seconds = atoi(argv[1]);

    ForkedPipeInfo fork_pipe;
    char *command1 = (char *)malloc(sizeof(char) * (STATIC_STRING_LEN(CMD_WHO) + 1));
    strcpy(command1, CMD_WHO);
    executeProgramLineWithPipe(&fork_pipe, &command1, envp, ctrlCHandler);
    char* who;
    readUntil(fdPipeInfo(fork_pipe, 0), &who, '\n');
	freeForkedPipeInfo(&fork_pipe);
    
    concat(&command, "ps -u %s --no-headers --format=%%P", who);
    free(who);

	char *command2;
	int random_pid;
    while (true) {
		write(DESCRIPTOR_SCREEN, MSG_SEARCH, STATIC_STRING_LEN(MSG_SEARCH));
		
        command2 = (char *)malloc(sizeof(char) * (strlen(command) + 1));
        strcpy(command2, command);
        executeProgramLineWithPipe(&fork_pipe, &command2, envp, ctrlCHandler);
        random_pid = getRandomPID(fdPipeInfo(fork_pipe, 0));
        freeForkedPipeInfo(&fork_pipe);
		
		if (random_pid != -1) {
			susPrintF(DESCRIPTOR_SCREEN, "killing pid %d\n", random_pid);
			
			concat(&command2, "kill -9 %d", random_pid);
			executeProgramLine(&command2, envp, ctrlCHandler);
		}
		
        sleep(seconds);
    }
	
	return 0;
}
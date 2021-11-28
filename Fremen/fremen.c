#include "fremen.h"

volatile Status current_status = RUNNING;

char *ip, *directory;
char *input = NULL;
int clientFD = -1;

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

void ctrlCHandler() {
	if (current_status == WAITING) {
		terminate();
		
		signal(SIGINT, SIG_DFL); // deprograma (tot i que hauria de ser així per defecte, per alguna raó no funciona)
		raise(SIGINT);
	}
	else {
		current_status = EXIT;
		
		signal(SIGINT, ctrlCHandler); // no hauria de caldre, però per si és un impacient
	}
}

int main(int argc, char *argv[], char *envp[]) {
	unsigned int timeClean;
	unsigned short port;

	signal(SIGINT, ctrlCHandler); // reprograma Control+C
	
	if (argc < 2) {
		write(DESCRIPTOR_ERROR, ERROR_ARGS, STATIC_STRING_LEN(ERROR_ARGS));
		exit(EXIT_FAILURE);
	}
	
	if (readConfig(argv[1], &timeClean, &ip, &port, &directory) == -1) {
		write(DESCRIPTOR_ERROR, ERROR_FILE, STATIC_STRING_LEN(ERROR_FILE));
		exit(EXIT_FAILURE);
	}
	
	write(DESCRIPTOR_SCREEN, MSG_INIT, STATIC_STRING_LEN(MSG_INIT));
	
	char *read;
	char **output;
	initCommands();
	
	int clientID = -1;
	while (current_status != EXIT) {
		free(input); // allibera l'últim readUntil
		input = NULL;
		
		current_status = WAITING;
		readUntil(0, &input, '\n');
		current_status = RUNNING;
		
		switch(searchCommand(input, &output)) {
			/**
			 * -- Fremen -> Atreides --
			 * C|<nom>*<codi>\n -> login <nom> <codi>
			 * s|<codi>\n -> search <codi>
			 * [x] n|<file>\n -> send <file>
			 * [x] p|<id>\n -> photo <id>
			 * Q|\n -> logout
			 *
			 * -- Atreides -> Fremen --
			 * O|<id>\n -> login efectuat correctament
			 **/
			
			case LOGIN:
				if (clientID >= 0) {
					freeCommand(LOGIN, &output);
					write(DESCRIPTOR_ERROR, ERROR_ALREADY_LOGGED, STATIC_STRING_LEN(ERROR_ALREADY_LOGGED));
					break;
				}
				
				clientFD = socketConnect(ip, port);
				if (clientFD < 0) {
					freeCommand(LOGIN, &output);
					write(DESCRIPTOR_ERROR, ERROR_SOCKET, STATIC_STRING_LEN(ERROR_SOCKET));
					break;
				}
				
				sendLogin(clientFD, output[0], output[1]);
				
				readUntil(clientFD, &read, '|');
				if (*read != 'O') {
					freeCommand(LOGIN, &output);
					write(DESCRIPTOR_ERROR, ERROR_COMUNICATION, STATIC_STRING_LEN(ERROR_COMUNICATION));
					free(read);
					break;
				}
				free(read);
				
				clientID = readInteger(clientFD, NULL);
				susPrintF(DESCRIPTOR_SCREEN, "Benvingut %s. Tens ID %d.\n", output[0], clientID);
				
				write(DESCRIPTOR_SCREEN, MSG_CONNECTED, STATIC_STRING_LEN(MSG_CONNECTED));
				
				freeCommand(LOGIN, &output);
				break;
				
			case SEARCH:
				if (clientID < 0) {
					write(DESCRIPTOR_ERROR, ERROR_NO_CONNECTION, STATIC_STRING_LEN(ERROR_NO_CONNECTION));
					freeCommand(SEARCH, &output);
					break;
				}
				
				write(DESCRIPTOR_SCREEN, output[0], strlen(output[0])); // code
				write(DESCRIPTOR_SCREEN, "\n", sizeof(char));
				
				freeCommand(SEARCH, &output);
				break;
				
			case PHOTO:
				if (clientID < 0) {
					write(DESCRIPTOR_ERROR, ERROR_NO_CONNECTION, STATIC_STRING_LEN(ERROR_NO_CONNECTION));
					freeCommand(PHOTO, &output);
					break;
				}
				
				write(DESCRIPTOR_SCREEN, output[0], strlen(output[0])); // id
				write(DESCRIPTOR_SCREEN, "\n", sizeof(char));
				
				freeCommand(PHOTO, &output);
				break;
				
			case SEND:
				if (clientID < 0) {
					write(DESCRIPTOR_ERROR, ERROR_NO_CONNECTION, STATIC_STRING_LEN(ERROR_NO_CONNECTION));
					freeCommand(SEND, &output);
					break;
				}
				
				write(DESCRIPTOR_SCREEN, output[0], strlen(output[0])); // file
				write(DESCRIPTOR_SCREEN, "\n", sizeof(char));
				
				freeCommand(SEND, &output);
				break;
				
			case LOGOUT:
				current_status = EXIT;
				// el socket es tanca a terminate()
				
				// logout no té arguments -> no cal free
				break;
			
			case NO_MATCH:
				if (executeProgramLine(input, envp) != 0) write(DESCRIPTOR_ERROR, ERROR_EXECUTE, STATIC_STRING_LEN(ERROR_EXECUTE));
				break;
				
			case ERROR:
				write(DESCRIPTOR_ERROR, ERROR_MALLOC, STATIC_STRING_LEN(ERROR_MALLOC));
				break;
		}
	}
	
	terminate();
	
	return 0;
}

void terminate() {
	write(DESCRIPTOR_SCREEN, MSG_LOGOUT, STATIC_STRING_LEN(MSG_LOGOUT));
	
	if (clientFD >= 0) {
		write(clientFD, "Q|\n", 3*sizeof(char));
		close(clientFD);
	}
	
	freeCommands();
	
	free(input);
	free(ip);
	free(directory);
}

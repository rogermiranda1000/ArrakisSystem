#include "fremen.h"

volatile Status current_status = RUNNING;

char *ip, *directory;
char *input = NULL;
int clientFD = -1;

/**
 * Donat un format i els paràmetres (de la mateixa forma que es pasen a sprintf), retorna la string
 * /!\ Cal fer el free del buffer /!\
 * @param buffer	On es guardarà el resultat (char**)
 * @param format	Format (com a sprintf)
 * @param ...		Paràmetres del format (com a sprintf)
 * @return			String resultant
 */
#define concat(buffer, format, ...) ({													\
	size_t size = snprintf(NULL, 0, format, __VA_ARGS__); /* obtè la mida resultant */	\
	*buffer = (char*)malloc(size+1);													\
	sprintf(*buffer, format, __VA_ARGS__); /* retorna la mida */						\
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
	
	while (current_status != EXIT) {
		free(input); // allibera l'últim readUntil
		input = NULL;
		
		current_status = WAITING;
		readUntil(0, &input, '\n');
		current_status = RUNNING;
		
		switch(searchCommand(input, &output)) {
			/**
			 * -- Fremen -> Atreides --
			 * l|<nom>|<codi>\n -> login <nom> <codi>
			 * s|<codi>\n -> search <codi>
			 * [x] n|<file>\n -> send <file>
			 * [x] p|<id>\n -> photo <id>
			 * e|\n -> logout
			 *
			 * -- Atreides -> Fremen --
			 * l|<id>\n -> login efectuat correctament
			 **/
			
			case LOGIN:
				if (clientFD >= 0) {
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
				
				write(clientFD, "l|", 2*sizeof(char));
				write(clientFD, output[0], strlen(output[0])); // login
				write(clientFD, "|", sizeof(char));
				write(clientFD, output[1], strlen(output[1])); // code
				write(clientFD, "\n", sizeof(char));
				
				readUntil(clientFD, &read, '|');
				if (*read != 'l') {
					freeCommand(LOGIN, &output);
					write(DESCRIPTOR_ERROR, ERROR_COMUNICATION, STATIC_STRING_LEN(ERROR_COMUNICATION));
					free(read);
					break;
				}
				free(read);
				
				clientFD = readInteger(clientFD, NULL);
				write(DESCRIPTOR_SCREEN, read, concat(&read, "Benvingut %s. Tens ID %d.\n", output[0], clientFD));
				free(read);
				
				write(DESCRIPTOR_SCREEN, MSG_CONNECTED, STATIC_STRING_LEN(MSG_CONNECTED));
				
				freeCommand(LOGIN, &output);
				break;
				
			case SEARCH:
				write(DESCRIPTOR_SCREEN, output[0], strlen(output[0])); // code
				write(DESCRIPTOR_SCREEN, "\n", sizeof(char));
				
				freeCommand(SEARCH, &output);
				break;
				
			case PHOTO:
				write(DESCRIPTOR_SCREEN, output[0], strlen(output[0])); // id
				write(DESCRIPTOR_SCREEN, "\n", sizeof(char));
				
				freeCommand(PHOTO, &output);
				break;
				
			case SEND:
				write(DESCRIPTOR_SCREEN, output[0], strlen(output[0])); // file
				write(DESCRIPTOR_SCREEN, "\n", sizeof(char));
				
				freeCommand(SEND, &output);
				break;
				
			case LOGOUT:
				current_status = EXIT;
				clientFD = -1;
				
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
	
	freeCommands();
	
	free(input);
	free(ip);
	free(directory);
}

#include "fremen.h"

volatile Status current_status = RUNNING;

char *ip, *directory;
char *input = NULL;

void ctrlCHandler() {
	if (current_status == WAITING) {
		freeEverything();
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
	
	char **output;
	initCommands();
	
	// tmp (si no surt warning)
	char buffer[100];
	write(DESCRIPTOR_SCREEN, buffer, sprintf(buffer, "%d, %s, %d, %s\n", timeClean, ip, port, directory));
	
	while (current_status != EXIT) {
		free(input); // allibera l'últim readUntil
		input = NULL;
		
		current_status = WAITING;
		input = readUntil(0, '\n');
		current_status = RUNNING;
		
		switch(searchCommand(input, &output)) {
			case LOGIN:
				write(DESCRIPTOR_SCREEN, output[0], strlen(output[0])); // login
				write(DESCRIPTOR_SCREEN, " ", sizeof(char));
				write(DESCRIPTOR_SCREEN, output[1], strlen(output[1])); // code
				write(DESCRIPTOR_SCREEN, "\n", sizeof(char));
				
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
	
	freeEverything();
	
	return 0;
}

void freeEverything() {
	write(DESCRIPTOR_SCREEN, LOGOUT_MSG, STATIC_STRING_LEN(LOGOUT_MSG));
	
	freeCommands();
	
	free(input);
	free(ip);
	free(directory);
}

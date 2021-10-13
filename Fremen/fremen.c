#include "fremen.h"

Status current_status = RUNNING;

void intHandler(int signum) {
	if (current_status == RUNNING) {
		current_status = EXIT;
		signal(signum, intHandler); // no hauria de caldre, però per si és un impacient
	}
	else {
		current_status = EXIT;
		raise(SIGINT);
	}
}

int main(int argc, char *argv[], char *envp[]) {
	unsigned int timeClean;
	char* ip;
	unsigned short port;
	char* directory;

	signal(SIGINT, intHandler); // reprograma Control+C
	
	if (argc < 2) {
		write(DESCRIPTOR_ERROR, ERROR_ARGS, STATIC_STRING_LEN(ERROR_ARGS));
		exit(EXIT_FAILURE);
	}
	
	if (readConfig(argv[1], &timeClean, &ip, &port, &directory) == -1) {
		write(DESCRIPTOR_ERROR, ERROR_FILE, STATIC_STRING_LEN(ERROR_FILE));
		exit(EXIT_FAILURE);
	}
	
	char *input = NULL;
	int r;
	RegEx login_regex = regExInit("^LOGIN (\\S+) ([0-9]+)$", true),
		logout_regex = regExInit("^LOGOUT$", true);
	
	// TODO tmp
	char buffer[100], login[80], code[80];
	write(DESCRIPTOR_SCREEN, buffer, sprintf(buffer, "%d, %s, %d, %s\n", timeClean, ip, port, directory));
	
	while (current_status != EXIT) {
		free(input); // allibera l'últim readUntil
		input = NULL;
		
		current_status = WAITING;
		input = readUntil(0, '\n');
		if (current_status == EXIT) break;
		current_status = RUNNING;
		
		r = regExGet(&login_regex, input, login, code);
		if (r == EXIT_SUCCESS) {
			//int code_int = atoi(code);
			write(DESCRIPTOR_SCREEN, login, strlen(login));
			write(DESCRIPTOR_SCREEN, " ", sizeof(char));
			write(DESCRIPTOR_SCREEN, code, strlen(code));
			write(DESCRIPTOR_SCREEN, "\n", sizeof(char));
			continue;
		}
		
		r = regExGet(&logout_regex, input);
		if (r == EXIT_SUCCESS) {
			current_status = EXIT;
			break;
		}
		
		// any match
		write(DESCRIPTOR_ERROR, "Comanda invalida\n", sizeof("Comanda invalida\n")/sizeof(char));
		if (executeProgram(argv[1], &argv[1], envp) != 0) write(DESCRIPTOR_ERROR, "Error en executar la comanda\n", sizeof("Error en executar la comanda\n")/sizeof(char)); // TODO error
	}
	
	// alliberar memòria
	regExDestroy(&login_regex);
	regExDestroy(&logout_regex);
	
	free(input);
	free(ip);
	free(directory);
	
	return 0;
}

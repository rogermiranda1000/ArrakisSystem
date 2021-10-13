#include "fremen.h"

Status current_status = RUNNING;

void intHandler(int signum) {
	if (current_status == RUNNING) {
		current_status = EXIT;
		signal(signum, intHandler); // no hauria de caldre, però per si és un impacient
	}
	else {
		current_status = EXIT;
		//raise(SIGINT);
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
	
	int r;
	char *input = NULL, **output;
	RegEx login_regex = regExInit("^LOGIN\\s+(\\S+)\\s+(" REGEX_INTEGER ")$", true),
		logout_regex = regExInit("^LOGOUT$", true);
	
	// TODO tmp
	char buffer[100];
	write(DESCRIPTOR_SCREEN, buffer, sprintf(buffer, "%d, %s, %d, %s\n", timeClean, ip, port, directory));
	
	while (current_status != EXIT) {
		free(input); // allibera l'últim readUntil
		input = NULL;
		
		current_status = WAITING;
		input = readUntil(0, '\n');
		if (current_status == EXIT) break;
		current_status = RUNNING;
		
		r = regExSearch(&login_regex, input, &output);
		if (r == MALLOC_ERROR) {
			write(DESCRIPTOR_ERROR, ERROR_MALLOC, STATIC_STRING_LEN(ERROR_MALLOC));
			regExSearchFree(&login_regex, &output);
		}
		else if (r == EXIT_SUCCESS) {
			//int code_int = atoi(code);
			write(DESCRIPTOR_SCREEN, output[0], strlen(output[0])); // login
			write(DESCRIPTOR_SCREEN, " ", sizeof(char));
			write(DESCRIPTOR_SCREEN, output[1], strlen(output[1])); // code
			write(DESCRIPTOR_SCREEN, "\n", sizeof(char));
			
			// frees
			free(output[0]);
			free(output[1]);
			free(output);
			
			continue;
		}
		
		if (regExGet(&logout_regex, input) == EXIT_SUCCESS) {
			current_status = EXIT;
			break;
		}
		
		// no match
		if (executeProgramLine(input, envp) != 0) write(DESCRIPTOR_ERROR, "Error en executar la comanda\n", sizeof("Error en executar la comanda\n")/sizeof(char)); // TODO error
	}
	
	// alliberar memòria
	regExDestroy(&login_regex);
	regExDestroy(&logout_regex);
	
	free(input);
	free(ip);
	free(directory);
	
	return 0;
}

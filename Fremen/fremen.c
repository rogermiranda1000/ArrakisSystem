#include "fremen.h"

volatile Status current_status = RUNNING;

char *input = NULL, *ip, *directory;
RegEx login_regex, logout_regex, search_regex, photo_regex, send_regex;

void intHandler(int signum) {
	if (current_status == WAITING) {
		freeEverything();
		signal(SIGINT, SIG_DFL); // deprograma (tot i que hauria de ser així per defecte, per alguna raó no funciona)
		raise(SIGINT);
	}
	else {
		current_status = EXIT;
		signal(signum, intHandler); // no hauria de caldre, però per si és un impacient
	}
}

int main(int argc, char *argv[], char *envp[]) {
	unsigned int timeClean;
	unsigned short port;

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
	char **output;
	login_regex = regExInit("^LOGIN\\s+(\\S+)\\s+(" REGEX_INTEGER ")$", true);
	logout_regex = regExInit("^LOGOUT$", true);
	search_regex = regExInit("^SEARCH\\s+(" REGEX_INTEGER ")$", true);
	photo_regex = regExInit("^PHOTO\\s+(" REGEX_INTEGER ")$", true);
	send_regex = regExInit("^SEND\\s+(\\S+)$", true);
	
	// TODO tmp
	char buffer[100];
	write(DESCRIPTOR_SCREEN, buffer, sprintf(buffer, "%d, %s, %d, %s\n", timeClean, ip, port, directory));
	
	while (current_status != EXIT) {
		free(input); // allibera l'últim readUntil
		input = NULL;
		
		current_status = WAITING;
		input = readUntil(0, '\n');
		current_status = RUNNING;
		
		r = regExSearch(&login_regex, input, &output);
		if (r == MALLOC_ERROR) {
			write(DESCRIPTOR_ERROR, ERROR_MALLOC, STATIC_STRING_LEN(ERROR_MALLOC));
			regExSearchFree(&login_regex, &output);
		}
		else if (r == EXIT_SUCCESS) {
			write(DESCRIPTOR_SCREEN, output[0], strlen(output[0])); // login
			write(DESCRIPTOR_SCREEN, " ", sizeof(char));
			write(DESCRIPTOR_SCREEN, output[1], strlen(output[1])); // code
			write(DESCRIPTOR_SCREEN, "\n", sizeof(char));
			
			regExSearchFree(&login_regex, &output);
			
			continue;
		}
		
		r = regExSearch(&search_regex, input, &output);
		if (r == MALLOC_ERROR) {
			write(DESCRIPTOR_ERROR, ERROR_MALLOC, STATIC_STRING_LEN(ERROR_MALLOC));
			regExSearchFree(&search_regex, &output);
		}
		else if (r == EXIT_SUCCESS) {
			write(DESCRIPTOR_SCREEN, output[0], strlen(output[0])); // code
			write(DESCRIPTOR_SCREEN, "\n", sizeof(char));
			
			regExSearchFree(&search_regex, &output);
			
			continue;
		}
		
		r = regExSearch(&photo_regex, input, &output);
		if (r == MALLOC_ERROR) {
			write(DESCRIPTOR_ERROR, ERROR_MALLOC, STATIC_STRING_LEN(ERROR_MALLOC));
			regExSearchFree(&photo_regex, &output);
		}
		else if (r == EXIT_SUCCESS) {
			write(DESCRIPTOR_SCREEN, output[0], strlen(output[0])); // id
			write(DESCRIPTOR_SCREEN, "\n", sizeof(char));
			
			regExSearchFree(&photo_regex, &output);
			
			continue;
		}
		
		r = regExSearch(&send_regex, input, &output);
		if (r == MALLOC_ERROR) {
			write(DESCRIPTOR_ERROR, ERROR_MALLOC, STATIC_STRING_LEN(ERROR_MALLOC));
			regExSearchFree(&send_regex, &output);
		}
		else if (r == EXIT_SUCCESS) {
			write(DESCRIPTOR_SCREEN, output[0], strlen(output[0])); // file
			write(DESCRIPTOR_SCREEN, "\n", sizeof(char));
			
			regExSearchFree(&send_regex, &output);
			
			continue;
		}
		
		if (regExGet(&logout_regex, input) == EXIT_SUCCESS) {
			current_status = EXIT;
			break;
		}
		
		// no match
		if (executeProgramLine(input, envp) != 0) write(DESCRIPTOR_ERROR, "Error en executar la comanda\n", sizeof("Error en executar la comanda\n")/sizeof(char)); // TODO error
	}
	
	freeEverything();
	
	return 0;
}

void freeEverything() {
	regExDestroy(&login_regex);
	regExDestroy(&logout_regex);
	regExDestroy(&search_regex);
	regExDestroy(&photo_regex);
	regExDestroy(&send_regex);
	
	free(input);
	free(ip);
	free(directory);
}

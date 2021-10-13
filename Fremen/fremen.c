#include "fremen.h"

int main(int argc, char *argv[], char *envp[]) {
	unsigned int timeClean;
	char* ip;
	unsigned short port;
	char* directory;
	
	if (argc < 2) {
		write(DESCRIPTOR_ERROR, ERROR_ARGS, STATIC_STRING_LEN(ERROR_ARGS));
		exit(EXIT_FAILURE);
	}
	
	if (readConfig(argv[1], &timeClean, &ip, &port, &directory) == -1) {
		write(DESCRIPTOR_ERROR, ERROR_FILE, STATIC_STRING_LEN(ERROR_FILE));
		exit(EXIT_FAILURE);
	}
	
	char buffer[100];
	write(DESCRIPTOR_SCREEN, buffer, sprintf(buffer, "%d, %s, %d, %s", timeClean, ip, port, directory));
		
	RegEx login_regex = regExInit("^LOGIN (\\S+) ([0-9]+)$", true);
	char login[80], code[80];
	int r = regExGet(&login_regex, "lOgIn roger.miranda 08760", login, code);
	if (r == EXIT_SUCCESS) {
		//int code_int = atoi(code);
		write(DESCRIPTOR_SCREEN, login, strlen(login));
		write(DESCRIPTOR_SCREEN, " ", sizeof(char));
		write(DESCRIPTOR_SCREEN, code, strlen(code));
		write(DESCRIPTOR_SCREEN, "\n", sizeof(char));
	}
	r = regExGet(&login_regex, "lsssogin roger.miranda 08760", login, code);
	if (r == REG_NOMATCH) write(DESCRIPTOR_SCREEN, "Comanda invalida\n", sizeof("Comanda invalida\n")/sizeof(char));
	regExDestroy(&login_regex);
	free(ip);
	free(directory);
	
	return executeProgram(argv[1], &argv[1], envp);
}

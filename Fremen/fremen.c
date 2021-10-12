#include "fremen.h"

#define ERROR_FILE "Error al llegir el fitxer de configuració\n"

int main(int argc, char *argv[], char *envp[]) {
	if (argc < 2) return 1;
	
	char* name;
	unsigned int timeClean;
	char* ip;
	unsigned int port;
	char* directory;
	if (readConfig(argv[0], name, &timeClean, ip, &port, directory) != 0) write(DESCRIPTOR_SCREEN, ERROR_FILE, sizeof(ERROR_FILE)/sizeof(char));
	printf("%s, %d, %s, %d, %s", name, ip, port, directory);
		
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
	
	return executeProgram(argv[1], &argv[1], envp);
}

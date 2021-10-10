#include "fremen.h"

int main(int argc, char *argv[], char *envp[]) {
	if (argc < 2) return 1;
	
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

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
	if (argc < 2) return 1;
	
	signal(SIGINT, intHandler); // reprograma Control+C
	
	RegEx login_regex = regExInit("^LOGIN (\\S+) ([0-9]+)$", true);
	char login[80], code[80], buffer[80];
	int r, len;
	while (current_status != EXIT) {
		current_status = WAITING;
		len = read(0, buffer, 80*sizeof(char));
		if (current_status == EXIT) break;
		current_status = RUNNING;
		
		if (len < 0) continue; // TODO error
		buffer[len-1] = '\0'; // elimina el '\n' i afegeix '\0'
		
		r = regExGet(&login_regex, buffer, login, code);
		if (r == EXIT_SUCCESS) {
			//int code_int = atoi(code);
			write(DESCRIPTOR_SCREEN, login, strlen(login));
			write(DESCRIPTOR_SCREEN, " ", sizeof(char));
			write(DESCRIPTOR_SCREEN, code, strlen(code));
			write(DESCRIPTOR_SCREEN, "\n", sizeof(char));
		}
		else if (r == REG_NOMATCH) {
			write(DESCRIPTOR_ERROR, "Comanda invalida\n", sizeof("Comanda invalida\n")/sizeof(char));
			if (executeProgram(argv[1], &argv[1], envp) != 0) write(DESCRIPTOR_ERROR, "Error en executar la comanda\n", sizeof("Error en executar la comanda\n")/sizeof(char)); // TODO error
		}
	}
	regExDestroy(&login_regex);
	
	return 0;
}

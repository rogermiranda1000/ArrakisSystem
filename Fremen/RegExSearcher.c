#include "RegExSearcher.h"

RegEx regExInit(char *regex, bool ignore_case) {
	regex_t re;
	regmatch_t *rm = NULL;
	int retval;
	bool valid = false;

	int cflags = REG_EXTENDED;
	if (ignore_case) cflags |= REG_ICASE;

	if ((retval = regcomp(&re, regex, cflags)) != 0) {
		// print error
		char error[100], buffer[150];
		regerror(retval, &re, error, 255);
		sprintf(buffer, "Error en el RegEx '%s': %s\n", regex, error);
		write(2, buffer, strlen(buffer));
	}
	else {
		rm = (regmatch_t *) malloc(sizeof(regmatch_t) * (re.re_nsub + 1));
		valid = true;
	}

	return (RegEx){re, rm, valid};
}

int regExMatchesSize(RegEx *regex) {
	return regex->re.re_nsub;
}

void regExDestroy(RegEx *regex) {
	regfree(&regex->re);
	if (regex->rm != NULL) free(regex->rm);
	regex->rm = NULL;
	regex->valid = false;
}

int regExSearch(RegEx *regex, char *line, char ***matches) {
	int retval, size, characters;

	if (!regex->valid) return EXIT_FAILURE;

	if ((retval = regexec(&regex->re, line, regExMatchesSize(regex)+1, regex->rm, 0)) == 0) {
		size = regExMatchesSize(regex);
		*matches = (char**)malloc(sizeof(char*) * size);
		if (*matches == NULL) return MALLOC_ERROR;

		for (int x = 0; x < size; x++) {
			characters = (int)(regex->rm[x+1].rm_eo - regex->rm[x+1].rm_so);
			(*matches)[x] = (char*) malloc(sizeof(char) * (characters + 1));
			if ((*matches)[x] == NULL) return MALLOC_ERROR;
			strncpy((*matches)[x], line + regex->rm[x+1].rm_so, characters+1);
		}
	}
	else {
		if (retval == REG_NOMATCH) {
			return REG_NOMATCH;
		} else {
			// TODO: que error?
			//regerror(retval);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

int regExGet(RegEx *regex, char *line, ...) {
	va_list args;
	char **matches, *next;
	int r = regExSearch(regex, line, &matches);

	va_start(args, line); // el segundo argumento debe ser el último argumento válido de la función

	if (r == EXIT_SUCCESS) {
		for (int x = 0; x < regExMatchesSize(regex); x++) {
			next = va_arg(args, char*);
			if (next == NULL) continue;
			strncpy(next, matches[x], strlen(matches[x])+1);
			free(matches[x]);
		}
		free(matches);
	}
	return r;
}
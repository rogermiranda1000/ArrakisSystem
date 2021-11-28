#include "Commands.h"

RegEx login_regex, logout_regex, search_regex, photo_regex, send_regex;

void initCommands() {
	login_regex = regExInit("^LOGIN\\s+(\\S{1,230})\\s+(" REGEX_INTEGER ")$", true); // 230 son els màxims caracters com a usuari que el protocol establert pot suportar
	logout_regex = regExInit("^LOGOUT$", true);
	search_regex = regExInit("^SEARCH\\s+(" REGEX_INTEGER ")$", true);
	photo_regex = regExInit("^PHOTO\\s+(" REGEX_INTEGER ")$", true);
	send_regex = regExInit("^SEND\\s+(\\S+)$", true);
}

CommandResult searchCommand(char *input, char ***output) {
	int r;
	
	r = regExSearch(&login_regex, input, output);
	if (r == MALLOC_ERROR) {
		regExSearchFree(&login_regex, output);
		return ERROR;
	}
	else if (r == EXIT_SUCCESS) return LOGIN;
	
	r = regExSearch(&search_regex, input, output);
	if (r == MALLOC_ERROR) {
		regExSearchFree(&search_regex, output);
		return ERROR;
	}
	else if (r == EXIT_SUCCESS) return SEARCH;
	
	r = regExSearch(&photo_regex, input, output);
	if (r == MALLOC_ERROR) {
		regExSearchFree(&photo_regex, output);
		return ERROR;
	}
	else if (r == EXIT_SUCCESS) return PHOTO;
	
	r = regExSearch(&send_regex, input, output);
	if (r == MALLOC_ERROR) {
		regExSearchFree(&send_regex, output);
		return ERROR;
	}
	else if (r == EXIT_SUCCESS) return SEND;
	
	if (regExGet(&logout_regex, input) == EXIT_SUCCESS) return LOGOUT; // logout no té arguments -> no pot tenir malloc error
	
	return NO_MATCH;
}

void freeCommand(CommandResult result, char ***output) {
	switch(result) {
		case LOGIN:
			regExSearchFree(&login_regex, output);
			break;
		
		case LOGOUT:
			regExSearchFree(&logout_regex, output);
			break;
		
		case SEARCH:
			regExSearchFree(&search_regex, output);
			break;
		
		case PHOTO:
			regExSearchFree(&photo_regex, output);
			break;
		
		case SEND:
			regExSearchFree(&send_regex, output);
			break;
		
		default:
			// NO_MATCH/ERROR
			break;
	}
}

void freeCommands() {
	regExDestroy(&login_regex);
	regExDestroy(&logout_regex);
	regExDestroy(&search_regex);
	regExDestroy(&photo_regex);
	regExDestroy(&send_regex);
}
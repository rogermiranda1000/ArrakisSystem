#include "fremen.h"

volatile Status current_status = RUNNING;

char *ip, *directory;
char *input = NULL;

int clientFD = -1, clientID = -1;
char *name;

/**
 * Donat un format i els paràmetres (de la mateixa forma que es pasen a sprintf), imprimeix la string
 * @param fd		FileDescriptor on imprimir la string
 * @param format	Format (com a sprintf)
 * @param ...		Paràmetres del format (com a sprintf)
 */
#define susPrintF(fd, format, ...) ({							\
	char *buffer;												\
	write(fd, buffer, concat(&buffer, format, __VA_ARGS__));	\
	free(buffer);												\
})

void ctrlCHandler() {
	if (current_status == WAITING) {
		secureTermination();
		
		signal(SIGINT, SIG_DFL); // deprograma (tot i que hauria de ser així per defecte, per alguna raó no funciona)
		raise(SIGINT);
	}
	else {
		current_status = EXIT;
		
		signal(SIGINT, ctrlCHandler); // no hauria de caldre, però per si és un impacient
	}
}

void lostConnection() {
	write(DESCRIPTOR_ERROR, WARNING_LOST_CONNECTION, STATIC_STRING_LEN(WARNING_LOST_CONNECTION));
	
	clientID = -1; // estableix el client com desconectat
	
	// tanca la conexió
	close(clientFD);
	clientFD = -1;
}

int main(int argc, char *argv[], char *envp[]) {
	unsigned int timeClean;
	unsigned short port;

	signal(SIGINT, ctrlCHandler);	// reprograma Control+C
	signal(SIGPIPE, SIG_IGN);		// ignorem SIGPIPE
	
	if (argc < 2) {
		write(DESCRIPTOR_ERROR, ERROR_ARGS, STATIC_STRING_LEN(ERROR_ARGS));
		exit(EXIT_FAILURE);
	}
	
	if (readConfig(argv[1], &timeClean, &ip, &port, &directory) == -1) {
		write(DESCRIPTOR_ERROR, ERROR_FILE, STATIC_STRING_LEN(ERROR_FILE));
		exit(EXIT_FAILURE);
	}
	
	write(DESCRIPTOR_SCREEN, MSG_INIT, STATIC_STRING_LEN(MSG_INIT));
	
	char **output;
	Comunication data;
	SearchResults sr;
	
	int r;
	
	initCommands();
	
	while (current_status != EXIT) {
		free(input); // allibera l'últim readUntil
		input = NULL;
		
		current_status = WAITING;
		readUntil(0, &input, '\n');
		current_status = RUNNING;
		
		switch(searchCommand(input, &output)) {
			/**
			 * -- Fremen -> Atreides --
			 * C|<nom>*<codi>\n -> login <nom> <codi>
			 * s|<codi>\n -> search <codi>
			 * n|<file>\n -> send <file>
			 * p|<id>\n -> photo <id>
			 * Q|\n -> logout
			 *
			 * -- Atreides -> Fremen --
			 * O|<id>\n -> login efectuat correctament
			 **/
			
			case LOGIN:
				if (clientID >= 0) {
					freeCommand(LOGIN, &output);
					write(DESCRIPTOR_ERROR, ERROR_ALREADY_LOGGED, STATIC_STRING_LEN(ERROR_ALREADY_LOGGED));
					break;
				}
				
				clientFD = socketConnect(ip, port);
				if (clientFD < 0) {
					freeCommand(LOGIN, &output);
					write(DESCRIPTOR_ERROR, ERROR_SOCKET, STATIC_STRING_LEN(ERROR_SOCKET));
					break;
				}
				
				sendLogin(clientFD, output[0], output[1]);
				
				if (getMsg(clientFD, &data) != PROTOCOL_LOGIN_RESPONSE) {
					write(DESCRIPTOR_ERROR, ERROR_COMUNICATION, STATIC_STRING_LEN(ERROR_COMUNICATION));
					
					freeCommand(LOGIN, &output);
					break;
				}
				
				if ((clientID = getLoginResponse(&data)) == -1) {
					write(DESCRIPTOR_ERROR, ERROR_ID_ASSIGNMENT, STATIC_STRING_LEN(ERROR_ID_ASSIGNMENT));
					
					freeCommand(LOGIN, &output);
					break;
				}
				
				name = (char*)malloc(sizeof(char)*(strlen(output[0])+1));
				strcpy(name, output[0]);
				susPrintF(DESCRIPTOR_SCREEN, "Benvingut %s. Tens ID %d.\n", name, clientID);
				
				write(DESCRIPTOR_SCREEN, MSG_CONNECTED, STATIC_STRING_LEN(MSG_CONNECTED));
				
				freeCommand(LOGIN, &output);
				break;
				
			case LOGIN_INVALID:
				write(DESCRIPTOR_ERROR, ERROR_LOGIN_ARGS, STATIC_STRING_LEN(ERROR_LOGIN_ARGS));
				break;
				
			case SEARCH:
				if (clientID < 0) {
					write(DESCRIPTOR_ERROR, ERROR_NO_CONNECTION, STATIC_STRING_LEN(ERROR_NO_CONNECTION));
					freeCommand(SEARCH, &output);
					break;
				}
				
				sendSearch(clientFD, name, clientID, output[0]);
				
				sr = getSearchResponse(clientFD);
				
				if (sr.size == (size_t)-2) {
					lostConnection();
					freeCommand(SEARCH, &output);
					break;
				}
				else if (sr.size == (size_t)-1) write(DESCRIPTOR_ERROR, ERROR_COMUNICATION, STATIC_STRING_LEN(ERROR_COMUNICATION)); // Atreides ha rebut una trama incorrecta
				else if (sr.size == 0) susPrintF(DESCRIPTOR_SCREEN, "No hi ha cap persona humana a %s\n", output[0]);
				else {
					if (sr.size == 1) susPrintF(DESCRIPTOR_SCREEN, "Hi ha una persona humana a %s\n", output[0]);
					else susPrintF(DESCRIPTOR_SCREEN, "Hi ha %ld persones humanes a %s\n", sr.size, output[0]);
					
					for (size_t n = 0; n < sr.size; n++) susPrintF(DESCRIPTOR_SCREEN, "%d %s\n", sr.results[n].id, sr.results[n].name);
					freeSearchResponse(&sr);
				}
				
				freeCommand(SEARCH, &output);
				break;
				
			case SEARCH_INVALID:
				write(DESCRIPTOR_ERROR, ERROR_SEARCH_ARGS, STATIC_STRING_LEN(ERROR_SEARCH_ARGS));
				break;
				
			case SEND:
				if (clientID < 0) {
					write(DESCRIPTOR_ERROR, ERROR_NO_CONNECTION, STATIC_STRING_LEN(ERROR_NO_CONNECTION));
					freeCommand(SEND, &output);
					break;
				}
				
				r = sendPhoto(clientFD, "FREMEN", output[0], ".", envp, &terminate);
				if (r != 0) {
					if (r == -1) write(DESCRIPTOR_ERROR, ERROR_NO_FILE, STATIC_STRING_LEN(ERROR_NO_FILE));
					else lostConnection();
					
					freeCommand(SEND, &output);
					break;
				}
				
				if (getMsg(clientFD, &data) == PROTOCOL_SEND_RESPONSE && data.type == 'I') write(DESCRIPTOR_SCREEN, MSG_SEND_PHOTO_OK, STATIC_STRING_LEN(MSG_SEND_PHOTO_OK));
				else write(DESCRIPTOR_ERROR, ERROR_COMUNICATION, STATIC_STRING_LEN(ERROR_COMUNICATION));
				
				freeCommand(SEND, &output);
				break;
				
			case SEND_INVALID:
				write(DESCRIPTOR_ERROR, ERROR_SEND_ARGS, STATIC_STRING_LEN(ERROR_SEND_ARGS));
				break;
				
			case PHOTO:
				if (clientID < 0) {
					write(DESCRIPTOR_ERROR, ERROR_NO_CONNECTION, STATIC_STRING_LEN(ERROR_NO_CONNECTION));
					freeCommand(PHOTO, &output);
					break;
				}
				
				requestPhoto(clientFD, output[0]);
				
				r = getMsg(clientFD, &data);
				if (r != PROTOCOL_SEND) {
					if (r == PROTOCOL_UNKNOWN) lostConnection();
					else write(DESCRIPTOR_ERROR, ERROR_COMUNICATION, STATIC_STRING_LEN(ERROR_COMUNICATION));
					
					freeCommand(PHOTO, &output);
					break;
				}
				
				if (strcmp(data.data, "FILE NOT FOUND") == 0) {
					write(DESCRIPTOR_ERROR, ERROR_NO_PHOTO, STATIC_STRING_LEN(ERROR_NO_PHOTO));
					
					freeCommand(PHOTO, &output);
					break;
				}
				
				if (getPhoto(clientFD, directory, atoi(output[0]), envp, &terminate, &data, NULL, NULL) == 0) {
					write(DESCRIPTOR_SCREEN, MSG_DOWNLOAD_PHOTO_OK, STATIC_STRING_LEN(MSG_DOWNLOAD_PHOTO_OK));
					sendPhotoResponse(clientFD, "FREMEN", true);
				}
				else {
					write(DESCRIPTOR_ERROR, ERROR_PHOTO, STATIC_STRING_LEN(ERROR_PHOTO));
					sendPhotoResponse(clientFD, "FREMEN", false);
				}
				
				freeCommand(PHOTO, &output);
				break;
				
			case PHOTO_INVALID:
				write(DESCRIPTOR_ERROR, ERROR_PHOTO_ARGS, STATIC_STRING_LEN(ERROR_PHOTO_ARGS));
				break;
				
			case LOGOUT:
				current_status = EXIT;
				// el socket es tanca a terminate()
				
				// logout no té arguments -> no cal free
				break;
				
			case LOGOUT_INVALID:
				write(DESCRIPTOR_ERROR, ERROR_LOGOUT_ARGS, STATIC_STRING_LEN(ERROR_LOGOUT_ARGS));
				break;
			
			case NO_MATCH:
				if (executeProgramLine(&input, envp, &terminate) != 0) write(DESCRIPTOR_ERROR, ERROR_EXECUTE, STATIC_STRING_LEN(ERROR_EXECUTE));
				break;
				
			case ERROR:
				write(DESCRIPTOR_ERROR, ERROR_MALLOC, STATIC_STRING_LEN(ERROR_MALLOC));
				break;
		}
	}
	
	secureTermination();
	
	return 0;
}

void secureTermination() {
	write(DESCRIPTOR_SCREEN, MSG_LOGOUT, STATIC_STRING_LEN(MSG_LOGOUT));
	
	if (clientFD >= 0) {
		sendLogout(clientFD, name, clientID);
		// el socket es tanca a terminate()
	}
	
	terminate();
}

void terminate() {
	if (clientFD >= 0) close(clientFD);
	
	freeCommands();
	
	free(input);
	free(ip);
	free(directory);
	free(name);
}

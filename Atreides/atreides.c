#include "atreides.h"

#define DESCRIPTOR_SCREEN 1
#define DESCRIPTOR_ERROR 2
#define STATIC_STRING_LEN(str) (sizeof(str)/sizeof(char))

char *ip = NULL, *users_file_path = NULL, *directory = NULL;
int socketFD;

// TODO player FD

#define REGEX_INTEGER	"[0-9]{1,9}"

/**
 * Donat un format i els paràmetres (de la mateixa forma que es pasen a sprintf), retorna la string
 * /!\ Cal fer el free del buffer /!\
 * @param buffer	On es guardarà el resultat (char**)
 * @param format	Format (com a sprintf)
 * @param ...		Paràmetres del format (com a sprintf)
 * @return			String resultant
 */
#define concat(buffer, format, ...) ({													\
	size_t size = snprintf(NULL, 0, format, __VA_ARGS__); /* obtè la mida resultant */	\
	*buffer = (char*)malloc(size+1);													\
	sprintf(*buffer, format, __VA_ARGS__); /* retorna la mida */						\
})

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


/**
 * Allibera tota memoria
 */
void terminate() {
	close(socketFD);
	
	terminateUsers();

	free(ip);
	free(users_file_path);
	free(directory);
}

void ctrlCHandler() {
	terminateThreads(); // per alguna raó no ho puc posar a terminate()
	saveUsersFile(users_file_path);
	terminate();
	
	signal(SIGINT, SIG_DFL); // deprograma (tot i que hauria de ser així per defecte, per alguna raó no funciona)
	raise(SIGINT);
}

typedef struct {
	int clientFD;
	char **envp;
} ThreadInfo;

/**
 * Allibera tots els recursos de manageThread()
 * @param arg	Informació necesaria per alliberar els recursos
 */
static void freeThread(void *arg) {
	int clientFD = ((ThreadInfo*)arg)->clientFD;
	free(arg);
	
	close(clientFD);
}

/**
 * Gestiona la connexió amb un Fremen particular
 * @param arg	
 */
static void *manageThread(void *arg) {
	/**
	 * -- Fremen -> Atreides --
	 * C|<nom>*<codi>\n -> login <nom> <codi>
	 * s|<codi>\n -> search <codi>
	 * [x] n|<file>\n -> send <file>
	 * [x] p|<id>\n -> photo <id>
	 * e|\n -> logout
	 *
	 * -- Atreides -> Fremen --
	 * O|<id>\n -> login efectuat correctament
	 **/
	
	int clientFD = ((ThreadInfo*)arg)->clientFD;
	// el free es fa a freeThread
	pthread_cleanup_push(freeThread, arg);
	
	bool exit = false;
	Comunication data;
	
	RegEx regex;
	char **cmd_match;
	
	int user_id = -1;
	int search_postal;
	SearchResults search_data;

	char *tmp, tmp2[20];
	int response;

	while (!exit) {
		switch(getMsg(clientFD, &data)) {
			case PROTOCOL_LOGIN:
				regex = regExInit("^(\\S+)\\*(" REGEX_INTEGER ")$", false);
				if (regExSearch(&regex, data.data, &cmd_match) == REG_NOMATCH) {
					sendLoginResponse(clientFD, -1);
					
					// no cal fer el free del resultat (no n'hi ha cap)
					regExDestroy(&regex);
					break;
				}
				
				user_id = newLogin(cmd_match[0], cmd_match[1]);
				
				susPrintF(DESCRIPTOR_SCREEN, "Rebut login de %s %s\nAssignat a ID %d.\n", cmd_match[0], cmd_match[1], user_id);
				
				// envia l'ID a Fremen
				sendLoginResponse(clientFD, user_id); // login efectuat correctament
				
				write(DESCRIPTOR_SCREEN, INFO_SEND, STATIC_STRING_LEN(INFO_SEND));
				
				regExSearchFree(&regex, &cmd_match);
				regExDestroy(&regex);
				break;
				
			case PROTOCOL_SEARCH:
				search_postal = getSearch(&data);
				if (search_postal == -1) {
					// error -> enviar trama d'error
					search_data.size = -1;
					search_data.results = NULL;
					sendSearchResponse(clientFD, &search_data);
				}
				
				susPrintF(DESCRIPTOR_SCREEN, "Rebut search %d de %s %d\n", search_postal, getUser(user_id).login, user_id);
				
				search_data = getUsersByPostal(search_postal);
				
				write(DESCRIPTOR_SCREEN, INFO_SEARCH, STATIC_STRING_LEN(INFO_SEARCH));
				
				if (search_data.size == 0) susPrintF(DESCRIPTOR_SCREEN, "No hi ha cap persona humana a %d\n", search_postal);
				else {
					if (search_data.size == 1) susPrintF(DESCRIPTOR_SCREEN, "Hi ha una persona humana a %d\n", search_postal);
					else susPrintF(DESCRIPTOR_SCREEN, "Hi ha %ld persones humanes a %d\n", search_data.size, search_postal);
					
					for (size_t n = 0; n < search_data.size; n++) susPrintF(DESCRIPTOR_SCREEN, "%d %s\n", search_data.results[n].id, search_data.results[n].name);
				}
				
				sendSearchResponse(clientFD, &search_data);
				
				freeSearchResponse(&search_data);
				
				write(DESCRIPTOR_SCREEN, INFO_SEND, STATIC_STRING_LEN(INFO_SEND));
				
				break;
				
			case PROTOCOL_SEND:
				response = getPhoto(clientFD, directory, clientFD, ((ThreadInfo*)arg)->envp, &terminate, &data, &tmp, tmp2);
				susPrintF(DESCRIPTOR_SCREEN, "Rebut send %s de %s %d\n", tmp, getUser(user_id).login, user_id);
				if (response == 0) {
					susPrintF(DESCRIPTOR_SCREEN, "Guardada com %s\n", tmp2);
					setImage(user_id, tmp2);
					
					sendPhotoResponse(clientFD, true);
				}
				else {
					if (response < 0) write(DESCRIPTOR_ERROR, ERROR_PROTOCOL, STATIC_STRING_LEN(ERROR_PROTOCOL)); // error en la trama
					else write(DESCRIPTOR_ERROR, ERROR_MD5, STATIC_STRING_LEN(ERROR_MD5)); // error en l'md5
					setImage(user_id, NULL); // com hem sobreescribit la imatge, s'ha d'eliminar
					
					sendPhotoResponse(clientFD, false);
				}
				// TODO send rersponse
				break;
				
			case PROTOCOL_LOGOUT:
				if (user_id == -1) break;
				susPrintF(DESCRIPTOR_SCREEN, "Rebut logout de %s %d\n", getUser(user_id).login, user_id);
				write(DESCRIPTOR_SCREEN, USER_LOGOUT, STATIC_STRING_LEN(USER_LOGOUT));
				user_id = -1; // no cal, pero per si de cas
				exit = true;
				break;
				
			default:
				exit = true;
				break;
		}
		write(DESCRIPTOR_SCREEN, INFO_WAITING_USERS, STATIC_STRING_LEN(INFO_WAITING_USERS));
	}

	// Tancar conexió
	pthread_cleanup_pop(1);
	return NULL;
}

int main(int argc, char *argv[], char *envp[]) {
	struct sockaddr_in servidor;
	unsigned short port;
	
	signal(SIGINT, ctrlCHandler);
	
	write(DESCRIPTOR_SCREEN, INFO_START, STATIC_STRING_LEN(INFO_START));
	if (argc != 2) {
		write(DESCRIPTOR_ERROR, ERROR_ARGS, STATIC_STRING_LEN(ERROR_ARGS));
		exit(EXIT_FAILURE);
	}
	
	if (readConfig(argv[1], &ip, &port, &directory) == -1) {
		write(DESCRIPTOR_ERROR, ERROR_CONFIG_FILE, STATIC_STRING_LEN(ERROR_CONFIG_FILE));
		free(ip);
		free(directory);
		exit(EXIT_FAILURE);
	}
	concat(&users_file_path, "%s/%s", directory, USERS_FILE);
	
	loadUsersFile(users_file_path);
	
	// Crea el socket
	if ((socketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		free(ip);
		free(directory);
		free(users_file_path);
		terminateUsers();
		
		write(DESCRIPTOR_ERROR, ERROR_SOCKET, STATIC_STRING_LEN(ERROR_SOCKET));
		exit(EXIT_FAILURE);
	}

	memset(&servidor, 0, sizeof(servidor));
	servidor.sin_port = htons(port); // guarda el port del servidor
	servidor.sin_family = AF_INET; // SEMPRE AF_INET
	servidor.sin_addr.s_addr = inet_addr(ip); // acepta conexions de la ip

	// Assigna la IP al socket
 	if (bind(socketFD, (struct sockaddr*) &servidor, sizeof(servidor)) < 0) {
		free(ip);
		free(directory);
		free(users_file_path);
		terminateUsers();
		close(socketFD);
		
		write(DESCRIPTOR_ERROR, ERROR_BIND, STATIC_STRING_LEN(ERROR_BIND));
		exit(EXIT_FAILURE);
	}

	// Escolta conexions al socket
	if (listen(socketFD, SOCKET_QUEUE) < 0) {
		free(ip);
		free(directory);
		free(users_file_path);
		terminateUsers();
		close(socketFD);
		
		write(DESCRIPTOR_ERROR, ERROR_LISTEN, STATIC_STRING_LEN(ERROR_LISTEN));
		exit(EXIT_FAILURE);
	}
	
	while (true) {
		int clientFD = accept(socketFD, (struct sockaddr*) NULL, NULL);
		ThreadInfo info = (ThreadInfo){clientFD, envp};
		
		// Creació del thread
		if (createThread(&manageThread, &info, sizeof(ThreadInfo)) /* TODO la variable es destruirà abans de ser llegida? */ != 0) {
			write(DESCRIPTOR_ERROR, ERROR_THREAD, STATIC_STRING_LEN(ERROR_THREAD));
		}
	}

	return 0;
}

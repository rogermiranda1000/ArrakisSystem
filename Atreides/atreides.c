#include "atreides.h"

#define DESCRIPTOR_SCREEN 1
#define DESCRIPTOR_ERROR 2
#define STATIC_STRING_LEN(str) (sizeof(str)/sizeof(char))

typedef struct {
	User user;
	int fd;
} ThreadData;

char *ip = NULL, *users_file_path = NULL;
RegEx command_regex, login_regex;
int socketFD, num_threads = 0;
pthread_t *threads = NULL;
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

void ctrlCHandler() {
	int result;
	void *r;
	for (;num_threads > 0; num_threads--) {
		result = pthread_join(threads[num_threads-1], &r);
		if (result != 0) {
			write(DESCRIPTOR_ERROR, ERROR_JOIN, STATIC_STRING_LEN(ERROR_JOIN));
		}
	}
	free(threads);
	saveUsersFile(users_file_path);
	
	regExDestroy(&command_regex);
	regExDestroy(&login_regex);

	free(ip);
	free(users_file_path);
	
	
	signal(SIGINT, SIG_DFL); // deprograma (tot i que hauria de ser així per defecte, per alguna raó no funciona)
	raise(SIGINT);
}

/**
 *Gestiona la connexió amb un Fremen particular
 *
 */
static void *manageThread(void *arg) {
	char *msg, *cmd;
	char **matches = NULL;
	ThreadData data = *((ThreadData *)arg);
	User user = data.user;
	int clientFD = data.fd;
	write(DESCRIPTOR_SCREEN, msg, concat(&msg, "Rebut usuari %s.\n", user.login));
	free(msg);

	while (matches == NULL || *matches[0] != 'l') {
		readUntil(clientFD, &cmd, '\n');
		if (regExSearch(&command_regex, cmd, &matches) == EXIT_SUCCESS) {
			switch(*matches[0]) {
				case 's':
					break;
				case 'n':
					// TODO
					break;
				case 'p':
					// TODO
					break;
				case 'e':
					write(DESCRIPTOR_SCREEN, USER_LOGOUT, STATIC_STRING_LEN(USER_LOGOUT));
					break;
				default:
					//TODO
					break;
			}
		}
		free(cmd);
	}
	regExSearchFree(&command_regex, &matches);
	//TODO: Comprobar que no estigui descnectat?

	// Tancar conexió
	close(clientFD);
	clientFD = 0;
	return NULL;
}

int main(int argc, char *argv[]) {
	struct sockaddr_in servidor;
	unsigned short port;
	
	signal(SIGINT, ctrlCHandler);
	
	write(DESCRIPTOR_SCREEN, INFO_START, STATIC_STRING_LEN(INFO_START));
	if (argc != 2) {
		write(DESCRIPTOR_ERROR, ERROR_ARGS, STATIC_STRING_LEN(ERROR_ARGS));
		exit(EXIT_FAILURE);
	}
	
	char *directory = NULL;
	if (readConfig(argv[1], &ip, &port, &directory) == -1) {
		write(DESCRIPTOR_ERROR, ERROR_CONFIG_FILE, STATIC_STRING_LEN(ERROR_CONFIG_FILE));
		free(ip);
		free(directory);
		exit(EXIT_FAILURE);
	}
	concat(&users_file_path, ".%s/%s", directory, USERS_FILE);
	free(directory);
	
	loadUsersFile(users_file_path);
	
	// Crea el socket
	if ((socketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		write(DESCRIPTOR_ERROR, ERROR_SOCKET, STATIC_STRING_LEN(ERROR_SOCKET));
		exit(EXIT_FAILURE);
	}

	memset(&servidor, 0, sizeof(servidor));
	servidor.sin_port = htons(port); // guarda el port del servidor
	servidor.sin_family = AF_INET; // SEMPRE AF_INET
	servidor.sin_addr.s_addr = inet_addr(ip); // acepta conexions de la ip

	// Assigna la IP al socket
 	if (bind(socketFD, (struct sockaddr*) &servidor, sizeof(servidor)) < 0) {
		write(DESCRIPTOR_ERROR, ERROR_BIND, STATIC_STRING_LEN(ERROR_BIND));
		exit(EXIT_FAILURE);
	}

	// Escolta conexions al socket
	if (listen(socketFD, SOCKET_QUEUE) < 0) {
		write(DESCRIPTOR_ERROR, ERROR_LISTEN, STATIC_STRING_LEN(ERROR_LISTEN));
		exit(EXIT_FAILURE);
	}
	write(DESCRIPTOR_SCREEN, INFO_WAITING_USERS, STATIC_STRING_LEN(INFO_WAITING_USERS));

	command_regex = regExInit("^(.)|(.*)$", false);
	login_regex = regExInit("^(.)|(" REGEX_INTEGER ")$", false);
	
	
	while (true) {
		int clientFD = accept(socketFD, (struct sockaddr*) NULL, NULL);
		
		/**
		 * -- Fremen -> Atreides --
		 * l|<nom>|<codi>\n -> login <nom> <codi>
		 * s|<codi>\n -> search <codi>
		 * [x] n|<file>\n -> send <file>
		 * [x] p|<id>\n -> photo <id>
		 * e|\n -> logout
		 *
		 * -- Atreides -> Fremen --
		 * l|\n -> login efectuat correctament
		 **/

		char *cmd, *msg;
		char **matches = NULL, **cmd_match;
		int user_id = -1;
		int resultat_thread;
		User user = {0};
		while (matches == NULL || *matches[0] != 'l') {
			readUntil(clientFD, &cmd, '\n');
			if (regExSearch(&command_regex, cmd, &matches) == EXIT_SUCCESS) {
				switch(*matches[0]) {
					case 'l':
						regExSearch(&login_regex, matches[1], &cmd_match);
						strcpy(user.login, cmd_match[0]);
						user.postal = atoi(cmd_match[1]);
						user_id = newLogin(cmd_match[0], cmd_match[1]);
						
						write(DESCRIPTOR_SCREEN, "Rebut login ", STATIC_STRING_LEN("Rebut login "));
						write(DESCRIPTOR_SCREEN, cmd_match[0], strlen(cmd_match[0]));
						write(DESCRIPTOR_SCREEN, " ", sizeof(char));
						write(DESCRIPTOR_SCREEN, cmd_match[1], strlen(cmd_match[1]));
						write(DESCRIPTOR_SCREEN, "\n", sizeof(char));

						write(DESCRIPTOR_SCREEN, msg, concat(&msg, "Assignat a ID %d.\n", user_id));
						free(msg);
						
						write(clientFD, LOGIN_OK, STATIC_STRING_LEN(LOGIN_OK)); // login efectuat correctament
						write(DESCRIPTOR_SCREEN, INFO_SEND, STATIC_STRING_LEN(INFO_SEND));

						// Creació del thread
						threads = (pthread_t *)realloc(threads, sizeof(pthread_t)*(++num_threads));
						resultat_thread = pthread_create(&threads[num_threads-1], NULL, manageThread, &user);
						if (resultat_thread != 0) {
							write(DESCRIPTOR_ERROR, ERROR_THREAD, STATIC_STRING_LEN(ERROR_THREAD));
						}

						break;
					case 's':
						break;
					case 'n':
						// TODO
						break;
					case 'p':
						// TODO
						break;
					case 'e':
						write(DESCRIPTOR_SCREEN, USER_LOGOUT, STATIC_STRING_LEN(USER_LOGOUT));
						break;
				}
			}
			free(cmd);
		}
		regExSearchFree(&command_regex, &matches);

		// Tancar conexió
		close(clientFD);
		clientFD = 0;
	}

	return 0;
}

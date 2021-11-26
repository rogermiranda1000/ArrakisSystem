#include "atreides.h"

char *ip = NULL, *users_file_path = NULL;

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
	free(ip);
	free(users_file_path);
}

int main(int argc, char *argv[]) {
	struct sockaddr_in servidor;
	unsigned short port;
	
	signal(SIGINT, ctrlCHandler);
	
	if (argc != 2) {
		// TODO
		exit(EXIT_FAILURE);
	}
	
	
	char *directory;
	if (readConfig(argv[1], &ip, &port, &directory) != -1) {
		// TODO
		exit(EXIT_FAILURE);
	}
	concat(&users_file_path, "%s/%s", directory, USERS_FILE)
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
	write(DESCRIPTOR_SCREEN, MSG_SERVER_INIT, STATIC_STRING_LEN(MSG_SERVER_INIT));

	while (true) {
		write(DESCRIPTOR_SCREEN, MSG_WAITING_USER, STATIC_STRING_LEN(MSG_WAITING_USER));
		clientFD = accept(socketFD, (struct sockaddr*) NULL, NULL);

		readUntil(clientFD, name, '\n');
		write(DESCRIPTOR_SCREEN, buffer, sprintf(buffer, "%s ha iniciado Sesión.\n", name));
		sprintf(buffer, "%s/%s.txt", DIRECTORY, name);
		
		clientFile = open(buffer, O_RDWR | O_CREAT, 00666);
		items = readItemsFromFile(clientFile);

		session_interrupted = false;
		write(DESCRIPTOR_SCREEN, MSG_SENDING_LIST, STATIC_STRING_LEN(MSG_SENDING_LIST));
		sendItems(clientFD, items);
		write(DESCRIPTOR_SCREEN, buffer, sprintf(buffer, "Lista enviada a %s.\n\n", name));
		while (!session_interrupted) {
			// envia Producto#N\n o OUT\n
			readUntil(clientFD, buffer, '#');
			if (strcmp(buffer, "OUT\n") == 0) break;
			Item *i = getItemFromList(&items, buffer);
			readUntil(clientFD, buffer, '\n');
			value = secureAtoi(buffer);
			
			write(DESCRIPTOR_SCREEN, MSG_RECIEVED_MOD, STATIC_STRING_LEN(MSG_RECIEVED_MOD));
			if (value == RESET) write(DESCRIPTOR_SCREEN, buffer, sprintf(buffer, "\tEliminar %s\n", i->name));
			else write(DESCRIPTOR_SCREEN, buffer, sprintf(buffer, "\t%dx %s\n", value, i->name));

			if (operateItem(i, value)) {
				write(DESCRIPTOR_SCREEN, buffer, sprintf(buffer, "Lista de %s actualizada.\n", name));
				write(DESCRIPTOR_SCREEN, MSG_SENDING_LIST, STATIC_STRING_LEN(MSG_SENDING_LIST));
				sendItems(clientFD, items);
				write(DESCRIPTOR_SCREEN, buffer, sprintf(buffer, "Lista enviada a %s.\n\n", name));
			} else {
				write(clientFD, "KO\n", 3*sizeof(char));
				write(DESCRIPTOR_SCREEN, buffer, sprintf(buffer, "No se ha podido actualizar la lista de %s.\n", name));
			}
		}

		// Tancar conexió
		write(DESCRIPTOR_SCREEN, buffer, sprintf(buffer, "%s ha cerrado la sesión.\n", name));
		saveItemsInFile(clientFile, items);
		close(clientFile);
		clientFile = 0;

		freeItems(&items);
		close(clientFD);
		clientFD = 0;
	}

	return 0;
}

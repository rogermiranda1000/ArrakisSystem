#include "Users.h"

#define ERROR_FILE_CREATE
#define INIT_FILE "0\n"

/**
 * Llista d'usuaris
 */
Users usuaris = {NULL, 0};

/**
 * Llegeix del FileDescriptor fins trobar el delimitador i decideix si continuar o no
 * Útil per utilitzar junt amb '\n' com a stop_delimiter
 * /!\ El delimitador no es guarda al buffer, però sí s'elimina del FileDescriptor /!\
 * /!\ Cal fer el free del buffer /!\
 * @param fd					FileDescriptor on agafar les dades
 * @param buffer				Array on guardar la informació (amb '\0')
 * @param size					Caracters obtninguts (sense contar '\0'; NULL si no es vol guardar)
 * @param continue_delimiter	Caracter que marca quan parar la lectura
 * @param stop_delimiter		Caracter que marca quan parar la lectura
 * @retval true					S'ha detingut a continue_delimiter
 * @retval false				S'ha detingut a stop_delimiter
 */
bool readUntilAndContinue(int fd, char **buffer, size_t *size, char continue_delimiter, char stop_delimiter) {
	char aux;
	size_t s = 0;
	*buffer = NULL;
	
	while(read(fd, &aux, sizeof(char)) == sizeof(char) && aux != continue_delimiter && aux != stop_delimiter) {
		*buffer = (char*)realloc(*buffer, (s+1)*sizeof(char));
		(*buffer)[s++] = aux;
	}
	*buffer = (char*)realloc(*buffer, (s+1)*sizeof(char));
	(*buffer)[s] = '\0';
	
	if (size != NULL) *size = s;
	
	return (aux == continue_delimiter);
}

/**
 * Llegeix del FileDescriptor fins trobar el delimitador
 * /!\ El delimitador no es guarda al buffer, però sí s'elimina del FileDescriptor /!\
 * /!\ Cal fer el free del buffer /!\
 * @param fd		FileDescriptor on agafar les dades
 * @param buffer	Array on guardar la informació (amb '\0')
 * @param delimiter	Caracter que marca quan parar
 * @return			Caracters obtninguts (sense contar '\0')
 */
size_t readUntil(int fd, char **buffer, char delimiter) {
	size_t size;
	
	readUntilAndContinue(fd, buffer, &size, delimiter, delimiter);
	
	return size;
}

/**
 * Llegeix del File Descriptor tranforman-lo a enter i para a delimiter (agafant-lo)
 * /!\ El delimitador no es guarda al buffer, però sí s'elimina del FileDescriptor /!\
 * @param fd 		File Desriptor a consultar
 * @param stop 		Caracter on s'ha parat la conversió (NULL si no es vol guardar)
 * @return			Enter trobat (0 si res)
 */
int readInteger(int fd, char *stop) {
	char aux;
	int r = 0;
	bool negative = false;
	
	if (read(fd, &aux, sizeof(char)) != sizeof(char)) return r;
	if (aux == '-') negative = true;
	else r = aux - '0';
	
	while(read(fd, &aux, sizeof(char)) == sizeof(char) && aux >= '0' && aux <= '9') {
		r *= 10;
		r += aux - '0';
	}
	
	if (stop != NULL) *stop = aux; // guarda el caracter on s'ha detingut la conversió
	
	return negative ? -r : r;
}

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
 * Afegeix una entrada a la variable interna d'usuaris
 * @param login		Login del usuari
 * @param postal	Codi postal del usuari
 * @return			ID del usuari afegit
 */
int addEntry(char *login, char *postal) {
	usuaris.len++;
	usuaris.users = (User*)realloc(usuaris.users, sizeof(User)*usuaris.len);
	usuaris.users[usuaris.len-1].login = (char*)malloc(sizeof(char)*(strlen(login)+1));
	strcpy(usuaris.users[usuaris.len-1].login, login);
	usuaris.users[usuaris.len-1].postal = atoi(postal);
	return usuaris.len-1;
}

/**
 * Elimina tots els usuaris de la memoria interna
 */
void emptyUsers() {
	for (size_t x = 0; x < usuaris.len; x++) free(usuaris.users[x].login);
	
	free(usuaris.users);
	usuaris.users = NULL;
	usuaris.len = 0;
}

int loadUsersFile(char *path) {
	int file = open(path, O_RDWR);
    
    if (file < 0) return -1; // no hi ha fitxer -> deixar usuaris tal com està

    int num_users = readInteger(file, NULL);
    char *userFound, *postalFound;
    for (int userId = 0; userId < num_users; userId++) {
		userFound = NULL;
		postalFound = NULL;
		
        if (readUntil(file, &userFound,',') == 0 || readUntil(file, &postalFound,'\n')) {
			free(userFound);
			free(postalFound);
			close(file);
            return -2;
        }
		
		addEntry(userFound, postalFound);
		
		free(userFound);
		free(postalFound);
    }
    
    close(file);
	return 0;
}

int saveUsersFile(char *path) {
	int file = open(path, O_RDWR | O_CREAT);
	
	if (file < 0) return file;
	
	char *entry;
	write(file, entry, concat(&entry, "%d\n", (int)usuaris.len));
	free(entry);
	
	for (size_t x = 0; x < usuaris.len; x++) {
		write(file, entry, concat(&entry, "%s,%d\n", usuaris.users[x].login, usuaris.users[x].postal));
		free(entry);
	}
	
	emptyUsers(); // allibera memoria
	return 0;
}

/**
 * Donat un nom retorna el seu ID
 * @param login Nom del usuari
 * @return 		ID del usuari (-1) si no trobat
 */
int searchUserByName(char *login) {
	for (size_t userID = 0; userID < usuaris.len; userID++) {
		if (strcmp(usuaris.users[userID].login, login) == 0) return userID;
	}
	
	return -1; // no trobat
}

int newLogin(char *login, char *postal) {
    int r = searchUserByName(login);
	
	if (r == -1) return addEntry(login, postal); // no existeix -> afegir
	
	// exiteix
	usuaris.users[r].postal = atoi(postal); // per si de cas ha cambiat de regió
    return r;
}
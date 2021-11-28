#include "Users.h"

#define ERROR_FILE_CREATE
#define INIT_FILE "0\n"

/**
 * Llista d'usuaris
 */
Users usuaris = {NULL, 0};

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
	
    close(file);
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

User getUser(int id) {
	return usuaris.users[id];
}

SearchResults getUsersByPostal(int postal) {
	SearchResults r = (SearchResults){NULL, 0};
	for (size_t x = 0; x < usuaris.len; x++) {
		if (usuaris.users[x].postal != postal) continue;
		r.results = (SearchResult*)realloc(r.results, sizeof(SearchResult)*(++r.size));
		r.results[r.size-1] = (SearchResult){(char*)malloc(sizeof(char)*(1+strlen(usuaris.users[x].login))), x};
		strcpy(r.results[r.size-1].name, usuaris.users[x].login);
	}
	return r;
}
#include "Users.h"

#define ERROR_FILE_CREATE
#define INIT_FILE "0\n"

/**
 * Asegura que només 1 està accedint a usuaris
 */
pthread_mutex_t users_lock = PTHREAD_MUTEX_INITIALIZER;

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
 * @param image_type Tipus d'imatge del usuari. NULL si cap, "\0" si sense tipus
 * @return			ID del usuari afegit
 */
int addEntry(char *login, char *postal, char *image_type) {
	usuaris.len++;
	usuaris.users = (User*)realloc(usuaris.users, sizeof(User)*usuaris.len);
	
	usuaris.users[usuaris.len-1].login = (char*)malloc(sizeof(char)*(strlen(login)+1));
	strcpy(usuaris.users[usuaris.len-1].login, login);
	
	usuaris.users[usuaris.len-1].postal = atoi(postal);
	
	usuaris.users[usuaris.len-1].image_type = NULL;
	if (image_type != NULL) {
		usuaris.users[usuaris.len-1].image_type = (char*)malloc(sizeof(char)*(strlen(image_type)+1));
		strcpy(usuaris.users[usuaris.len-1].image_type, image_type);
	}
	
	return usuaris.len-1;
}

void setImage(int user_id, char *image) {
	char *image_type = "";
	if (image == NULL) {
		pthread_mutex_lock(&users_lock);
		free(usuaris.users[user_id].image_type);
		usuaris.users[user_id].image_type = NULL;
		pthread_mutex_unlock(&users_lock);
	}
	else {
		// buscar la extensió
		while (*image != '\0') {
			if (*image == '.') image_type = image+1;
			image++;
		}
		
		pthread_mutex_lock(&users_lock);
		free(usuaris.users[user_id].image_type);
		
		usuaris.users[user_id].image_type = (char*)malloc(sizeof(char)*(strlen(image_type)+1));
		strcpy(usuaris.users[user_id].image_type, image_type);
		pthread_mutex_unlock(&users_lock);
	}
}

/**
 * Elimina tots els usuaris de la memoria interna
 */
void emptyUsers() {
	pthread_mutex_lock(&users_lock);
	for (size_t x = 0; x < usuaris.len; x++) {
		free(usuaris.users[x].login);
		free(usuaris.users[x].image_type);
	}
	
	free(usuaris.users);
	usuaris.users = NULL;
	usuaris.len = 0;
	pthread_mutex_unlock(&users_lock);
}

void terminateUsers() {
	emptyUsers(); // allibera memoria
	pthread_mutex_destroy(&users_lock);
}

int loadUsersFile(char *path) {
	int file = open(path, O_RDONLY);
    
    if (file < 0) return -1; // no hi ha fitxer -> deixar usuaris tal com està

    int num_users = readInteger(file, NULL);
    char *userFound, *postalFound, *image_type;
    for (int userId = 0; userId < num_users; userId++) {
		userFound = NULL;
		postalFound = NULL;
		
        if (readUntil(file, &userFound,',') == 0 || readUntil(file, &postalFound,',') == 0 || readUntil(file, &image_type,'\n') == 0) {
			free(userFound);
			free(postalFound);
			free(image_type);
			close(file);
            return -2;
        }
		
		// no cal protegir (en el moment que es fa loadUsersFile() no hi ha cap thread creat)
		addEntry(userFound, postalFound, (strcmp(image_type, "-") == 0) ? NULL : image_type);
		
		free(userFound);
		free(postalFound);
		free(image_type);
    }
    
    close(file);
	return 0;
}

int saveUsersFile(char *path) {
	int file = open(path, O_WRONLY | O_CREAT, 00666);
	
	if (file < 0) return file;
	
	char *entry;
	
	pthread_mutex_lock(&users_lock);
	write(file, entry, concat(&entry, "%d\n", (int)usuaris.len));
	free(entry);
	
	for (size_t x = 0; x < usuaris.len; x++) {
		write(file, entry, concat(&entry, "%s,%d,%s\n", usuaris.users[x].login, usuaris.users[x].postal, (usuaris.users[x].image_type == NULL) ? "-" : usuaris.users[x].image_type));
		free(entry);
	}
	pthread_mutex_unlock(&users_lock);
	
    close(file);
	
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
	pthread_mutex_lock(&users_lock);
    int r = searchUserByName(login);
	if (r == -1) r = addEntry(login, postal, NULL); // no existeix -> afegir
	pthread_mutex_unlock(&users_lock);
	
	// exiteix
	usuaris.users[r].postal = atoi(postal); // per si de cas ha cambiat de regió
    return r;
}

User getUser(int id) {
	pthread_mutex_lock(&users_lock);
	User r = usuaris.users[id];
	pthread_mutex_unlock(&users_lock);
	
	return r;
}

SearchResults getUsersByPostal(int postal) {
	SearchResults r = (SearchResults){NULL, 0};
	
	pthread_mutex_lock(&users_lock);
	for (size_t x = 0; x < usuaris.len; x++) {
		if (usuaris.users[x].postal != postal) continue;
		r.results = (SearchResult*)realloc(r.results, sizeof(SearchResult)*(++r.size));
		r.results[r.size-1] = (SearchResult){(char*)malloc(sizeof(char)*(1+strlen(usuaris.users[x].login))), x};
		strcpy(r.results[r.size-1].name, usuaris.users[x].login);
	}
	pthread_mutex_unlock(&users_lock);
	
	return r;
}
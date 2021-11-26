#include "Users.h"

#define ERROR_FILE_CREATE
#define INIT_FILE "0\n"

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

int newLogin(char* login) {
    int userId;
    int file = open(USERS_FILE, O_RDWR);
    
    if (file < 0) {
        file = open(USERS_FILE, O_RDWR | O_CREAT);
        if (file < 0) {
            //TODO: Missatge error aquí? o a fora?
            return -1;
        }
        //Inicialitzo el fitxer
        write(file, INIT_FILE, STATIC_STRING_LEN(INIT_FILE));
    }

    //Busquem l'usuari
    lseek(file, 0, SEEK_SET);
    int users = readInteger(file, NULL);
    char* userFound;
    bool found = false;
	int i;
    for (i = 0; i < users && !found; i++) {
        if (readUntil(file, &userFound,'\n') == 0) {
			free(userFound);
            //TODO: Error aquí?
            return -1;
        }
		free(userFound);
        if (strcmp(userFound, login) == 0) {
            found = true;
        }
    }
    userId = i;
    if (!found) {
        write(file, login, strlen(login));
        // Augmentar numero primer
    }
    

    close(file);

    return userId;
}
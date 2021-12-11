#include "ConfigReader.h"

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

size_t readUntil(int fd, char **buffer, char delimiter) {
	size_t size;
	
	readUntilAndContinue(fd, buffer, &size, delimiter, delimiter);
	
	return size;
}

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

int readConfig(char* name, char** ip, unsigned short* port, char** directory) {
	int file = open(name, O_RDONLY);
	if (file < 0) return -1;
	
	readUntil(file, ip, '\n');
	*port = readInteger(file, NULL);
	readUntil(file, directory, '\n');
	close(file);
	
	// directory és path relatiu, establert com absolut
	if (**directory == '/') {
		char *aux = (char*)malloc(sizeof(char)*(2+strlen(*directory)));
		aux[0] = '.';
		strcpy(&aux[1], *directory);
		free(*directory);
		*directory = aux;
	}
	
	return 0;
}
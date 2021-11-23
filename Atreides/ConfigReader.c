#include "ConfigReader.h"

int readInteger(int fd, char delimiter) {
	char aux;
	int r = 0;
	
	while(read(fd, &aux, sizeof(char)) == sizeof(char) && aux != delimiter) {
		r *= 10;
		r += aux - '0';
	}
	
	return r;
}

char *readUntil(int fd, char delimiter) {
	char aux;
	size_t size = 0;
	char *r = NULL;
	
	while (read(fd, &aux, sizeof(char)) == sizeof(char) && aux != delimiter) {
		r = (char*)realloc(r, sizeof(char)*(++size));
		r[size-1] = aux;
	}
	
	// '\0'
	r = (char*)realloc(r, sizeof(char)*(++size));
	r[size-1] = '\0';
	
	return r;
}

int readConfig(char* name, char** ip, unsigned short* port, char** directory) {
	int file = open(name, O_RDONLY);
	if (file < 0) {
		return -1;
	}
	*ip = readUntil(file, '\n');
	*port = readInteger(file, '\n');
	*directory = readUntil(file, '\n');
	close(file);
	return 0;
}
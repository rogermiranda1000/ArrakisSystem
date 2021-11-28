#include "Comunication.h"

/**
 * Donada una mida i dos Strings, les copia
 * /!\ origen ha de cabre a desti /!\
 * @param dest		String a on copiar la informació
 * @param origen	String d'on agafar la informació
 * @param lenght	Caracters a copiar
 */
void staticLenghtCopy(char *desti, char *origen, size_t lenght) {
	bool origen_ended = false;
	for (size_t x = 0; x < lenght; x++) {
		if (origen_ended) desti[x] = '\0';
		else {
			desti[x] = origen[x];
			origen_ended = (origen[x] == '\0');
		}
	}
}

void sendLogin(int socket, char *name, char *postal) {
	char *data;
	Comunication trama;
	staticLenghtCopy(trama.name, "FREMEN", COMUNICATION_NAME_LEN);
	trama.type = 'C';
	concat(&data, "%s*%s", name, postal);
	staticLenghtCopy(trama.data, data, DATA_LEN);
	free(data);
	
	write(socket, &trama, sizeof(Comunication));
}

MsgType getMsg(int socket, Comunication *data) {
	read(socket, data, sizeof(Comunication));
	switch(data->type) {
		case 'C':
			return PROTOCOL_LOGIN;
			
		case 'O':
		case 'E':
			return PROTOCOL_LOGIN_RESPONSE;
			
		case 'Q':
			return PROTOCOL_LOGOUT;
			
		default:
			return PROTOCOL_UNKNOWN;
	}
}

int getLoginResponse(Comunication *data) {
	if (data->type == 'E') return -1;
	return atoi(data->data);
}

void sendLoginResponse(int socket, int id) {
	char *data;
	Comunication trama;
	staticLenghtCopy(trama.name, "ATREIDES", COMUNICATION_NAME_LEN);
	trama.type = 'O';
	concat(&data, "%d", id);
	staticLenghtCopy(trama.data, data, DATA_LEN);
	free(data);
	
	write(socket, &trama, sizeof(Comunication));
}

void sendLogout(int socket, char *name, int id) {
	char *data;
	Comunication trama;
	staticLenghtCopy(trama.name, "FREMEN", COMUNICATION_NAME_LEN);
	trama.type = 'Q';
	concat(&data, "%s*%d", name, id);
	staticLenghtCopy(trama.data, data, DATA_LEN);
	free(data);
	
	write(socket, &trama, sizeof(Comunication));
}
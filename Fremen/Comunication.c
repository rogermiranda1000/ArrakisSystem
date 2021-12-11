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

void sendPhoto(int socket, char *photoName, char *md5sum) {
	char *data;
	Comunication msg;
	char *fileSize;
	int fileBytes;

	int photoFd = open(photoName, O_RDONLY);
	// Mida del fitxer
	fileBytes = lseek(photoFd, 0, SEEK_END);
	concat(&fileSize, "%d", fileBytes);
	lseek(photoFd, 0, SEEK_SET);

	// Creem la trama
	staticLenghtCopy(msg.name, "FREMEN", COMUNICATION_NAME_LEN);
	msg.type = 'F';
	concat(&data, "%s*%s*%s", photoName, fileSize, md5sum);
	staticLenghtCopy(msg.data, data, DATA_LEN);

	// Enviem la trama inicial
	write(socket, &msg, sizeof(Comunication));

	free(data);
	free(fileSize);
	
	// Enviem les dades
	Comunication msgData;
	staticLenghtCopy(msgData.name, "FREMEN", COMUNICATION_NAME_LEN);
	msgData.type = 'D';
	for (int i = 0; i < fileBytes; i+= DATA_LEN) {
		read(photoFd, msgData.data, sizeof(char)*DATA_LEN);
		write(socket, &msg, sizeof(Comunication));
	}
	close(photoFd);
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
	if (read(socket, data, sizeof(Comunication)) != sizeof(Comunication)) return PROTOCOL_UNKNOWN;
	switch(data->type) {
		case 'C':
			return PROTOCOL_LOGIN;
			
		case 'O':
		case 'E':
			return PROTOCOL_LOGIN_RESPONSE;
			
		case 'S':
			return PROTOCOL_SEARCH;
			
		case 'L':
		case 'K':
			return PROTOCOL_SEARCH_RESPONSE;
			
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
	if (id >= 0) {
		trama.type = 'O';
		concat(&data, "%d", id);
	}
	else {
		trama.type = 'E';
		concat(&data, "%s", "ERROR"); // ha de tenir un argument obligatoriament
	}
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

bool sendSearch(int socket, char *name, int id, char *postal) {
	char *data;
	Comunication trama;
	staticLenghtCopy(trama.name, "FREMEN", COMUNICATION_NAME_LEN);
	trama.type = 'S';
	concat(&data, "%s*%d*%s", name, id, postal);
	staticLenghtCopy(trama.data, data, DATA_LEN);
	free(data);
	
	return write(socket, &trama, sizeof(Comunication)) == sizeof(Comunication);
}

int getSearch(Comunication *data) {
	int postal;
	char **cmd_match;
	RegEx regex = regExInit("^\\S+\\*" REGEX_INTEGER "\\*(" REGEX_INTEGER ")$", false);
	if (regExSearch(&regex, data->data, &cmd_match) == REG_NOMATCH) {
		// no cal fer el free del resultat (no n'hi ha cap)
		regExDestroy(&regex);
		return -1;
	}
	
	postal = atoi(cmd_match[0]);
	
	regExSearchFree(&regex, &cmd_match);
	regExDestroy(&regex);
	
	return postal;
}

void sendSearchResponse(int socket, SearchResults *results) {
	char *data, *data_append, *tmp;
	Comunication trama;
	staticLenghtCopy(trama.name, "ATREIDES", COMUNICATION_NAME_LEN);
	trama.type = 'L';
	concat(&data, "%ld", results->size);
	for (size_t x = 0; x < results->size; x++) {
		concat(&data_append, "*%s*%d", results->results[x].name, results->results[x].id);
		if (strlen(data)+strlen(data_append) < DATA_LEN-1) {
			concat(&tmp, "%s%s", data, data_append);
			free(data);
			data = tmp;
			free(data_append);
		}
		else {
			staticLenghtCopy(trama.data, data, DATA_LEN);
			free(data);
			data = data_append;
			
			write(socket, &trama, sizeof(Comunication));
		}
	}
	staticLenghtCopy(trama.data, data, DATA_LEN);
	free(data);
	
	write(socket, &trama, sizeof(Comunication));
}

SearchResults getSearchResponse(int socket) {
	SearchResults r;
	Comunication data;
	MsgType first_message = getMsg(socket, &data);
	if (first_message != PROTOCOL_SEARCH_RESPONSE || data.type == 'K') return (SearchResults){NULL, -1};
	
	char *tmp = data.data;
	
	// obtè el número d'elements
	int amount = 0;
	while (*tmp != '*' && *tmp != '\0') {
		amount *= 10;
		amount += (*tmp - '0');
		tmp++;
	}
	
	r.size = amount;
	if (amount > 0) r.results = (SearchResult*)malloc(sizeof(SearchResult)*amount);
	else r.results = NULL;
	
	char *text_data = (char*)malloc(sizeof(char)*(strlen(tmp)+1));
	strcpy(text_data, tmp);
	
	RegEx regex = regExInit("^\\*([^*]+)\\*(" REGEX_INTEGER ")(\\S*)$", false); // TODO fer que els noms aceptin '*'
	char **cmd_match;
	while (amount > 0) {
		if (*text_data == '\0') {
			// s'ha esgotat la trama, però no s'ha acabat
			free(text_data);
			
			first_message = getMsg(socket, &data);
			if (first_message != PROTOCOL_SEARCH_RESPONSE) {
				regExDestroy(&regex);
				return r;
			}
			
			text_data = (char*)malloc(sizeof(char)*(strlen(data.data)+1));
			strcpy(text_data, data.data);
		}
		
		regExSearch(&regex, text_data, &cmd_match);
		
		r.results[amount-1] = (SearchResult){(char*)malloc(sizeof(char)*(1+strlen(cmd_match[0]))), atoi(cmd_match[1])};
		strcpy(r.results[amount-1].name, cmd_match[0]);
		
		// mou el "punter"
		free(text_data);
		text_data = (char*)malloc(sizeof(char)*(strlen(cmd_match[2]) + 1));
		strcpy(text_data, cmd_match[2]);
		
		regExSearchFree(&regex, &cmd_match);
		amount--;
	}
	
	free(text_data);
	regExDestroy(&regex);
	
	return r;
}

void freeSearchResponse(SearchResults *data) {
	for (size_t n = 0; n < data->size; n++) free(data->results[n].name);
	free(data->results);
	
	data->results = NULL;
	data->size = 0;
}
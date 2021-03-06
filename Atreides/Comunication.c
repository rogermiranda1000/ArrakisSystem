#include "Comunication.h"

#define DESCRIPTOR_SCREEN 1
#define DESCRIPTOR_ERROR 2
#define STATIC_STRING_LEN(str) (sizeof(str)/sizeof(char))

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

/**
 * Extreu el nom i extensió del fitxer
 * /!\ No cal free; si es destrueix 'str' es destrueixen les demés /!\
 * /!\ 'str' es modificarà (s'agefeix un '\0' un cop trobat 'delimiter') /!\
 * @param str				String on comença el nom del fitxer
 * @param delimiter			Caracter on pararà de llegir
 * @param file_name			On guardarà el nom del fitxer (+ extensió); NULL si no es vol guardar
 * @param file_extension	On guardarà la extensió; NULL si no es vol guardar
 * @return					On s'ha quedat d''str'
 */
char *getFileNameInfo(char *str, char delimiter, char **file_name, char **file_extension) {
	if (file_extension != NULL) *file_extension = NULL;
	if (file_name != NULL) *file_name = str;
	while (*str != delimiter) {
		if (*str == '.' && file_extension != NULL) *file_extension = str+1;
		str++;
	}
	*str = '\0'; // per la lectura de després
	return str+1; // elimina el '*'
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
			
		case 'F':
		case 'D':
			return PROTOCOL_SEND;
			
		case 'I':
		case 'R':
			return PROTOCOL_SEND_RESPONSE;
			
		case 'P':
			return PROTOCOL_PHOTO;
			
		default:
			return PROTOCOL_UNKNOWN;
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

void sendSearch(int socket, char *name, int id, char *postal) {
	char *data;
	Comunication trama;
	staticLenghtCopy(trama.name, "FREMEN", COMUNICATION_NAME_LEN);
	trama.type = 'S';
	concat(&data, "%s*%d*%s", name, id, postal);
	staticLenghtCopy(trama.data, data, DATA_LEN);
	free(data);
	
	write(socket, &trama, sizeof(Comunication));
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

int sendPhoto(int socket, char *origin, char *photo_name, char *photo_path, char *envp[], void (*freeMallocs)()) {
	char *data, *type;
	Comunication msg;
	char *fileSize;
	int fileBytes;
	
	// obtenim l'extensió
	getFileNameInfo(photo_name, '\0', NULL, &type);
	if (type == NULL || (strcmp(type, "jpg") != 0 && strcmp(type, "JPG") != 0)) return -2;
	
	int photoFD;
	char *file_path;
	concat(&file_path, "%s/%s", photo_path, photo_name);
	if ((photoFD = open(file_path, O_RDONLY)) < 0) {
		free(file_path);
		return -1;
	}
	
	// md5sum command
	ForkedPipeInfo fork_pipe;
	char *command = (char*)malloc(sizeof(char) * (2 + STATIC_STRING_LEN("md5sum ") + strlen(photo_path) + strlen(photo_name)));
	strcpy(command, "md5sum ");
	strcat(command, file_path);
	free(file_path);
	// el free es fa a executeProgramLineWithPipe()
	
	executeProgramLineWithPipe(&fork_pipe, &command, envp, freeMallocs);
	char *md5sum;
	readUntil(fdPipeInfo(fork_pipe, 0), &md5sum, ' '); // md5sum retorna '<md5> *<nom fitxer>'
	freeForkedPipeInfo(&fork_pipe);

	// Mida del fitxer
	fileBytes = lseek(photoFD, 0, SEEK_END);
	concat(&fileSize, "%d", fileBytes);
	lseek(photoFD, 0, SEEK_SET);

	// Creem la trama
	staticLenghtCopy(msg.name, origin, COMUNICATION_NAME_LEN);
	msg.type = 'F';
	concat(&data, "%s*%s*%s", photo_name, fileSize, md5sum);
	free(md5sum);
	staticLenghtCopy(msg.data, data, DATA_LEN);

	free(data);
	free(fileSize);

	// Enviem la trama inicial
	if (write(socket, &msg, sizeof(Comunication)) != sizeof(Comunication)) {
		close(photoFD);
		return -3;
	}
	
	// Enviem les dades
	msg.type = 'D';
	lseek(photoFD, 0, SEEK_SET);
	for (int i = 0; i < fileBytes; i+= DATA_LEN) {
		read(photoFD, msg.data, DATA_LEN);
		if (write(socket, &msg, sizeof(Comunication)) != sizeof(Comunication)) {
			close(photoFD);
			return -3;
		}
	}
	
	close(photoFD);
	return 0;
}

void sendNoPhoto(int socket) {
	Comunication msg;
	
	staticLenghtCopy(msg.name, "ATREIDES", COMUNICATION_NAME_LEN);
	msg.type = 'F';
	staticLenghtCopy(msg.data, "FILE NOT FOUND", DATA_LEN);

	write(socket, &msg, sizeof(Comunication));
}

int getPhoto(int socket, char *img_folder_path, int user_id, char *envp[], void (*freeMallocs)(), Comunication *data, char **original_image_name, char *final_image_name) {
	char *ptr = data->data, *md5, *type;
	size_t file_size = 0;
	MsgType msg;
	Comunication extra_data;
	int photo_fd;
	
	ptr = getFileNameInfo(ptr, '*', original_image_name, &type);
	
	// creem el FD
	char *path_name, *file_name;
	if (type == NULL) concat(&file_name, "%d", user_id); // no té tipus
	else concat(&file_name, "%d.%s", user_id, type);
	concat(&path_name, "%s/%s", img_folder_path, file_name);
	if ((photo_fd = open(path_name, O_WRONLY | O_CREAT, 00666)) < 0) return -1;
	
	if (final_image_name != NULL) strcpy(final_image_name, file_name);
	free(file_name);
	
	// seguim tractant la trama original
	while (*ptr != '*') {
		file_size *= 10;
		file_size += *ptr - '0';
		ptr++;
	}
	ptr++; // elimina el '*'
	
	md5 = ptr;
	
	while (file_size > 0) {
		msg = getMsg(socket, &extra_data);
		if (msg != PROTOCOL_SEND) return -2;
		write(photo_fd, extra_data.data, (file_size > DATA_LEN) ? DATA_LEN : file_size);
		
		if (file_size > DATA_LEN) file_size -= DATA_LEN;
		else file_size = 0;
	}
	
	close(photo_fd);
	
	// md5sum command
	ForkedPipeInfo fork_pipe;
	char *command = (char*)malloc(sizeof(char) * (1 + STATIC_STRING_LEN("md5sum ") + strlen(path_name)));
	strcpy(command, "md5sum ");
	strcat(command, path_name);
	free(path_name);
	// el free es fa a executeProgramLineWithPipe()
	
	executeProgramLineWithPipe(&fork_pipe, &command, envp, freeMallocs);
	char *md5sum;
	readUntil(fdPipeInfo(fork_pipe, 0), &md5sum, ' '); // md5sum retorna '<md5> *<nom fitxer>'
	int r = (strcmp(md5sum, md5) != 0);
	free(md5sum);
	freeForkedPipeInfo(&fork_pipe);
	
	return r;
}

void sendPhotoResponse(int socket, char *origin, bool ok) {
	Comunication msg;
	
	staticLenghtCopy(msg.name, origin, COMUNICATION_NAME_LEN);
	if (ok) {
		msg.type = 'I';
		staticLenghtCopy(msg.data, "IMAGE OK", DATA_LEN);
	}
	else {
		msg.type = 'R';
		staticLenghtCopy(msg.data, "IMAGE KO", DATA_LEN);
	}
	
	write(socket, &msg, sizeof(Comunication));
}

void requestPhoto(int socket, char *id) {
	Comunication msg;
	
	staticLenghtCopy(msg.name, "FREMEN", COMUNICATION_NAME_LEN);
	msg.type = 'P';
	staticLenghtCopy(msg.data, id, DATA_LEN);

	write(socket, &msg, sizeof(Comunication));
}
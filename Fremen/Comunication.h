#pragma once

#include <unistd.h>
#include <stdbool.h>	// bool/true/false
#include <stdio.h>		// snprintf
#include <stdlib.h>		// malloc/free
#include <fcntl.h>		// O_RDONLY
#include "RegExSearcher.h"
#include "ProgramLauncher.h"
#include "ConfigReader.h" // readUntil

#define COMUNICATION_NAME_LEN 	15
#define DATA_LEN				240

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

typedef struct {
	char name[COMUNICATION_NAME_LEN];
	char type;
	char data[DATA_LEN];
} Comunication;

typedef enum {
	PROTOCOL_LOGIN,				// Fremen fa login a Atreides
	PROTOCOL_LOGIN_RESPONSE,	// Atreides dona l'OK/error al login de Fremen
	PROTOCOL_LOGOUT,			// Fremen fa logout d'Atreides
	PROTOCOL_SEARCH,			// Fremen solicita una busqueda
	PROTOCOL_SEARCH_RESPONSE,	// Atreides respon a Fremen sobre la busqueda
	PROTOCOL_SEND,
	PROTOCOL_SEND_RESPONSE,
	PROTOCOL_PHOTO,
	PROTOCOL_PHOTO_RESPONSE,
	PROTOCOL_LOST,
	PROTOCOL_UNKNOWN
} MsgType;

typedef struct {
	char *name;
	int id;
} SearchResult;

typedef struct {
	SearchResult *results;
	size_t size;
} SearchResults;

/**
 * Obtè un missatge
 * @param socket 	Socket d'on escoltar
 * @param data		Trama obtinguda
 * @return			Tipu de trama obtinguda
 */
MsgType getMsg(int socket, Comunication *data);

/**
 * Fremen -> Atreides
 * Envia login
 * @param socket 	Socket de comunicació amb Atreides
 * @param name 		Nom del usuari
 * @param postal 	Codi postal
 */
void sendLogin(int socket, char *name, char *postal);

/**
 * Obtè les dades de la variable data
 * /!\ Només cridar si getMsg() ha retornat PROTOCOL_LOGIN_RESPONSE /!\
 * @param data		Punter a la trama obtinguda
 * @return			ID obtingut; -1 si error
 */
int getLoginResponse(Comunication *data);


/**
 * Atreides -> Fremen
 * Envia l'OK del login
 * @param socket 	Socket de comunicació amb Atreides
 * @param id 		ID asignat al usuari (-1 si error)
 */
void sendLoginResponse(int socket, int id);

/**
 * Fremen -> Atreides
 * Envia logout
 * @param socket 	Socket de comunicació amb Atreides
 * @param name 		Nom del usuari
 * @param id		ID del usuari
 */
void sendLogout(int socket, char *name, int id);

/**
 * Fremen -> Atreides
 * Envia solicitud de search
 * @param socket 	Socket de comunicació amb Atreides
 * @param name 		Nom del usuari
 * @param id		ID del usuari
 * @param postal	Codi postal a cercar
 * @retval true		Tot OK
 * @retval false	Error al enviar pel socket
 */
bool sendSearch(int socket, char *name, int id, char *postal);

/**
 * Obtè la sol·licitud de cerca
 * /!\ Només cridar si getMsg() ha retornat PROTOCOL_SEARCH /!\
 * @param data 		Punter a la trama obtinguda
 * @return			Codi postal que es desitja cercar (-1 si error)
 */
int getSearch(Comunication *data);

/**
 * Atreides -> Fremen
 * Envia logout
 * @param socket 	Socket de comunicació amb Atreides
 * @param results 	Informació a enviar
 */
void sendSearchResponse(int socket, SearchResults *results);

/**
 * Obtè les dades de la variable data
 * /!\ Només cridar si getMsg() ha retornat PROTOCOL_SEARCH_RESPONSE /!\
 * /!\ El contingut retornar s'ha d'alliberar cridant freeSearchResponse() /!\
 * @param socket 	Socket de comunicació amb Atreides
 * @return			Resultats (size és -1 si ha pasat algo amb la comunicació, -2 si s'ha perdut la conexió)
 */
SearchResults getSearchResponse(int socket);

/**
 * Allibera la memoria reservada per SearchResults
 * @param data		Contingut a alliberar
 */
void freeSearchResponse(SearchResults *data);

/**
 * Fremen -> Atreides
 * Envia la foto del usuari
 * @param socket 	Socket de comunicació amb Atreides
 * @param photoName Ruta de la foto
 * @param photoFD 	FileDescriptor de la imatge
 *					/!\ Ha de ser vàl·lid /!\
 * @param md5sum 	Suma md5 del fitxer
 */
void sendPhoto(int socket, char *photoName, int photoFD, char *md5sum);


/**
 * Obtè la foto del usuari
 * @param socket 		Socket de comunicació amb Fremen
 * @param user_id 		Usuari que envia la imatge
 * @param envp 			Variables d'envirement
 * @param freeMallocs	Funció per alliberar la memoria del pare al fer el fork
 * @param data 			Primera trama llegida
 * @retval -1			Error obrint el fitxer
 * @retval -2			Error en la trama
 * @retval >0			Error en MD5
 * @retval 0			Tot OK
 */
int getPhoto(int socket, char *img_folder_path, int user_id, char *envp[], void (*freeMallocs)(), Comunication *data);
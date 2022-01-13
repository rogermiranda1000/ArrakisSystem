#pragma once

#include <unistd.h>
#include <stdbool.h>			// bool/true/false
#include <stdio.h>				// snprintf
#include <stdlib.h>				// malloc/free
#include <fcntl.h>				// O_RDONLY
#include "RegExSearcher.h"
#include "ProgramLauncher.h"	// executeProgramLineWithPipe
#include "ConfigReader.h" 		// readUntil

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
	PROTOCOL_SEND_RESPONSE,		// send & photo response
	PROTOCOL_PHOTO,
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
 */
void sendSearch(int socket, char *name, int id, char *postal);

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
 * Fremen -> Atreides o Atreides -> Fremen
 * Envia la foto del usuari
 * @param socket 	Socket de comunicació
 * @param origin	FREMEN/ATREIDES
 * @param photo_name Nom de la foto
 * @param photo_path Ruta de la foto
 * @param envp 			Variables d'envirement
 * @param freeMallocs	Funció per alliberar la memoria del pare al fer el fork
 * @retval 0		Tot OK
 * @retval -1		El fitxer no existeix
 * @retval -2		El fitxer té una extensió invàl·lida
 * @retval -3		SIGPIPE
 */
int sendPhoto(int socket, char *origin, char *photo_name, char *photo_path, char *envp[], void (*freeMallocs)());

/**
 * Atreides -> Fremen
 * La foto solicitada no existeix
 * @param socket 	Socket de comunicació amb Fremen
 */
void sendNoPhoto(int socket);

/**
 * Obtè la foto del usuari
 * @param socket 		Socket de comunicació amb Fremen
 * @param user_id 		Usuari que envia la imatge
 * @param envp 			Variables d'envirement
 * @param freeMallocs	Funció per alliberar la memoria del pare al fer el fork
 * @param data 			Primera trama llegida
 * @param original_image_name 	Punter a on guardar el nom de la imatge (no cal fer free; apunta a data). NULL si no es vol guardar
 * @param final_image_name 		char[] on guardar el nom final de la imatge (no pot ser dinamic). NULL si no es vol guardar
 * @retval -1			Error obrint el fitxer
 * @retval -2			Error en la trama
 * @retval >0			Error en MD5
 * @retval 0			Tot OK
 */
int getPhoto(int socket, char *img_folder_path, int user_id, char *envp[], void (*freeMallocs)(), Comunication *data, char **original_image_name, char *final_image_name);

/**
 * Fremen -> Atreides o Atreides -> Fremen
 * Retorna la informació corresponent a la imatge
 * @param socket	Socket de comunicació
 * @param origin	FREMEN/ATREIDES
 * @param ok		1 si tot ok, 0 si algun error
 */
void sendPhotoResponse(int socket, char *origin, bool ok);

/**
 * Fremen -> Atreides
 * Soliciata la imatge d'un usuari
 * @param socket	Socket de comunicació amb Atreides
 * @param id		ID del usuari a obtenir la imatge
 */
void requestPhoto(int socket, char *id);
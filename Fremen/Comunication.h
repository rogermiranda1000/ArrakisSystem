#pragma once

#include <unistd.h>
#include <stdbool.h>	// bool/true/false
#include <stdio.h>		// snprintf
#include <stdlib.h>		// malloc/free

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
	PROTOCOL_SEARCH,
	// TODO altres
	PROTOCOL_UNKNOWN
} MsgType;

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
 * @param id 		ID asignat al usuari
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
#pragma once

#include "ProgramLauncher.h"
#include "ConfigReader.h"
#include "Commands.h"
#include "Socket.h"
#include "Comunication.h"
#include "FileCleaner.h"
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h> // exit

#define DESCRIPTOR_SCREEN 1
#define DESCRIPTOR_ERROR 2

#define STATIC_STRING_LEN(str) (sizeof(str)/sizeof(char))

#define MSG_INIT				"Benvingut a Fremen\n"
#define MSG_LOGOUT				"Tancant Fremen...\n"
#define MSG_CONNECTED			"Ara estàs connectat a Atreides.\n"
#define MSG_SEND_PHOTO_OK		"Foto enviada amb èxit a Atreides.\n"
#define MSG_DOWNLOAD_PHOTO_OK	"Foto descarregada\n"

#define ERROR_FILE 				"Error al llegir el fitxer de configuració\n"
#define ERROR_ARGS 				"Has d'indicar el nom del fitxer de configuració\n"
#define ERROR_LOGIN_ARGS		"La comanda login s'executa de la següent forma: 'LOGIN <usuari> <codi postal>'\n"
#define ERROR_LOGOUT_ARGS		"La comanda logout s'executa de la següent forma: 'LOGOUT'\n"
#define ERROR_SEARCH_ARGS		"La comanda search s'executa de la següent forma: 'SEARCH <codi postal>'\n"
#define ERROR_PHOTO_ARGS		"La comanda photo s'executa de la següent forma: 'PHOTO <id>'\n"
#define ERROR_SEND_ARGS			"La comanda send s'executa de la següent forma: 'SEND <archiu>'\n"
#define ERROR_MALLOC 			"Malloc error, torna a intentar-ho quan Matagalls no estigui tant saturat\n"
#define ERROR_EXECUTE			"Error en executar la comanda\n"
#define ERROR_ALREADY_LOGGED	"Ja estas conectat a Atreides!\n"
#define ERROR_NO_CONNECTION		"No estas conectat a Atreides\n"
#define ERROR_SOCKET			"Error al crear el socket\n"
#define ERROR_COMUNICATION		"S'esperava un altre resposta per part d'Atreides\n"
#define ERROR_ID_ASSIGNMENT		"Atreides ha retornat una ID invàl·lida\n"
#define ERROR_NO_FILE			"El fitxer no existeix\n"
#define ERROR_PHOTO_EXTENSION	"Les imatges han de ser .jpg\n"
#define ERROR_NO_PHOTO			"No hi ha foto registrada.\n"
#define ERROR_PHOTO				"Error al descarregar la imatge\n"

#define WARNING_LOST_CONNECTION	"Atreides s'ha desconectat\n"

typedef enum {
	WAITING,	// en un read()
	EXIT,		// ha saltat interrupció estant en estat 'RUNNING'; apaga quan pugui
	RUNNING		// està treballant
} Status;

int main(int argc, char *argv[], char *envp[]);

/**
 * Acaba el programa o marca un flag indicant que ha d'acabar
 */
void ctrlCHandler();

/**
 * Allibera tota memoria global reservada, donant abans un missatge informatiu
 */
void secureTermination();

/**
 * Allibera tota memoria global reservada
 */
void terminate();

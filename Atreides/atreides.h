#pragma once

#include <unistd.h>
#include <stdbool.h> 	// bool/true/false
#include <arpa/inet.h>	// inet_pton/htons
#include <sys/socket.h>	// socket/connect
#include <signal.h>
#include <stdlib.h>     // itoa
#include <stdio.h>      
#include <string.h>     // strcpy
#include "ConfigReader.h"
#include "Users.h"
#include "Threads.h"
#include "Comunication.h"
#include "RegExSearcher.h"

#define SOCKET_QUEUE 10

#define ERROR_ARGS				"Fichero de configuración no indicado\n"
#define ERROR_CONFIG_FILE		"Error en la lectura del fichero de configuración\n"
#define ERROR_SOCKET			"Error creando el socket\n"
#define ERROR_BIND				"Error haciendo el bind\n"
#define ERROR_LISTEN			"Error haciendo el listen\n"
#define ERROR_THREAD			"Error creando thread\n"
#define ERROR_PROTOCOL			"Error: no s'esperaba aquesta trama\n"
#define ERROR_MD5				"Error en la suma md5\n"

#define INFO_START				"SERVIDOR ATREIDES\n"
#define INFO_READ_FILE			"Llegit el fitxer de configuració\n"
#define INFO_WAITING_USERS		"Esperant connexions...\n\n"
#define INFO_SEND				"Enviada resposta\n"
#define INFO_SEARCH				"Feta la cerca\n"

#define USER_LOGOUT				"Desconnectat d'Atreides.\n"

#define USERS_FILE				"registeredUsers.txt"

int main(int argc, char *argv[], char *envp[]);

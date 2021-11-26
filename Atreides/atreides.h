#pragma once

#include <unistd.h>
#include <stdbool.h> 	// bool/true/false
#include <pthread.h>	// pthread_create/pthread_join/pthread_t
#include <arpa/inet.h>	// inet_pton/htons
#include <sys/socket.h>	// socket/connect
#include "ConfigReader.h"
#include "Users.h"

#define ERROR_ARGS				"IP y puerto de servidor no especificado\n"
#define ERROR_SOCKET			"Error creando el socket\n"
#define ERROR_BIND				"Error haciendo el bind\n"
#define ERROR_LISTEN			"Error haciendo el listen\n"

int main(int argc, char *argv[]);

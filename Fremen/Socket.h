#pragma once

#include <arpa/inet.h>	// inet_pton/htons
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>		// memset

/**
 * Crea una conexió amb sockets
 * @param ip	IPv4 a la que conectar-se
 * @param port 	Port de conexió
 * @return		El socket asignat al usuari (o <0 si error)
 */
int socketConnect(char *ip, unsigned short port);
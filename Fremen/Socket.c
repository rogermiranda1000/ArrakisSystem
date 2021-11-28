#include "Socket.h"

int socketConnect(char *ip, unsigned short port) {
	int socketFD;
	
	// Creem el socket
	if ((socketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) return -1;
	
	// Configurem socket struct
	struct sockaddr_in s_addr;
	memset(&s_addr, 0, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(port);
	
	if (inet_pton(AF_INET, ip, &s_addr.sin_addr) != 1) return -2;
	
	// Connectem
	if (connect(socketFD, (struct sockaddr*) &s_addr, sizeof(s_addr)) < 0) return -3;
	
	return socketFD;
}
#pragma once

#include <unistd.h>
#include <fcntl.h>		// O_RDONLY
#include <stdlib.h>		// realloc

#define DESCRIPTOR_SCREEN 1
#define DESCRIPTOR_ERROR 2

/**
 * Llegeix del FileDescriptor fd fins delimiter, sense incloure (però borrant-lo de fd)
 * /!\ S'ha de fer free del retorn /!\
 * @param fd 		FileDescriptor on llegir
 * @param delimiter	Delimitador per parar de llegir
 * @return			String llegida (amb '\0')
 */
char *readUntil(int fd, char delimiter);

/**
 * Llegeix un fitxer de configuració, guardant la informació a las variables
 * /!\ Important fer els frees dels char* /!\
 * @param name		Ruta del fitxer de configuració
 * @param timeClean	Temps a esperar abans de netejar els fitxers temporals
 * @param ip		IP del servidor central
 * @param port		Port del servidor central
 * @param directory	Directori de configuració
 * @retval -1 		Error al obrir el fitxer
 * @retval 0		OK
 */
int readConfig(char* name, unsigned int* timeClean, char** ip, unsigned short* port, char** directory);
#pragma once

#include <unistd.h>
#include <fcntl.h>		// O_RDONLY
#include <stdlib.h>		// realloc
#include <stdbool.h>	// bool
#include <string.h>		// strlen

#define DESCRIPTOR_SCREEN 1
#define DESCRIPTOR_ERROR 2

/**
 * Llegeix del FileDescriptor fins trobar el delimitador
 * /!\ El delimitador no es guarda al buffer, però sí s'elimina del FileDescriptor /!\
 * /!\ Cal fer el free del buffer /!\
 * @param fd		FileDescriptor on agafar les dades
 * @param buffer	Array on guardar la informació (amb '\0')
 * @param delimiter	Caracter que marca quan parar
 * @return			Caracters obtninguts (sense contar '\0')
 */
size_t readUntil(int fd, char **buffer, char delimiter);

/**
 * Llegeix del File Descriptor tranforman-lo a enter i para a delimiter (agafant-lo)
 * /!\ El delimitador no es guarda al buffer, però sí s'elimina del FileDescriptor /!\
 * @param fd 		File Desriptor a consultar
 * @param stop 		Caracter on s'ha parat la conversió (NULL si no es vol guardar)
 * @return			Enter trobat (0 si res)
 */
int readInteger(int fd, char *stop);

/**
 * Llegeix un fitxer de configuració, guardant la informació a las variables
 * /!\ Important fer els frees dels char* /!\
 * @param name		Ruta del fitxer de configuració
 * @param ip		IP del servidor central
 * @param port		Port del servidor central
 * @param directory	Directori de configuració
 * @retval -1 		Error al obrir el fitxer
 * @retval 0		OK
 */
int readConfig(char* name, char** ip, unsigned short* port, char** directory);
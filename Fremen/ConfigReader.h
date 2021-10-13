#pragma once

#include <fcntl.h>
#include <stdlib.h>

#define DESCRIPTOR_SCREEN 1
#define DESCRIPTOR_ERROR 2

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
int readConfig(char* name, unsigned int** timeClean, char** ip, unsigned short** port, char** directory);
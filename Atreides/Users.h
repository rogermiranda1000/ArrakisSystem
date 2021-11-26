/**
 * Gestiona els usuaris del sistema
 * @author Roger Miranda
 * @author Carles Roch
 */

 #pragma once

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include "ConfigReader.h" // readUntil/readInteger

#define STATIC_STRING_LEN(str) (sizeof(str)/sizeof(char))

typedef struct{
    char *login;
	int postal;
} User;

typedef struct {
	User *users; // l'ordre és la ID (l'usuari de la posició 0 té l'ID 0, el de la posició 1 la ID 1...)
	size_t len;
} Users;

/**
 * Carrega a la variable interna els usuaris
 * @param path	Ruta on es troba el fitxer d'usuaris
 * @retval 0	Tot OK
 * @retval -1	No s'ha actualitzat la variable (error obrint el fitxer)
 * @retval -2	Error llegint el fitxer
 */
int loadUsersFile(char *path);

/**
 * Persisteix la variable interna dels usuaris, destruint-la
 * @param path Ruta on es troba el fitxer d'usuaris
 * @return 0 si OK, <0 si error
 */
int saveUsersFile(char *path);

/**
 * Consulta la id del usuari, si no existeix el crea
 * @param login 	Login del usuari
 * @param postal	Codi postal del usuari
 * @return          Id asignada al usuari (-1 si error de creació de fitxer,
 * 					-2 si error de lectura de fitxer)
 */
int newLogin(char *login, char *postal);

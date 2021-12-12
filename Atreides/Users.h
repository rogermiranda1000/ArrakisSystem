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
#include <pthread.h>		// pthread_mutex_lock/pthread_mutex_unlock
#include "ConfigReader.h"	// readUntil/readInteger
#include "Comunication.h"	// SearchResults

#define STATIC_STRING_LEN(str) (sizeof(str)/sizeof(char))

typedef struct{
    char *login;
	int postal;
    char *image_type;
} User;

typedef struct {
	User *users; // l'ordre és la ID (l'usuari de la posició 0 té l'ID 0, el de la posició 1 la ID 1...)
	size_t len;
} Users;

/**
 * Elimina tots els usuaris de la memoria interna i destrueix el mutex
 */
void terminateUsers();

/**
 * Carrega a la variable interna els usuaris
 * @param path	Ruta on es troba el fitxer d'usuaris
 * @retval 0	Tot OK
 * @retval -1	No s'ha actualitzat la variable (error obrint el fitxer)
 * @retval -2	Error llegint el fitxer
 */
int loadUsersFile(char *path);

/**
 * Persisteix la variable interna dels usuaris
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

/**
 * Obtè un usuari a partir de la seva ID
 * @param id	ID del usuari
 *				/!\ Ha de ser menor a usuaris.len /!\
 * @return		Usuari guardat amb aquella ID
 */
User getUser(int id);

/**
 * Obtè tots els usuaris amb el codi postal donat
 * /!\ S'han d'alliberar els recursos /!\
 * @param postal 	Codi postal a cercar
 * @return			Usuaris de la zona
 */
SearchResults getUsersByPostal(int postal);

/**
 * Cambia la imatge d'un usuari
 * @param user_id	Usuari a cambiar-li la imatge
 * @param image		Nova imatge (NULL per eliminar-la)
 */
void setImage(int user_id, char *image);
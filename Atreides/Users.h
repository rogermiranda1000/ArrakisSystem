/**
 * Gestiona els usuaris del sistema
 * @author Carles Roch
 * @author Roger Miranda
 */

 #pragma once

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

#define STATIC_STRING_LEN(str) (sizeof(str)/sizeof(char))

#define USERS_FILE "registeredUsers.txt"

typedef struct{
    int id;
    char *login;
}User;

/**
 * Consulta la id del usuario y lo crea si no existe
 * /!\ Debe ser liberada llamando regExDestroy /!\
 * @param login 	Login del usuario
 * @param codigo	Codigo postal del usuario
 * @return          Id asignado al usuario.
 */
int newLogin(char* login);

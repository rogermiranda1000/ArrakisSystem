/**
 * Permite la ejecución de programas en Linux
 * @author Roger Miranda
 */

#pragma once

#define _GNU_SOURCE		// execvpe
#include <unistd.h>
#include <sys/types.h> 	// pid_t
#include <sys/wait.h> 	// waitpid
#include <stdlib.h> 	// exit/EXIT_FAILURE
#include <string.h>		// strcpy/strcat

#define FORK_SON 			0
#define CREATE_FORK_ERROR 	-1
#define WAIT_FORK_ERROR 	-2

#define ARGV_MAX_LEN		20
#define CMD_MAX_LEN			500

#define LINUX_PROGRAM_DIR	"/usr/bin/"

/**
 * Ejecuta el comando 'cmd' con los argumentos 'argv'
 * @param cmd			Comando a ejecutar
 * @param argv			Array de argumentos a adjuntar con el comando (comando incluido)
 *						/!\ Tiene que terminar en NULL /!\
 * @param envp			Array de variables de usuario
 * @param freeMallocs	Función para aliberar la memoria del padre al hacer el fork
 * @return		Retorno de la ejecución del hijo, o -1 si error al crear el fork
 */
int executeProgram(char *cmd, char *argv[], char *envp[], void (*freeMallocs)());

/**
 * Ejecuta el comando 'cmd' con los argumentos separados por espacio
 * /!\ Solo 1 espacio, y no puede haber espacios en el inicio/final del comando /!\
 * @param cmd			Comando a ejecutar
 * @param envp			Array de variables de usuario
 * @param freeMallocs	Función para aliberar la memoria del padre al hacer el fork
 * @return		Retorno de la ejecución del hijo, o -1 si error al crear el fork
 */
int executeProgramLine(char **cmd, char *envp[], void (*freeMallocs)());
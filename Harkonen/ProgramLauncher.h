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
#include <stdbool.h>	// bool
#include <stdarg.h>		// va_list

#define FORK_SON 			0
#define CREATE_FORK_ERROR 	-1
#define WAIT_FORK_ERROR 	-2

#define ARGV_MAX_LEN		20
#define CMD_MAX_LEN			500

#define LINUX_PROGRAM_DIR	"/usr/bin/"

typedef struct {
	int fd;			// FileDescriptor de la pipe
	bool type;		// Tipus de pipe (PIPE_LECTURA/PIPE_ESCRIPTURA)
} Pipe;

typedef struct {
	pid_t other_pid;	// en el fill indica el PID del pare, en el pare el PID del fill
	size_t pipe_num;	// nº de pipes
	Pipe *pipes;		// pipes
} ForkedPipeInfo;

/**
 * Obtè el FileDescriptor d'un ForkedPipeInfo
 * @param info	ForkedPipeInfo on extreure el FileDescriptor
 * @param index	Index del FileDescriptor
 * @return		FileDescriptor d'info en la posició index
 */
int fdPipeInfo(ForkedPipeInfo info, size_t index);

/**
 * Tanca els FileDescriptors i fa els frees dels punters
 * @param info	ForkedPipeInfo a destruir
 */
void freeForkedPipeInfo(ForkedPipeInfo *info);




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


/**
 * Ejecuta el comando 'cmd' con los argumentos 'argv' enviando todo el texto por la pipe dentro de pipe_info
 * @param pipe_info		Pipe donde leer el texto que el comando muestra por pantalla
 *						/!\ Tiene que liberarse con freeForkedPipeInfo() /!\
 * @param cmd			Comando a ejecutar
 * @param argv			Array de argumentos a adjuntar con el comando (comando incluido)
 *						/!\ Tiene que terminar en NULL /!\
 * @param envp			Array de variables de usuario
 * @param freeMallocs	Función para aliberar la memoria del padre al hacer el fork
 * @return		Retorno de la ejecución del hijo, o -1 si error al crear el fork
 */
int executeProgramWithPipe(ForkedPipeInfo *pipe_info, char *cmd, char *argv[], char *envp[], void (*freeMallocs)());

/**
 * Ejecuta el comando 'cmd' con los argumentos separados por espacio
 * /!\ Solo 1 espacio, y no puede haber espacios en el inicio/final del comando /!\
 * @param pipe_info		Pipe donde leer el texto que el comando muestra por pantalla
 *						/!\ Tiene que liberarse con freeForkedPipeInfo() /!\
 * @param cmd			Comando a ejecutar
 * @param envp			Array de variables de usuario
 * @param freeMallocs	Función para aliberar la memoria del padre al hacer el fork
 * @return		Retorno de la ejecución del hijo, o -1 si error al crear el fork
 */
int executeProgramLineWithPipe(ForkedPipeInfo *pipe_info, char **cmd, char *envp[], void (*freeMallocs)());
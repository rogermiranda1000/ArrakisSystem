#pragma once

#include "RegExSearcher.h"
#include "ProgramLauncher.h"
#include "ConfigReader.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h> // exit

#define DESCRIPTOR_SCREEN 1
#define DESCRIPTOR_ERROR 2

#define STATIC_STRING_LEN(str) (sizeof(str)/sizeof(char))

#define ERROR_FILE "Error al llegir el fitxer de configuració\n"
#define ERROR_ARGS "Has d'indicar el nom del fitxer de configuració\n"
#define ERROR_MALLOC "Malloc error, torna a intentar-ho quan Matagalls no estigui tant saturat\n"

typedef enum {
	WAITING,	// en un read()
	EXIT,		// ha saltat interrupció estant en estat 'RUNNING'; apaga quan pugui
	RUNNING		// està treballant
} Status;

int main(int argc, char *argv[], char *envp[]);
void intHandler(int signum);

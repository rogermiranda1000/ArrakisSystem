#pragma once

#include "Comunication.h" // concat
#include "ProgramLauncher.h"

#define CLEANING_FILES	"Eliminant fitxers...\n"

/**
 * Inicia el netejador de fitxers
 * /!\ directory ha d'estar reservat dinamicament, i el que el crida és responsable de fer el free /!\
 * @param directory	Directori on borrar els fitxers
 * @param delay 	Delay (en minuts) a dur a terme la nateja
 */
void startCleaner(char *directory, unsigned int delay, char *envp[], void (*freeMallocs)());
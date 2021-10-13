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

int main(int argc, char *argv[], char *envp[]);
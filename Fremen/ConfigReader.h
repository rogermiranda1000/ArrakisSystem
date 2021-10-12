#pragma once

#include "ProgramLauncher.h"
#define DESCRIPTOR_SCREEN 1
#define DESCRIPTOR_ERROR 2

#define STATIC_STRING_LEN(str) (sizeof(str)/sizeof(char))

int readConfig(char* name, unsigned int* timeClean, char* ip, unsigned int* port, char* directory);
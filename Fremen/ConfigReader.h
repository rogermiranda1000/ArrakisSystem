#pragma once

#include <fcntl.h>
#include <stdlib.h>
#include "ProgramLauncher.h"
#define DESCRIPTOR_SCREEN 1
#define DESCRIPTOR_ERROR 2

int readConfig(char* name, unsigned int* timeClean, char** ip, unsigned int* port, char** directory);
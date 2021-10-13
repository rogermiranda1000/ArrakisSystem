#pragma once

#include "RegExSearcher.h"
#include "ProgramLauncher.h"
#include <string.h>
#include <stdbool.h>

#define DESCRIPTOR_SCREEN 1
#define DESCRIPTOR_ERROR 2

#define STATIC_STRING_LEN(str) (sizeof(str)/sizeof(char))

typedef enum {
	WAITING,	// en un read()
	EXIT,		// ha saltat interrupció estant en estat 'RUNNING'; apaga quan pugui
	RUNNING		// està treballant
} Status;

int main(int argc, char *argv[], char *envp[]);
void intHandler(int signum);
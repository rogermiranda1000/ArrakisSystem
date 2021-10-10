#pragma once

#include "RegExSearcher.h"
#include "ProgramLauncher.h"
#include <string.h>
#include <stdbool.h>

#define DESCRIPTOR_SCREEN 1
#define DESCRIPTOR_ERROR 2

#define STATIC_STRING_LEN(str) (sizeof(str)/sizeof(char))

int main(int argc, char *argv[], char *envp[]);
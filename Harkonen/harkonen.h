#pragma once

#include "ProgramLauncher.h"

#include <string.h>
#include <stdbool.h>
#include <stdlib.h> // exit
#include <unistd.h>
#include <stdio.h>	// snprintf
#include <time.h>	// time(NULL)
#include <fcntl.h>	// socket non-blocking mode
#include <signal.h>	// kill

#define DESCRIPTOR_SCREEN 1
#define DESCRIPTOR_ERROR 2

#define REGEX_ARRAKIS			"Atreides|Fremen"

#define STATIC_STRING_LEN(str) (sizeof(str)/sizeof(char))

#define MSG_INIT		"Starting Harkonen\n"
#define MSG_SEARCH		"Scanning pids\n"
#define MSG_NOT_FOUND	"Arrakis' processes not found\n"
#define MSG_EXIT		"Exiting...\n"

#define ERROR_ARGS	"Please enter one argument stating the number of seconds between deletions.\n\n"
#define ERROR_KILL	"Couldn't kill the process\n"

int main(int argc, char *argv[], char *envp[]);
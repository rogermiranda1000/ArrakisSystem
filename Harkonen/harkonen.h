#pragma once

#include "ProgramLauncher.h"
#include "Configreader.h"

#include <string.h>
#include <stdbool.h>
#include <stdlib.h> // exit
#include <unistd.h>

#define DESCRIPTOR_SCREEN 1
#define DESCRIPTOR_ERROR 2

#define STATIC_STRING_LEN(str) (sizeof(str)/sizeof(char))

#define MSG_INIT    "Starting Harkonen\n"
#define MSG_SEARCH  "Scanning pids\n"
#define MSG_KILL    "Killing pid "
#define MSG_EXIT    "Exiting...\n"

#define ERR_ARGS    "Please enter one argument stating the number of seconds between deletions.\n\n"

#define CMD_WHO     "whoami"

int main(int argc, char *argv[], char *envp[]);
#pragma once

#include "ProgramLauncher.h"

#include <string.h>
#include <stdbool.h>
#include <stdlib.h> // exit

#define DESCRIPTOR_SCREEN 1
#define DESCRIPTOR_ERROR 2

#define STATIC_STRING_LEN(str) (sizeof(str)/sizeof(char))

#define MSG_INIT    "Starting Harkonen\n"
#define MSG_SEARCH  "Scanning pids\n"
#define MSG_KILL    "Killing pid %d"
#define MSG_EXIT    "Exiting...\n"

#define CMD_WHO     "whoami\n"
#define CMD_PIDS    "ps -u %S --no-headers\n"
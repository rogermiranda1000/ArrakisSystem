#pragma once

#include <pthread.h>	// pthread_create/pthread_join/pthread_t
#include <stdlib.h>		// malloc/free
#include <string.h>		// memccpy
#include <stdbool.h>	// true/false/bool
#include <signal.h>		// signals

/**
 * Uneix els threads que ja han acabat i els elimina de la llista
 */
void gc();

/**
 * Força l'aturada de tots els threads oberts
 */
void terminateThreads();

/**
 * Crea un nou thread
 * @param function	Funció a cridar pel thread [ha de fer free d'args]
 * @param args		Arguments a entregar al nou thread
 * @param args_size Mida del contingut de 'args'
 * @return			Retorn del pthread_create
 */
int createThread(void *(*function)(void *), void *args, size_t args_size);
#pragma once

#include <pthread.h>	// pthread_create/pthread_join/pthread_t
#include <stdlib.h>		// malloc/free
#include <string.h>		// memccpy

/**
 * Força l'aturada de tots els threads oberts
 */
void terminateThreads();

/**
 * Crea un nou thread
 * @param function	Funció a cridar pel thread [ha de fer free d'args]
 * @param args		Arguments a entregar al nou thread [no pot contenir cap punter]
 * @param args_size Mida del contingut de 'args'
 * @return			Retorn del pthread_create
 */
int createThread(void *(*function)(void *), void *args, size_t args_size);
#include "Threads.h"

typedef struct {
	void *(*function)(void *);
	void *args;
} Thread;

int num_threads = 0;
pthread_t *threads = NULL;

void terminateThreads() {
	while(num_threads > 0) {
		pthread_exit(&threads[num_threads-1]);
		num_threads--;
	}
	free(threads);
}

/**
 * Tots els threads apuntan a aquesta funció.
 * Al acabar-se l'execució s'allibera el thread de la linked list
 * @param args	Paràmetres a enviar a la funció del thread
 * @return		Retorn de la funció
 */
void *threadManager(void *args) {
	Thread thread_args = *((Thread*)args);
	free(args);
	void *r = (*thread_args.function)(thread_args.args);
	// TODO eliminar de la linked list
	return r;
}

int createThread(void *(*function)(void *), void *args, size_t args_size) {
	Thread *thread_args = (Thread*)malloc(sizeof(Thread));
	thread_args->function = function;
	thread_args->args = malloc(args_size);
	memcpy(thread_args->args, args, args_size);
	
	// TODO fer-ho en linked list
	threads = (pthread_t *)realloc(threads, sizeof(pthread_t)*(++num_threads));
	return pthread_create(&threads[num_threads-1], NULL, &threadManager, thread_args /* TODO la variable es destruirà abans de ser llegida? */);
}
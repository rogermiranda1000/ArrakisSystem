#include "Threads.h"

typedef struct {
	void *(*function)(void *);
	void *args;
} Thread;

struct _Node {
	pthread_t thread;
	struct _Node *next;
	bool hasEnded;
};
typedef struct _Node Node;

Node *head = NULL;

/**
 * Afegeix un thread a la llista
 * @return 		Punter a la variable thread de la llista
 */
pthread_t *getNewNode() {
	Node *node = (Node*)malloc(sizeof(Node));
	node->hasEnded = false;
	node->next = head;
	head = node; // TODO protegir
	return &node->thread;
}

/**
 * Estableix un thread com acabat
 * @param thread	Valor de pthread_self()
 * @retval 0		OK
 * @retval -1		No exiteix a la linked list
 */
int setThreadEnded(pthread_t thread) {
	// TODO protegir [que pasa si mentre s'executa terminateThreads() algu acaba?]
	Node *current = head;
	while(current != NULL) {
		if (current->thread == thread) {
			current->hasEnded = true;
			return 0;
		}
		current = current->next;
	}
	return -1;
}

/**
 * Elimina un node de la llista
 * @param node	Node a eliminar
 * @param pre	Node previ al node a eliminar [NULL si head]
 */
void removeFromList(Node *node, Node *pre) {
	if (pre == NULL) head = node->next;
	else pre->next = node->next;
	free(node);
}

void gc() {
	Node *current = head, *pre = NULL, *tmp;
	while(current != NULL) {
		tmp = current->next;
		if (current->hasEnded) {
			pthread_join(current->thread, NULL);
			removeFromList(current, pre);
		}
		else pre = current;
		current = tmp;
	}
}

void terminateThreads() {
	gc();
	
	Node *current = head, *next;
	head = NULL; // TODO protegir (entre el 'current = head' i 'head = NULL')
	while(current != NULL) {
		pthread_cancel(current->thread);
		pthread_join(current->thread, NULL);
		
		next = current->next;
		free(current);
		
		current = next;
	}
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
	setThreadEnded(pthread_self());
	return r;
}

int createThread(void *(*function)(void *), void *args, size_t args_size) {
	Thread *thread_args = (Thread*)malloc(sizeof(Thread)); // threadManager fa el free
	thread_args->function = function;
	thread_args->args = malloc(args_size);
	memcpy(thread_args->args, args, args_size);
	
	return pthread_create(getNewNode(), NULL, &threadManager, thread_args /* TODO la variable es destruirà abans de ser llegida? */);
}
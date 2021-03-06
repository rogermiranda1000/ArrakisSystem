#include "Threads.h"

struct _Node {
	pthread_t thread;
	struct _Node *next;
	bool hasEnded;
};
typedef struct _Node Node;

typedef struct {
	void *(*function)(void *);
	void *args;
	Node *node;
} Thread;

/**
 * LinkedList
 */
Node *head = NULL;

/**
 * Afegeix un thread a la llista
 * @return 		Punter a la variable de la llista
 */
Node *getNewNode() {
	Node *node = (Node*)malloc(sizeof(Node));
	node->hasEnded = false;
	
	node->next = head;
	head = node;
	
	return node;
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
	head = NULL;
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
	thread_args.node->hasEnded = true; // ja ha acabat
	return r;
}

int createThread(void *(*function)(void *), void *args, size_t args_size) {
	Thread *thread_args = (Thread*)malloc(sizeof(Thread)); // threadManager fa el free
	thread_args->function = function;
	thread_args->args = malloc(args_size);
	thread_args->node = getNewNode();
	memcpy(thread_args->args, args, args_size);
	
	return pthread_create(&thread_args->node->thread, NULL, &threadManager, thread_args);
}
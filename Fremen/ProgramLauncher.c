#include "ProgramLauncher.h"

/**
 * Crea una copia del codi. El fill crida son_call i finalitza la execució,
 * mentre que al pare simplement se li retorna el PID del fill.
 * Si es vol cridar una funció en cas d'error es pot utilitzar error_call.
 * /!\ El pare ha d'esperar al fill amb waitpid /!\
 *
 * @param	son_call		Funció que es cridarà en el cas del fill
 * @param	error_call		Funció que es cridarà en cas d'error
 * @return	0 si és el fill, -1 si error, si no és el PID del fill cap al pare
 *			[en principi, només es retornarà el PID del fill al pare;
 *			com a màxim retornarà -1 si error_call no fa exit]
 */
#define FORK_CODE(son_call, error_call) ({				\
	int r = fork();										\
	if (r == -1) error_call;							\
	else if (r == FORK_SON) {							\
		/* fill -> cridar a la funció i sortir */		\
		son_call;										\
		exit(0);										\
	}													\
														\
	r; /* return */										\
})

#define STATIC_STRING_LEN(str) (sizeof(str)/sizeof(char))

int executeProgram(char *cmd, char *argv[], char *envp[]) {
	int r = CREATE_FORK_ERROR;
	
	// obtiene la ruta absoluta
	char *absolute_cmd = (char*)malloc(sizeof(char)*(strlen(cmd) + 1 + STATIC_STRING_LEN(LINUX_PROGRAM_DIR)));
	strcpy(absolute_cmd, LINUX_PROGRAM_DIR);
	strcat(absolute_cmd, cmd);
	
	pid_t son = FORK_CODE(exit(execvpe(absolute_cmd, argv, envp)),);
	if (son != CREATE_FORK_ERROR) {
		if (waitpid(son, &r, WUNTRACED | WCONTINUED) == -1) r = WAIT_FORK_ERROR;
	}
	
	free(absolute_cmd);
	return r;
}

int executeProgramLine(char *cmd, char *envp[]) {
	int r, size = 0;
	char **argv = NULL;
	char *iter = cmd;
	
	argv = (char**)malloc(sizeof(char*));
	argv[size++] = iter;
	
	while (*iter != '\0') {
		if (*iter == ' ') {
			*iter = '\0';
			argv = (char**)realloc(argv, sizeof(char*)*(size+1));
			argv[size++] = iter+1;
		}
		iter++;
	}
	
	// l'últim element ha d'acabar en NULL
	argv = (char**)realloc(argv, sizeof(char*)*(size+1));
	argv[size++] = NULL;
	
	r = executeProgram(argv[0], argv, envp);
	
	free(argv);
	
	return r;
}
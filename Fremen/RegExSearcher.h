/**
 * Permite una interacción más senzilla con la libreria RegEx
 * @author Roger Miranda
 */

#pragma once

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>	// bool
#include <string.h>		// strcpy / strlen
#include <stdarg.h>		// argumento '...'
#include <regex.h>

#define MALLOC_ERROR -1

/**
 * Contiene una expresión creada, y información de control.
 * Evita estar creando constantemente la misma expresión.
 */
typedef struct {
	regex_t re;
	regmatch_t *rm;
	bool valid;
} RegEx;

/**
 * Inicia una expresión regular.
 * @param regex 		Expresión en texto
 * @param ignore_case 	Si el regex deberia ser caseinsensitive
 * @return La expresión creada (si no se pudo crear, se muestra el error y el parámetro valid vale false). Al terminar hay que llamar destroyRegEx().
 */
RegEx regExInit(char *regex, bool ignore_case);

/**
 * Retorna el número de coincidencias que guarda la expresión.
 * @param regex Expresión a consultar
 * @return Coincidencias guardadas
 */
int regExMatchesSize(RegEx *regex);

/**
 * Destruye una exprexión regular
 * @param regex RegEx a destruir
 */
void regExDestroy(RegEx *regex);

/**
 * Busca una coincidencia de la expresión.
 * @param regex Expresión a buscar
 * @param line Texto donde buscar
 * @param matches Coincidencias
 * @return Éxito (EXIT_SUCCESS), fallo (EXIT_FAILURE), sin coincidencia (REG_NOMATCH), o sin memoria (MALLOC_ERROR)
 */
int regExSearch(RegEx *regex, char *line, char ***matches);

/**
 * Busca una coincidencia de la expresión, pero con un estilo similar al de la función scanf.
 * @param regex Expresión a buscar
 * @param line Texto donde buscar
 * @param ... Variables de tipo char*. Debe haber tantas como resultado de 'regExMatchesSize(regex)'.
 * @return Éxito (EXIT_SUCCESS), fallo (EXIT_FAILURE), sin coincidencia (REG_NOMATCH), o sin memoria (MALLOC_ERROR)
 */
int regExGet(RegEx *regex, char *line, ...);
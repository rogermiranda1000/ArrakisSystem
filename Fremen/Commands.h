#pragma once

#include "RegExSearcher.h"

typedef enum {
	LOGIN,
	LOGOUT,
	SEARCH,
	PHOTO,
	SEND,
	
	NO_MATCH,
	ERROR
} CommandResult;

/**
 * Prepara les variables per la cerca de comandes
 * /!\ No es pot cridar dos cops sense abans haver cridat freeCommands() /!\
 */
void initCommands();

/**
 * Busca segons l'entrada si hi ha algun resultat en les comandes.
 * De ser el cas, guarda la informació a output
 * /!\ Si no ha retornat NO_MATCH ni ERROR s'ha de cridar freeCommand() /!\
 * @param input[in] 	Text a fer la cerca
 * @param output[out]	Resultat de la cerca
 * @retval	NO_MATCH	Cap comanda
 * @retval	ERROR		Malloc error
 * @return				Comanda executada
 */
CommandResult searchCommand(char *input, char ***output);

/**
 * Allibera el resultat de searchCommand()
 * @param result[in]	Valor retornat a searchCommand()
 * @param output[in]	Resultat retornat a searchCommand()
 */
void freeCommand(CommandResult result, char ***output);

/**
 * Allibera tota memòria reservada per initCommands()
 */
void freeCommands();
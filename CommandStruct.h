#ifndef COMMANDSTRUCT_H_INCLUDED
#define COMMANDSTRUCT_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "Defines.h"

/**
 * @brief structure that contains all the information needed for the execution of its command attribute :
 *  -next is the next Commande that should be executed
 *  -prev is the previous command that has been executed
 *  -piped is 1 if the command is connected through a pipe to next
 *  -anded is 1 if the command was preceded by &&
 *  -ored is 1 if the command was preceded by ||
 *  -ncO is 1 or 2 if the command is followed by > or >>
 *  -nCI is 1 or 2 if the command is followed by < or <<
 *  -irarg = input redirection argument, name of a file
 *  -orarg = output redirection argument, name of a file
 *  -zombie is 1 if the command was preceded by &
 *  -end_status is 1 is the command ended properly
 *
 */
typedef struct Commande Commande;
struct Commande{

    Commande* next;
    Commande* prev;

    char** command;
    int built_in;

    int piped;

    int nCO;
    int nCI;
    char* orarg;
    char* irarg;

    int anded;
    int ored;

    int zombie;
    int end_status;
};

Commande* createCommande();

/**
 * @param int* p_index is set to 0, it should be the size of current_command->command when it's given
 * @return  a pointer to the next Commande structure of current_command
 *
 */
Commande* next(Commande* current_command,int* p_index);

void destroyCommande(Commande* cmd);

#endif // COMMANDSTRUCT_H_INCLUDED

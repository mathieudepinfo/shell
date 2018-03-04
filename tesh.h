#ifndef TESH_H_INCLUDED
#define TESH_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <dlfcn.h>

#include "Defines.h"
#include "CommandStruct.h"
#include "StringManagement.h"

/**
 * A fork function with error management
 */
pid_t checkedFork();

int isBuiltIn(const char* command);

/**
 * @brief fill the Commande structure with the data of the string command
 * @return 0 if it ends properly
 */
int parsing(Commande* current_command,const char* commande);

/**
 * @brief built in implementation of cd if the given path starts with "/" it will be considered as absolute
 * otherwise it will be relative to the current directory
 * @return 0 if it succeeds, -1 otherwise
 */
int cd(const char* new_directory);

/**
 * Put pid in foreground
 */
int fg(pid_t pid);
/**
 * @brief function that executes a command by calling execvp
 * @param cmd, the current command of cmd which will be executed
 *
 */
int execute(Commande* cmd,char* hostname,char* current_directory);

/**
 *@return return cmd->next, it also destroy cmd if cmd->next is NULL
 */
Commande* following(Commande* cmd);

/**
 * @brief create a string with the good prompt for the tesh in interactive mode
 * @return a pointer to the prompt that should be freed later
 */
char* makeprompt(const char* user,const char* hostname,const char* current_directory);
#endif // TESH_H_INCLUDED

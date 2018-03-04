#ifndef STRINGMANAGEMENT_H_INCLUDED
#define STRINGMANAGEMENT_H_INCLUDED

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "Defines.h"
/*flush and clean can be found on https://openclassrooms.com */
/**
 * @brief function that empties stdin
 */
void flush();

/**
 * @brief function that cleans a char* filled with fgets cf StackOverflow
 */
void clean(char* chaine);

/**
 * @brief function that splits a string when it sees the char sep
 * @param chain, the string to split
 * @param sep the separator
 * @return an array of strings which ends by a NULL pointer
 */
char** split(const char* chaine,char sep);

/**
 * Return 1 if str is in stringArray, 0 otherwise
 */
int contains(int len,char** stringArray, const char* str);
#endif // STRINGMANAGEMENT_H_INCLUDED

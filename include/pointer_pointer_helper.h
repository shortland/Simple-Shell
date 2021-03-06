#ifndef POINTER_POINTER_HELPER_H
#define POINTER_POINTER_HELPER_H

#include <stdlib.h>
#include <string.h>

#include "debug.h"

void pointer_pointer_debug(char **array, int length);

char **pointer_pointer_merge(char **ptr1, int len1, char **ptr2, int len2);

char **pointer_pointer_dup(char **array);

#endif

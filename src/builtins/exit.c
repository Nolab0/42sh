#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils/alloc.h>
#include <utils/vec.h>

#include "builtin.h"

int builtin_exit(char *args)
{
    if (strcmp(args, "") == 0)
        return 0;
    int code = strtol(args, NULL, 10);
    return code % 256;
}
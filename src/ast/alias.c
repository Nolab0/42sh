#include "ast.h"

#define NONE 0
#define SIMPLE 1
#define DOUBLE 2

#define NAME 1
#define VALUE 2


// FILE NOT COMPILING BECAUSE NOT USED

static char *rep_alias(char *str, int start, char *name, char *value)
{
    char *new = strdup(str);
    free(str);

    char *unalias = strdup("unalias ");
    strcat(unalias, name);
    char *all = strdup("unalias ");
    strcat(all, " -a");
    char *first = strstr(new, unalias);
    char *second = strstr(new, all);
    char *max = NULL;
    if (!first)
        max = second;
    if (!second)
        max = first;
    if (first && second)
        max = first < second ? first : second;
    if (!max)
        max = str + strlen(str);

    char *sub;
    while ((sub = strstr(new, name)) != NULL && sub < max)
    {
        char *before = strndup(new, sub - new);
        char *after = strdup(sub + strlen(name));
    }
    return new;
}

char *replace_aliases(char *str)
{
    char *new = strdup(str);

    int i = 0;
    int context = NONE;
    int step = NONE;
    struct vec *name = vec_init();
    struct vec *value = vec_init();
    int save = 0;
    while (str[i] != 0)
    {
        if (str[i] == '\"')
        {
            if (context == NONE)
                context = DOUBLE;
            else if (context == DOUBLE)
                context = NONE;
        }
        if (str[i] == '\'')
        {
            if (context == NONE)
                context = SIMPLE;
            else if (context == SIMPLE)
                context = NONE;
            continue;
        }

        if (step == NAME)
        {
            if (str[i] == '=' && context == NONE)
            {
                step = VALUE;
            }
            else
            {
                if (str[i] == ' ' && context == NONE)
                    break;
                vec_push(name, str[i]);
            }
        }
        else if (step == VALUE)
        {
            if (str[i] == ' ' && context == NONE)
            {
                step = NONE;
                str = rep_alias(str, i, vec_cstring(name), vec_cstring(value));
                i = 0;
                continue;
            }
            vec_push(value, str[i]);
        }
        else if (context == NONE && strncmp(str + i, "alias ", 6) == 0 && step == NONE)
        {
            save = i;
            int j = i + 6;
            while (str[j] == ' ')
                j++;
            i = j - 1;
            step = NAME;
        }
        i++;
    }
    vec_destroy(name);
    vec_destroy(value);
    free(name);
    free(value);
    if (step != NONE)
    {
        free(new);
        return NULL;
    }
    return new;
}

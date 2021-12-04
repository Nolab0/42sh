#include <string.h>
#include <utils/alloc.h>
#include <utils/utils.h>
#include <stdio.h>

#include "ast.h"

struct list *vars;

char *my_strstr(char *str, char *var)
{
    for (int i = 0; str[i] != 0; i++)
    {
        if (str[i] == var[0])
        {
            int j = 0;
            for (; var[j] != 0 && str[i + j] != 0; j++)
            {
                if (var[j] != str[i + j])
                    break;
            }
            if (var[j] == 0 && (str[i + j] == 0 || is_separator(str[i + j]) || str[i + j] == '\"'))
                return str + i;
            else if (var[j] == 0)
            {
                if (str[i + j] == 0)
                    return str + i;
                if (str[i + j - 1] == '}')
                    return str + i;
            }
        }
    }
    return NULL;
}

char *replace_vars(char *str, char *var, char *replace)
{
    if (var == NULL)
        return strdup(str);
    char *substring = NULL;
    size_t to_copy = 0;
    char *cmd = strdup(str);
    while ((substring = my_strstr(cmd, var)) != NULL)
    {
        to_copy = substring - cmd; // determine length before substring
        char *before = strndup(cmd, to_copy);
        char *after = strdup(substring + strlen(var));
        char *tmp =
            zalloc(sizeof(char)
                   * (strlen(before) + strlen(replace) + strlen(after) + 1));
        sprintf(tmp, "%s%s%s", before, replace, after);
        free(cmd);
        free(before);
        free(after);
        cmd = tmp;
    }
    return cmd;
}

void expand_vars(char *str)
{
    str++;
    return;
}

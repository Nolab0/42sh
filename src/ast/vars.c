#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <utils/alloc.h>
#include <utils/utils.h>

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
            if (var[j] == 0
                && (str[i + j] == 0 || is_separator(str[i + j])
                    || str[i + j] == '\"' || str[i + j] == '='))
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

char *expand_vars(char *str)
{
    struct list *cur = vars;
    char *new = strdup(str);
    while (cur)
    {
        char *tmp = new;
        char *var = strdup(cur->name);
        char *newvar = zalloc(sizeof(char) * strlen(var) + 4);
        sprintf(newvar, "\"$%s\"", var);
        new = replace_vars(new, newvar, cur->value);
        free(tmp);
        free(newvar);
        newvar = zalloc(sizeof(char) * strlen(var) + 6);
        sprintf(newvar, "\"${%s}\"", var);
        tmp = new;
        new = replace_vars(new, newvar, cur->value);
        free(newvar);
        free(var);
        free(tmp);
        cur = cur->next;
    }
    free(str);
    return new;
}

char *remove_quotes(char *str)
{
    int i = 0;
    int quote_type = 0;
    int index = 0;
    char *new = zalloc(sizeof(char) * (strlen(str) + 1));
    while (str[i] != 0)
    {
        if (str[i] == '\'' && (i == 0 || str[i - 1] != '\\'))
        {
            if (quote_type == 0)
            {
                quote_type = 1;
                i++;
                continue;
            }
            if (quote_type == 1)
            {
                quote_type = 0;
                i++;
                continue;
            }
        }
        if (str[i] == '\"' && (i == 0 || str[i - 1] != '\\'))
        {
            if (quote_type == 0)
            {
                quote_type = 2;
                i++;
                continue;
            }
            if (quote_type == 2)
            {
                quote_type = 0;
                i++;
                continue;
            }
        }
        new[index++] = str[i];
        i++;
    }
    free(str);
    return new;
}

char *escape_chars(char *str)
{
    int i = 0;
    int index = 0;
    char *new = zalloc(sizeof(char) * (strlen(str) + 1));
    while (str[i] != 0)
    {
        if (str[i] == '\\')
            i++;
        new[index++] = str[i];
        i++;
    }
    free(str);
    return new;
}

void add_var(struct list *new)
{
    if (!vars)
    {
        vars = new;
        return;
    }
    struct list *cur = vars;
    struct list *prev = NULL;
    while (cur && strcmp(cur->name, new->name) != 0)
    {
        prev = cur;
        cur = cur->next;
    }
    if (!cur)
    {
        prev->next = new;
        return;
    }
    if (!prev)
    {
        new->next = cur->next;
        vars = new;
        free_var(cur);
        return;
    }
    new->next = cur->next;
    free_var(cur);
    prev->next = new;
}

int is_var_assign(char *str)
{
    char *equal = strchr(str, '=');
    char *space = strchr(str, ' ');
    char *quote = strchr(str, '\"');
    char *squote = strchr(str, '\'');
    if (isdigit(str[0]) || !equal || (space && space < equal))
        return 0;
    if ((quote && quote < equal) || (squote && squote < equal))
        return 0;
    char *tmp = str;
    while (tmp != equal && isalnum(tmp[0]))
        tmp++;
    if (tmp != equal)
        return 0;

    struct list *var = zalloc(sizeof(struct list));
    var->name = strndup(str, equal - str);
    var->value = strdup(equal + 1);
    add_var(var);
    return 1;
}

int var_assign_special(char *str)
{
    char *equal = strchr(str, '=');
    char *before = strndup(str, equal - str);
    struct list *var = zalloc(sizeof(struct list));
    var->name = before;
    var->value = strdup(equal + 1);
    add_var(var);
    return 1;
}

char *build_var(char *name, char *value)
{
    char *new = zalloc(sizeof(char) * (strlen(name) + 1 + strlen(value) + 1));
    sprintf(new, "%s=%s", name, value);
    return new;
}

char *my_itoa(int n)
{
    char *new = zalloc(sizeof(char) * 20); // more than max int
    sprintf(new, "%d", n);
    return new;
}

void set_special_vars(void)
{
    int pid = getpid();
    char *value = my_itoa(pid);
    char *var = build_var("$", value);
    var_assign_special(var);

    free(var);
    free(value);

    var = build_var("?", "0");
    var_assign_special(var);
    free(var);
}

char *remove_vars(char *str)
{
    int i = 0;
    int index = 0;
    char *new = zalloc(sizeof(char) * (strlen(str) + 1));
    int status = 0;
    while (str[i] != 0)
    {
        if (str[i] == '\'')
        {
            if (status != -1)
                status = -1;
            else if (status == -1)
                status = 0;
        }
        if (status != -1)
        {
            if (str[i] == '"' && status == 1)
                status = 0;
            if (str[i] == '}' && status == 2)
                status = 0;

            if (str[i] == '$' && str[i + 1] != '='
                && (i == 0 || str[i - 1] != '\\'))
            {
                if (str[i + 1] == '{')
                    status = 2;
                else
                    status = 1;
            }
        }
        if (status == 0 || status == -1)
            new[index++] = str[i];
        i++;
    }
    free(str);
    return new;
}

#include <builtins/builtin.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utils/alloc.h>
#include <utils/utils.h>
#include <utils/vec.h>

#include "ast.h"
#include "redirection.h"

/**
 * \brief The number of builtins commands
 */
#define BLT_NB 1

/**
 * \brief The number of redirection operators
 */
#define REDIR_NB 7

/**
 * \brief Get the command name from a string.
 * @param cmd: the string containing the command
 * @param i: the index where the commands stops
 * @return: the command, allocated
 */
static char *getcmdname(char *cmd, int *i)
{
    while (cmd[*i] != 0 && !is_separator(cmd[*i]))
        (*i)++;
    return strndup(cmd, *i);
}

/**
 * \brief Split a string into an array in which the
 * first element is the command.
 * @param cmd: the string to spilit
 * @param size: the size of the returned array
 * @return: the splited array
 */

static char **split_in_array(char *cmd, int *size)
{
    char **args = xmalloc(sizeof(char *) * strlen(cmd));
    int index = 0;
    int i = 0;
    while (cmd[i] != 0)
    {
        while (cmd[i] != 0 && cmd[i] == ' ')
            i++;
        index = i;
        while (cmd[i] != 0 && cmd[i] != ' ')
            i++;
        if (i != index)
            args[(*size)++] = strndup(cmd + index, i - index);
    }
    args[*size] = NULL;
    return args;
}

/**
 * \brief Execute a command in a sub-process
 * @param cmd: The command to execute
 * @return: return if the command fail or succeed
 */
static int fork_exec(char *cmd)
{
    int size = 0;
    char **args = split_in_array(cmd, &size);
    int pid = fork();
    if (pid == -1)
        errx(1, "Failed to fork\n");

    if (pid == 0)
    {
        if (execvp(args[0], args) == -1)
        {
            fprintf(stderr, "Command not found: '%s'\n", args[0]);
            for (int i = 0; i < size; i++)
                free(args[i]);
            free(args);
            exit(127);
        }
    }

    for (int i = 0; i < size; i++)
        free(args[i]);
    free(args);
    int wstatus;
    int cpid = waitpid(pid, &wstatus, 0);

    if (cpid == -1)
        errx(1, "Failed waiting for child\n%s", strerror(errno));
    return WEXITSTATUS(wstatus);
}

int cmd_exec(char *cmd)
{
    char *builtins[] = { "echo" };
    commands cmds[BLT_NB] = { &echo };

    int arg_index = 0;
    char *cmd_name = getcmdname(cmd, &arg_index);
    if (cmd_name[arg_index] == 0)
        arg_index--; // handle \0 for empty args
    int i = 0;
    while (i < BLT_NB)
    {
        if (strcmp(cmd_name, builtins[i]) == 0)
        {
            free(cmd_name);
            return cmds[i](cmd + arg_index + 1);
        }
        i++;
    }
    free(cmd_name);
    return fork_exec(cmd);
}

static int eval_pipe(struct ast *ast)
{
    int fds[2];

    if (pipe(fds) == -1)
        errx(1, "Failed to create pipe file descriptors.");

    int out = dup(STDOUT_FILENO);

    if (dup2(fds[1], STDOUT_FILENO) == -1)
        errx(1, "dup2 failed");
    ast_eval(ast->left);

    dup2(out, STDOUT_FILENO);
    close(out);

    int in = dup(STDIN_FILENO);

    if (dup2(fds[0], STDIN_FILENO) == -1)
        errx(1, "dup2 failed");
    close(fds[1]);
    int res = ast_eval(ast->right);

    dup2(in, STDIN_FILENO);
    close(in);
    close(fds[0]);
    close(fds[1]);

    return res;
}

int exec_redir(struct ast *ast)
{
    char *redirs_name[] = { ">", "<", ">>", ">&", ">|", "<&", "<>" };
    redirs_funcs redirs[REDIR_NB] = {
        &redir_simple_left,    &redir_simple_right, &redir_double_left,
        &redir_ampersand_left, &redir_simple_left,  &redir_ampersand_right,
        &redir_left_right
    };

    size_t i = 0;
    int fd = -1;
    if (!is_redirchar(ast->val->data[i])) // TODO: handle not digit case
        fd = ast->val->data[i++] - '0';
    while (is_redirchar(ast->val->data[i]))
        i++;
    if (fd == -1 && i > 2)
        return 2; // Not valid
    char *redir_mode = NULL;
    if (fd != -1)
        redir_mode = strndup(ast->val->data + 1, i - 1);
    else
        redir_mode = strndup(ast->val->data, i);
    while (isspace(ast->val->data[i])) // Skip spaces before WORD
        i++;
    char *right = strdup(ast->val->data + i); // Skip redir operator
    int return_code = -1;
    for (size_t index = 0; index < REDIR_NB; index++)
    {
        if (strcmp(redirs_name[index], redir_mode) == 0)
        {
            return_code = redirs[index](ast->left, fd, right);
            break;
        }
    }
    free(redir_mode);
    free(right);
    if (ast->right)
        return exec_redir(ast->right);
    return return_code;
}

static void set_var(char *var, struct ast *ast)
{
    if (ast == NULL)
        return;
    ast->var = strdup(var);
    set_var(var, ast->left);
    set_var(var, ast->right);
}

static void set_replace(char *value, struct ast *ast)
{
    if (ast == NULL)
        return;
    if (ast->replace)
        free(ast->replace);
    ast->replace = strdup(value);
    set_replace(value, ast->left);
    set_replace(value, ast->right);
}

static char *my_strstr(char *str, char *var)
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

static char *replace_vars(char *str, char *var, char *replace)
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

int ast_eval(struct ast *ast)
{
    if (!ast)
        return 0;
    if (ast->type == AST_OR)
    {
        int left = ast_eval(ast->left);
        if (left == 0 || !ast->right)
            return left;
        return ast_eval(ast->right);
    }
    if (ast->type == AST_ROOT)
    {
        int left = ast_eval(ast->left);
        if (!ast->right)
            return left;
        return ast_eval(ast->right);
    }
    else if (ast->type == AST_IF)
    {
        if (!ast_eval(ast->cond)) // true
            return ast_eval(ast->left);
        else
            return ast_eval(ast->right);
    }
    else if (ast->type == AST_THEN || ast->type == AST_ELSE)
        return ast_eval(ast->left);
    else if (ast->type == AST_CMD)
    {
        int res;
        if (ast->var != NULL)
        {
            char *cmd = replace_vars(ast->val->data, ast->var, ast->replace);
            char *tmp = strdup(ast->var + 1);
            char *newvar = zalloc(sizeof(char) * strlen(tmp) + 4);
            sprintf(newvar, "${%s}", tmp);
            char *cmd2 = replace_vars(cmd, newvar, ast->replace);
            free(newvar);
            free(cmd);
            free(tmp);
            res = cmd_exec(cmd2);
            free(cmd2);
        }
        else
            res = cmd_exec(ast->val->data);
        if (!ast->left)
            return res;
        return ast_eval(ast->left);
    }
    else if (ast->type == AST_REDIR)
        return exec_redir(ast);
    else if (ast->type == AST_PIPE)
        return eval_pipe(ast);
    else if (ast->type == AST_AND)
    {
        int left = ast_eval(ast->left);
        if (left != 0)
            return left;
        return ast_eval(ast->right);
    }
    else if (ast->type == AST_NEG)
        return !ast_eval(ast->left);
    else if (ast->type == AST_WHILE)
    {
        int a = 0;
        while (ast_eval(ast->cond) == 0)
        {
            a = ast_eval(ast->left);
        }
        return a;
    }
    else if (ast->type == AST_UNTIL)
    {
        int a = 0;
        while (ast_eval(ast->cond) != 0)
        {
            a = ast_eval(ast->left);
        }
        return a;
    }
    else if (ast->type == AST_FOR)
    {
        set_var(ast->val->data, ast->left);
        int ret_code = 0;
        for (size_t i = 0; i < ast->size; ++i)
        {
            set_replace(ast->list[i], ast->left);
            ret_code = ast_eval(ast->left);
        }
        return ret_code;
    }
    else
    {
        fprintf(stderr, "ast_eval: node type not known\n");
        return 2;
    }
}

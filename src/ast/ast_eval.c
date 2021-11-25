#include "ast.h"

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utils/alloc.h>
#include <utils/utils.h>
#include <utils/vec.h>

/**
 * \brief The number of builtins commands
 */
#define BLT_NB 1

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
    *size = 2;
    char **args = xmalloc(sizeof(char *) * 3);
    int i = 0;
    args[0] = getcmdname(cmd, &i);
    if (cmd[i] == 0)
        args[1] = NULL;
    else
        args[1] = strdup(cmd + i + 1);
    args[2] = NULL;
    return args;
}

/**
* \brief Execute a command in a sub-process
 * @param cmd: The command to execute
 * @return: return if the command fail or succeed
*/
static bool fork_exec(char *cmd)
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
            printf("%s\n", strerror(errno));
            exit(-42);
        }
    }

    for (int i = 0; i < size; i++)
        free(args[i]);
    free(args);
    int wstatus;
    int cpid = waitpid(pid, &wstatus, 0);

    if (cpid == -1)
        errx(1, "Failed waiting for child\n%s", strerror(errno));

    return WEXITSTATUS(wstatus) == 0;

}

static bool echo(char *args)
{
    args[0] = 0;
    printf("hello from echo !\n");
    return true;
}

/**
* \brief Choose to execute builtin commands or
 * not builtins.
 * @param cmd: the command to execute
 * @return: return if the command fail or succeed
*/
static bool cmd_exec(char *cmd)
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

bool ast_eval(struct ast *ast)
{
    if (!ast)
        return true;
    if (ast->type == AST_ROOT)
        return ast_eval(ast->left) && ast_eval(ast->right);
    else if (ast->type == AST_IF)
    {
        if (ast_eval(ast->cond))
            return ast_eval(ast->left);
        else
            return ast_eval(ast->right);
    }
    else if (ast->type == AST_THEN || ast->type == AST_ELSE)
        return ast_eval(ast->left);
    else if (ast->type == AST_CMD)
        return cmd_exec(vec_cstring(ast->val));
    else
    {
        fprintf(stderr, "ast_eval: node type not known\n");
        return false;
    }
}
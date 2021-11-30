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
#define REDIR_NB 1

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

// Return wether the char is a redir mode
static int is_redirchar(char c)
{
    char redir_char[4] = "<>|&";
    for (size_t i = 0; i < 4; i++)
    {
        if (c == redir_char[i])
            return 1;
    }
    return 0;
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
    while(cmd[i] != 0)
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
    char *redirs_name[] = { ">" };
    redirs_funcs redirs[REDIR_NB] = { &redir_simple_left };

    size_t i = 0;
    int fd = 1; // Default case: 1 = STDOUT
    if (!is_redirchar(ast->val->data[i]))
        fd = '0' + ast->val->data[i++];
    while (is_redirchar(ast->val->data[i]))
        i++;
    if (i > 2)
        return 0; // Not valid
    char *redir_mode = strndup(ast->val->data, i);
    while (isspace(ast->val->data[i])) // Skip spaces before WORD
        i++;
    char *right = strdup(ast->val->data + i); // Skip redir operator
    int return_code = -1;
    for (size_t index = 0; index < REDIR_NB; index++)
    {
        if (strcmp(redirs_name[index], redir_mode) == 0)
        {
            return_code = redirs[index](ast->left->val->data, fd, right);
            break;
        }
    }
    free(redir_mode);
    free(right);
    return return_code; // Redir not found
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
        int right = ast_eval(ast->right);
        if (right == 0)
            return right;
        return left;
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
        int res = cmd_exec(vec_cstring(ast->val));
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
    else
    {
        fprintf(stderr, "ast_eval: node type not known\n");
        return 2;
    }
}

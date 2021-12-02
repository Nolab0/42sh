#ifndef AST_H
#define AST_H

#include <stdbool.h>

/**
 * \brief Possible nodes types for ast structure.
 */
enum ast_type
{
    AST_ROOT,
    AST_IF,
    AST_THEN,
    AST_ELIF,
    AST_ELSE,
    AST_CMD,
    AST_REDIR,
    AST_PIPE,
    AST_AND,
    AST_OR,
    AST_NEG
};

/**
 * \brief Structure for ast.
 * @details: cond use for ifs conditions
 */
struct ast
{
    enum ast_type type;
    struct vec *val;
    struct ast *cond;
    struct ast *left;
    struct ast *right;
};

/**
 * \brief Array of pointers to builtins commands.
 */
typedef int (*commands)(char *args);

/**
 * \brief  Functions pointers arrays to redirection functions
 * @param left: the left part of the redirection
 * @param fd: the file descriptor (STDOUT by default)
 * @param right: the right part of the redirection
 * @return value: return code of exec, -1 on failure
 */
typedef int (*redirs_funcs)(struct ast *left, int fd, char *right);

/**
 * \brief Evaluate the ast and execute commands.
 */
int ast_eval(struct ast *ast);

struct ast *create_ast(enum ast_type type);

void ast_free(struct ast *ast);

void pretty_print(struct ast *ast);

/**
 * \brief Choose to execute builtin commands or
 * not builtins.
 * @param cmd: the command to execute
 * @return: return if the command fail or succeed
 */
int cmd_exec(char *cmd);

#endif /* ! AST_H */

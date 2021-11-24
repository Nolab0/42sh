#include "ast.h"

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <utils/alloc.h>
#include <utils/vec.h>

struct ast *create_ast(enum ast_type type)
{
    struct ast *new = zalloc(sizeof(struct ast));
    new->type = type;
    new->left = NULL;
    new->right = NULL;
    new->val = NULL;
    new->cond = NULL;
    return new;
}

void pretty_print(struct ast *ast)
{
    if (!ast)
        return;
    if (ast->type == AST_CMD)
    {
        printf("command \"");
        vec_print(ast->val);
        printf("\" ");
        pretty_print(ast->left);
    }
    if (ast->type == AST_IF)
    {
        printf("if { ");
        pretty_print(ast->cond);
        printf("};");
        pretty_print(ast->left);
        pretty_print(ast->right);
    }
    if (ast->type == AST_ELIF)
    {
        printf("elif { ");
        pretty_print(ast->cond);
        printf("};");
        pretty_print(ast->left);
        pretty_print(ast->right);
    }
    if (ast->type == AST_THEN)
    {
        printf("then { ");
        pretty_print(ast->left);
        printf("}");
    }
    if (ast->type == AST_ELSE)
    {
        printf("else { ");
        pretty_print(ast->left);
        printf("}");
    }
}

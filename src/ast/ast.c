#include "ast.h"

#include <err.h>
#include <stdlib.h>
#include <utils/alloc.h>

struct ast *create_ast(enum ast_type type)
{
    struct ast *new = zalloc(sizeof(struct ast));
    new->type = type;
    new->left = NULL;
    new->right = NULL;
    new->val = NULL;
    return new;
}

void pretty_print(struct ast *ast)
{
    if (!ast)
        return;
    if (ast->type == AST_CMD)
        printf("command \"%s\" ", ast->val);
    if (ast->type == AST_IF)
    {
        printf("if { %s } ", ast->val);
    }

    pretty_print(ast->left);
    pretty_print(ast->right);
}

#ifndef AST_H
#define AST_H

enum ast_type
{
    AST_ROOT,
    AST_IF,
    AST_THEN,
    AST_ELIF,
    AST_ELSE,
    AST_SEMIC,
    AST_NEWL,
    AST_SQUOTE,
    AST_CMD
};

struct ast
{
    enum ast_type type;
    struct vec *val;
    struct ast *cond;
    struct ast *left;
    struct ast *right;
};

struct ast *create_ast(enum ast_type type);

void ast_free(struct ast *ast);

void pretty_print(struct ast *ast);

#endif /* ! AST_H */

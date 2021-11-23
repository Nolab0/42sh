#ifndef AST_H
#define AST_H

enum ast_type
{
    AST_IF,
    AST_THEN,
    AST_ELIF,
    AST_ELSE,
    AST_SEMIC,
    AST_NEWL,
    AST_SQUOTE,
    AST_WORDS
};

struct ast
{
    enum ast_type type;
    char *val;
    struct ast *left;
    struct ast *right;
};

#endif /* ! AST_H */

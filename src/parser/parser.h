#ifndef PARSER_H
#define PARSER_H

#include <ast/ast.h>

enum state
{
    PARSER_OK,
    PARSER_PANIC
};

struct parser
{
    struct ast *ast;
    enum state state;
};

#endif /* ! PARSER_H */

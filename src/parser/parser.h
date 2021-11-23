#ifndef PARSER_H
#define PARSER_H

#include <ast/ast.h>
#include <lexer/lexer.h>

enum state
{
    PARSER_OK,
    PARSER_PANIC,
};

struct parser
{
    struct ast *ast;
    struct lexer *lexer;
};

enum state parsing(struct parser *parser);

struct parser *create_parser();

#endif /* ! PARSER_H */

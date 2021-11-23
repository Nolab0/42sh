#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

#include "token.h"

struct lexer
{
    char *input;
    size_t pos;
    struct token *cur_tok;
};

#endif /* ! LEXER_H */

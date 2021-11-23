#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

#include "token.h"

struct lexer
{
    char *input;
    size_t pos;
    struct token *current_tok;
};

// Create a new lexer with input
struct lexer *lexer_create(char *input);

// Free the lexer but not the input
void lexer_free(struct lexer *lexer);

// Return the next token without going forward in the input
struct token *lexer_peek(struct lexer *lexer);

// Return the next token, removes it from input and update cur_tok
struct token *lexer_pop(struct lexer *lexer);

#endif /* ! LEXER_H */

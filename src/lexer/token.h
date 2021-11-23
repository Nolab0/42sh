#ifndef TOKEN_H
#define TOKEN_H

#include <utils/vec.h>

enum token_type
{
    TOKEN_IF,
    TOKEN_THEN,
    TOKEN_ELIF,
    TOKEN_ELSE,
    TOKEN_FI,
    TOKEN_SEMIC,
    TOKEN_NEWL,
    TOKEN_WORD,
    TOKEN_EOF,
    TOKEN_ECHO
};

struct token
{
    enum token_type type;
    char *value;
};

// Create a token according to type
struct token *token_create(enum token_type type);

void token_free(struct token *token);

#endif /* ! TOKEN_H */

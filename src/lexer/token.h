#ifndef TOKEN_H
#define TOKEN_H

#include <utils/vec.h>

enum token_type
{
    TOKEN_IF,
    TOKEN_THEN,
    TOKEN_ELIF,
    TOKEN_ELSE,
    TOKEN_SEMIC,
    TOKEN_NEWL,
    TOKEN_SQUOTE,
    TOKEN_WORDS,
    TOKEN_EOF,
    TOKEN_ERROR
};

struct token
{
    enum token_type *type;
    struct vec *val;
};

#endif /* ! TOKEN_H */

#include "lexer.h"

#include <ctype.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <utils/alloc.h>

#define SIZE 9

static int match_token(char *str)
{
    char *names[SIZE] = { "if", "then", "else", "elif", "fi", ";", "\n", "'", "echo" };
    int types[SIZE] = { TOKEN_IF, TOKEN_THEN, TOKEN_ELSE, TOKEN_ELIF, TOKEN_FI, TOKEN_SEMIC, TOKEN_NEWL, TOKEN_SQUOTE, TOKEN_ECHO };
    for (size_t i = 0; i < SIZE; i++)
    {
        if (strcmp(str, names[i]) == 0)
            return types[i];
    }
    return TOKEN_WORD;
}

// Get a token in a stream
static struct token *get_token(struct lexer *lexer)
{
    size_t pos = lexer->pos;
    size_t slen = strlen(lexer->input);
    if (pos == slen)
        return token_create(TOKEN_EOF);
    while (pos < slen && isspace(lexer->input[pos]))
        pos++;
    size_t len = 0;
    // TODO: Add value to word token
    while ((pos + len < slen) && isalnum(lexer->input[pos + len]))
        len++;
    lexer->pos = pos + len;
    char *cur = strndup(lexer->input + pos, len);
    struct token *tok = token_create(match_token(cur));
    free(cur);
    return tok;
}

struct lexer *lexer_create(char *input)
{
    struct lexer *new = zalloc(sizeof(struct lexer));
    new->input = input;
    new->pos = 0;
    new->current_tok = get_token(new);
    return new;
}

void lexer_free(struct lexer *lexer)
{
    free(lexer);
}

struct token *lexer_peek(struct lexer *lexer)
{
    return lexer->current_tok;
}

struct token *lexer_pop(struct lexer *lexer)
{
    struct token *current = lexer->current_tok;
    lexer->current_tok = get_token(lexer);
    return current;
}

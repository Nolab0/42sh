#include "parser.h"

#include <stddef.h>

#include "lexer.h"

static enum parser_status handle_parse_error(enum state state,
                                             struct parser *parser)
{
    warnx("unexpected token");
    ast_free(parser->ast);
    parser->ast = NULL;
    return state;
}

struct parser *create_parser()
{
    struct parser *parser = zalloc(sizeof(struct parser));
    parser->lexer = NULL;
    parser->ast = NULL;
    return parser;
}

enum state parsing(struct parser *parser)
{
    struct token *tok = lexer_peek(lexer);

    if (tok->type == TOKEN_EOF)
    {
        parser->ast = NULL;
        return PARSER_OK;
    }

    enum state state = parse_command(parser, &(parser->ast));
    if (state != PARSER_OK)
        return handle_parse_error(parser);

    if (lexer_peek(lexer)->type == TOKEN_EOF)
        return PARSER_OK;

    return handle_parse_error(PARSER_PANIC, parser);
}

static enum state parse_command(struct parser *parser, struct ast **ast)
{
    enum state state = parse_shell_command(parser, ast);
    if (state != PARSER_OK)
    {
        enum state state2 = parse_simple_command(parser, ast);
        if (state2 != PARSER_OK)
            return state;
    }
    return state;
}

static enum state parse_simple_command(struct parser *parser, struct ast **ast)
{
    struct ast *new = create_ast(AST_CMD);
    struct token *tok;
    if (lexer_peek(parser->lexer)->type != TOKEN_WORD)
    {
        free(new);
        return PARSER_PANIC;
    }
    while ((tok = lexer_peek(parser->lexer))->type == TOKEN_WORD)
    {
        lexer_pop(parser->lexer);
        vec_concat(new->val, tok->val);
        free(tok->val);
        free(tok);
    }
    *ast = new;
    return PARSER_OK;
}

static enum state parse_shell_command(struct parser *parser, struct ast **ast)
{
    return parse_rule_if(parser, ast);
}

static enum state parse_rule_if(struct parser *parser, struct ast **ast)
{
    // checking for if token
    if (lexer_peek(parser->lexer)->type != TOKEN_IF)
        return PARSER_PANIC;
    lexer_pop(parser->lexer);
    struct ast *new = create_ast(AST_IF);
    *ast = new;

    // getting condition for if
    enum state state = parse_compound_list(parser, (*ast)->cond);
    if (state != PARSER_OK)
        return state;

    // checking for then token
    if (lexer_peek(parser->lexer)->type != TOKEN_THEN)
        return PARSER_PANIC;
    lexer_pop(parser->lexer);
    new = create_ast(AST_THEN);
    (*ast)->left = new;

    // getting commands for then
    state = parse_compound_list(parser, (*ast)->left->left);
    if (state != parser_ok)
        return state;

    // launch else_clause function
    state = parse_else_clause(parser, (*ast)->right);
    if (state == PARSER_PANIC)
        return state;

    // checking for fi token
    if (lexer_peek(parser->lexer)->type != TOKEN_FI)
        return PARSER_PANIC;
    lexer_pop(parser->lexer);
    return PARSER_OK;
}

static enum state parse_else_clause(struct parser *parser, struct ast **ast)
{
    // checking for else token
    if (lexer_peek(parser->lexer)->type != TOKEN_ELSE)
    {
        if (lexer_peek(parser->lexer)->type != TOKEN_ELIF)
            return PARSER_ABSENT;
        return parse_elif(parser, ast);
    }
    lexer_pop(parser->lexer);
    struct ast *new = create_ast(AST_ELSE);
    *ast = new;

    // getting commands for else
    return parse_compound_list(parser, (*ast)->left);
}

static enum state parse_elif(struct parser *parser, struct ast **ast)
{
    // skip elif (we know it is here)
    lexer_pop(parser->lexer);
    struct ast *new = create_ast(AST_ELIF);
    *ast = new;

    // getting condition for elif
    enum state state = parse_compound_list(parser, (*ast)->cond);
    if (state != PARSER_OK)
        return state;

    // checking for then token
    if (lexer_peek(parser->lexer)->type != TOKEN_THEN)
        return PARSER_PANIC;
    lexer_pop(parser->lexer);
    new = create_ast(AST_THEN);
    (*ast)->left = new;

    // getting commands for then
    state = parse_compound_list(parser, (*ast)->left->left);
    if (state != parser_ok)
        return state;

    // launch else_clause function
    state = parse_else_clause(parser, (*ast)->right);
    if (state == PARSER_PANIC)
        return state;

    return PARSER_OK;
}

static enum state parse_compound_list(struct parser *parser, struct ast **ast)
{
    struct token *tok;
    struct ast *cur = *ast;
    while ((tok = lexer_peek(parser->lexer))->type == TOKEN_NEWL)
        lexer_pop(parser->lexer);

    enum state state = parse_and_or(parser, ast);
    if (state != PARSER_OK)
        return state;
    if (cur != NULL)
        cur = cur->left;

    while (42)
    {
        tok = lexer_peek(parser->lexer);
        if (tok->type != NEWL && tok->type != SEMIC)
            break;
        lexer_pop(parser->lexer);
        while ((tok = lexer_peek(parser->lexer))->type == TOKEN_NEWL)
            lexer_pop(parser->lexer);

        state = parse_and_or(parser, ast);
        if (state == PARSER_ABSENT)
            break;
        else if (state == PARSER_PANIC)
            return state;
        if (cur != NULL)
            cur = cur->left;
    }
    return PARSER_OK;
}


static enum state parse_and_or(struct parser *parser, struct ast **ast)
{
    return parse_simple_command(parser, ast);
}

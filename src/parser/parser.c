#include "parser.h"

#include <ast/ast.h>
#include <err.h>
#include <lexer/lexer.h>
#include <lexer/token.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <utils/vec.h>

struct ast *parent = NULL;

enum parser_state parse_else_clause(struct parser *parser, struct ast **ast);
enum parser_state parse_elif(struct parser *parser, struct ast **ast);
enum parser_state parse_pipe(struct parser *parser, struct ast **ast);

static enum parser_state handle_parse_error(enum parser_state state,
                                            struct parser *parser)
{
    warnx("unexpected token");
    ast_free(parser->ast);
    parser->ast = NULL;
    return state;
}

void parser_free(struct parser *parser)
{
    lexer_free(parser->lexer);
    ast_free(parser->ast);
    free(parser);
}

struct parser *create_parser()
{
    struct parser *parser = zalloc(sizeof(struct parser));
    parser->lexer = NULL;
    parser->ast = NULL;
    return parser;
}

static enum parser_state parse_simple_command(struct parser *parser,
                                              struct ast **ast)
{
    struct ast *new = create_ast(AST_CMD);
    enum token_type tok_type = lexer_peek(parser->lexer)->type;
    if (tok_type == TOKEN_ERROR)
    {
        free(new);
        return PARSER_PANIC;
    }
    if (tok_type != TOKEN_WORD && tok_type != TOKEN_ECHO)
    {
        free(new);
        return PARSER_ABSENT;
    }
    struct token *tok = lexer_peek(parser->lexer);
    if (tok->type == TOKEN_ERROR)
    {
        free(new);
        return PARSER_PANIC;
    }
    while (tok->type == TOKEN_WORD || tok->type == TOKEN_ECHO)
    {
        struct vec *tmp = zalloc(sizeof(struct vec));
        tmp->data = strdup(tok->value);
        tmp->size = strlen(tok->value);
        tmp->capacity = tmp->size + 1;
        new->val = vec_concat(new->val, tmp);
        vec_destroy(tmp);
        free(tmp);
        lexer_pop(parser->lexer);
        token_free(tok);
        tok = lexer_peek(parser->lexer);
        if (tok->type == TOKEN_ERROR)
        {
            vec_destroy(new->val);
            free(new->val);
            free(new);
            return PARSER_PANIC;
        }
        if (tok->type == TOKEN_WORD || tok->type == TOKEN_ECHO)
        {
            new->val->size--;
            vec_push(new->val, ' ');
            vec_push(new->val, '\0');
        }
    }

    *ast = new;
    return PARSER_OK;
}

static enum parser_state parse_and_or(struct parser *parser, struct ast **ast)
{
    return parse_pipe(parser, ast);
}

static enum parser_state parse_compound_list(struct parser *parser,
                                             struct ast **ast)
{
    struct token *tok;
    while ((tok = lexer_peek(parser->lexer))->type == TOKEN_NEWL)
    {
        lexer_pop(parser->lexer);
        token_free(tok);
    }
    if (tok->type == TOKEN_ERROR)
        return PARSER_PANIC;

    enum parser_state state = parse_and_or(parser, ast);
    if (state != PARSER_OK)
        return state;

    struct ast *cur = *ast;

    while (42)
    {
        struct ast **tmp = &(cur->left);
        tok = lexer_peek(parser->lexer);
        if (tok->type == TOKEN_ERROR)
            return PARSER_PANIC;
        if (tok->type != TOKEN_NEWL && tok->type != TOKEN_SEMIC)
            break;
        lexer_pop(parser->lexer);
        token_free(tok);
        while ((tok = lexer_peek(parser->lexer))->type == TOKEN_NEWL)
        {
            lexer_pop(parser->lexer);
            token_free(tok);
        }
        if (tok->type == TOKEN_ERROR)
            return PARSER_PANIC;

        state = parse_and_or(parser, tmp);
        if (state == PARSER_ABSENT)
            break;
        else if (state == PARSER_PANIC)
            return state;
        cur = cur->left;
    }
    return PARSER_OK;
}

enum parser_state parse_else_clause(struct parser *parser, struct ast **ast)
{
    // checking for else token
    struct token *tok = lexer_peek(parser->lexer);
    if (tok->type == TOKEN_ERROR)
        return PARSER_PANIC;
    if (tok->type != TOKEN_ELSE)
    {
        if (tok->type != TOKEN_ELIF)
            return PARSER_ABSENT;
        return parse_elif(parser, ast);
    }
    tok = lexer_pop(parser->lexer);
    token_free(tok);
    struct ast *new = create_ast(AST_ELSE);
    parent = new;
    *ast = new;

    // getting commands for else
    return parse_compound_list(parser, &((*ast)->left));
}

enum parser_state parse_elif(struct parser *parser, struct ast **ast)
{
    // skip elif (we know it is here)
    struct token *tok = lexer_pop(parser->lexer);
    token_free(tok);
    struct ast *new = create_ast(AST_ELIF);
    *ast = new;

    // getting condition for elif
    enum parser_state state = parse_compound_list(parser, &((*ast)->cond));
    if (state != PARSER_OK)
        return state;

    // checking for then token
    if (lexer_peek(parser->lexer)->type != TOKEN_THEN)
        return PARSER_PANIC;
    tok = lexer_pop(parser->lexer);
    token_free(tok);
    new = create_ast(AST_THEN);
    parent = new;
    (*ast)->left = new;

    // getting commands for then
    state = parse_compound_list(parser, &((*ast)->left->left));
    if (state != PARSER_OK)
        return state;

    // launch else_clause function
    state = parse_else_clause(parser, &((*ast)->right));
    if (state == PARSER_PANIC)
        return state;

    return PARSER_OK;
}

static enum parser_state parse_rule_if(struct parser *parser, struct ast **ast)
{
    // checking for if token
    if (lexer_peek(parser->lexer)->type != TOKEN_IF)
        return PARSER_PANIC;
    struct token *tok = lexer_pop(parser->lexer);
    token_free(tok);

    struct ast *new = create_ast(AST_IF);
    *ast = new;

    // getting condition for if
    enum parser_state state = parse_compound_list(parser, &((*ast)->cond));
    if (state != PARSER_OK)
        return state;

    // checking for then token
    if (lexer_peek(parser->lexer)->type != TOKEN_THEN)
        return PARSER_PANIC;
    tok = lexer_pop(parser->lexer);
    token_free(tok);

    new = create_ast(AST_THEN);
    parent = new;
    (*ast)->left = new;

    // getting commands for then
    state = parse_compound_list(parser, &((*ast)->left->left));
    if (state != PARSER_OK)
        return state;

    // launch else_clause function
    state = parse_else_clause(parser, &((*ast)->right));
    if (state == PARSER_PANIC)
        return state;

    // checking for fi token
    if (lexer_peek(parser->lexer)->type != TOKEN_FI)
        return PARSER_PANIC;
    tok = lexer_pop(parser->lexer);
    token_free(tok);
    return PARSER_OK;
}

static enum parser_state parse_shell_command(struct parser *parser,
                                             struct ast **ast)
{
    return parse_rule_if(parser, ast);
}

static enum parser_state parse_command(struct parser *parser, struct ast **ast)
{
    enum parser_state state = parse_shell_command(parser, ast);
    if (state != PARSER_OK)
    {
        enum parser_state state2 = parse_simple_command(parser, ast);
        return state2;
    }
    return state;
}

enum parser_state parse_pipe(struct parser *parser, struct ast **ast)
{
    // parsing command
    enum parser_state state = parse_command(parser, ast);
    if (state != PARSER_OK)
    {
        return PARSER_PANIC;
    }
    // parsing ( '|' ( /n )* command )*
    while (1)
    {
        // parsing '|'
        struct token *tok = lexer_peek(parser->lexer);
        if (tok->type == TOKEN_ERROR)
            return PARSER_PANIC;
        if (tok->type != TOKEN_PIPE)
            break;
        struct ast *pipe_node = create_ast(AST_PIPE);
        if (!parent)
            pipe_node->left = *ast;
        else
        {
            if (parent->type == TOKEN_THEN || parent->type == TOKEN_ELSE)
                pipe_node->left = parent->left;
            else
                pipe_node->left = parent->right;
        }
        parent = pipe_node;
        *ast = pipe_node;
        parser->ast = *ast;
        lexer_pop(parser->lexer);
        token_free(tok);

        // parsing (/n)*
        while ((tok = lexer_peek(parser->lexer))->type == TOKEN_NEWL)
        {
            lexer_pop(parser->lexer);
            token_free(tok);
        }

        if (tok->type == TOKEN_ERROR)
        {
            return PARSER_PANIC;
        }

        // parsing command
        state = parse_command(parser, ast);
        if (state != PARSER_OK)
            return state;
    }
    return state;
}

static enum parser_state parse_list(struct parser *parser, struct ast **ast)
{
    enum parser_state state = parse_command(parser, ast);
    if (state != PARSER_OK)
        return state;

    struct ast *cur = *ast;
    while (1)
    {
        struct ast **tmp = &(cur->left);
        struct token *tok = lexer_peek(parser->lexer);
        if (tok->type == TOKEN_ERROR)
            return PARSER_PANIC;
        if (tok->type != TOKEN_SEMIC)
            break;
        lexer_pop(parser->lexer);
        token_free(tok);
        state = parse_command(parser, tmp);
        if (state == PARSER_ABSENT)
            break;
        else if (state == PARSER_PANIC)
            return state;
        cur = cur->left;
    }
    return state;
}

static enum parser_state parse_input(struct parser *parser, struct ast **ast)
{
    struct token *tok = lexer_peek(parser->lexer);

    if (tok->type == TOKEN_ERROR)
        return PARSER_PANIC;
    if (tok->type == TOKEN_EOF)
        return PARSER_OK;

    if (tok->type == TOKEN_NEWL)
    {
        lexer_pop(parser->lexer);
        token_free(tok);
        return parse_input(parser, ast);
    }

    enum parser_state state = PARSER_PANIC;
    if (*ast && (*ast)->type == AST_ROOT)
        state = parse_list(parser, &((*ast)->right));
    else
        state = parse_list(parser, ast);

    if (state != PARSER_OK)
        return state;

    tok = lexer_peek(parser->lexer);

    if (tok->type == TOKEN_ERROR)
        return PARSER_PANIC;
    if (tok->type == TOKEN_EOF)
        return PARSER_OK;

    if (tok->type == TOKEN_NEWL)
    {
        struct ast *placeholder = create_ast(AST_ROOT);
        placeholder->left = *ast;
        parent = placeholder;
        ast = &placeholder;
        parser->ast = (*ast);
        lexer_pop(parser->lexer);
        token_free(tok);
        return parse_input(parser, ast);
    }

    return PARSER_PANIC;
}

enum parser_state parsing(struct parser *parser)
{
    enum parser_state state = parse_input(parser, &(parser->ast));
    if (state != PARSER_OK)
        return handle_parse_error(state, parser);

    if (lexer_peek(parser->lexer)->type == TOKEN_EOF)
        return PARSER_OK;

    return handle_parse_error(PARSER_PANIC, parser);
}

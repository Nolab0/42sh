#include <builtins/builtin.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <parser/parser.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utils/alloc.h>
#include <utils/utils.h>
#include <utils/vec.h>

#include "ast.h"

int is_valid(char *str)
{
    size_t len = strlen(str);
    if (str[len - 1] == ')')
    {
        str[len - 1] = '\0'; // Remove closing parenthesis
        return 1;
    }
    return 0;
}

int subshell(char *args)
{
    if (!is_valid(args))
    {
        fprintf(stderr, "42sh: Syntax error: end of file unexecpected\n");
        return 2;
    }
    struct parser *parser = create_parser();
    parser->lexer = lexer_create(args);
    enum parser_state state = parsing(parser);
    if (state != PARSER_OK)
    {
        free(args);
        return 2;
    }
    int pid = fork();
    if (pid == 0)
    {
        int return_value = 0;
        ast_eval(parser->ast, &return_value);
        parser_free(parser);
        exit(return_value);
    }
    parser_free(parser);
    int wstatus;
    int cpid = waitpid(pid, &wstatus, 0);
    if (cpid == -1)
        errx(1, "Failed waiting for child\n%s", strerror(errno));
    struct list *ret = zalloc(sizeof(struct list));
    ret->name = strdup("?");
    ret->value = my_itoa(WEXITSTATUS(wstatus));
    ret->next = NULL;
    add_var(ret);
    return WEXITSTATUS(wstatus);
}


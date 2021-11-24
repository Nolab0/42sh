#include <err.h>
#include <io/cstream.h>
#include <parser/parser.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <utils/vec.h>

/**
 * \brief Parse the command line arguments
 * \return A character stream
 */
static struct cstream *parse_args(int argc, char *argv[])
{
    // If launched without argument, read the standard input
    if (argc == 1)
    {
        if (isatty(STDIN_FILENO))
            return cstream_readline_create();
        return cstream_file_create(stdin, /* fclose_on_free */ false);
    }

    // 42sh FILENAME
    if (argc == 2)
    {
        FILE *fp = fopen(argv[1], "r");
        if (fp == NULL)
        {
            warn("failed to open input file");
            return NULL;
        }

        return cstream_file_create(fp, /* fclose_on_free */ true);
    }

    fprintf(stderr, "Usage: %s [COMMAND]\n", argv[0]);
    return NULL;
}

/**
 * \brief Read and print lines on newlines until EOF
 * \return An error code
 */
enum error read_print_loop(struct cstream *cs, struct vec *line,
                           struct parser *parser)
{
    enum error err;
    struct vec *final = NULL;

    while (true)
    {
        // Read the next character
        int c;
        if ((err = cstream_pop(cs, &c)))
            return err;

        // If the end of file was reached, stop right there
        if (c == EOF)
            break;

        // If a newline was met, print the line
        if (c == '\n')
        {
            final = vec_concat(final, line);
            final->size--;
            vec_push(final, '\n');
            vec_push(final, '\0');
            vec_reset(line);
            continue;
        }

        // Otherwise, add the character to the line
        vec_push(line, c);
    }
    vec_destroy(line);

    parser->lexer = lexer_create(vec_cstring(final));
    enum parser_state state = parsing(parser);
    if (state != PARSER_OK)
        return PARSER_ERROR;
    pretty_print(parser->ast);
    vec_destroy(final);
    free(final);
    return NO_ERROR;
}

int main(int argc, char *argv[])
{
    int rc = 0;

    // Parse command line arguments and get an input stream
    struct cstream *cs;
    if ((cs = parse_args(argc, argv)) == NULL)
        return 1;

    // Create a vector to hold the current line
    struct vec *line = vec_init();

    struct parser *parser = create_parser();
    // Run the test loop
    if (read_print_loop(cs, line, parser) != NO_ERROR)
    {
        rc = 1;
        vec_destroy(line);
    }

    free(line);
    cstream_free(cs);
    free(cs);
    parser_free(parser);
    return rc;
}

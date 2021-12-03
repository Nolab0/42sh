#include <stddef.h>

int is_separator(char c)
{
    char separator[7] = ";|\0\n "; // Array of possible separator
    for (size_t i = 0; i < 7; i++)
    {
        if (c == separator[i])
            return 1;
    }
    return 0;
}

// Return wether the char is a redir mode
int is_redirchar(char c)
{
    char redir_char[4] = "<>|&";
    for (size_t i = 0; i < 4; i++)
    {
        if (c == redir_char[i])
            return 1;
    }
    return 0;
}

#include <stddef.h>

int is_separator(char c)
{
    char separator[5] = ";|\0\n "; // Array of possible separator
    for (size_t i = 0; i < 5; i++)
    {
        if (c == separator[i])
            return 1;
    }
    return 0;
}

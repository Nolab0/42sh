#include "redirection.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include "ast.h"

int redir_simple_left(char *left, int fd, char *right)
{
    int save_fd = dup(fd);

    int file_fd = open(right, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (file_fd == -1)
        return -1;
    if (dup2(file_fd, fd) == -1)
        return -1;
    int r_code = cmd_exec(left);
    fflush(stdout);
    dup2(save_fd, fd); // Restore file descriptor
    close(file_fd);
    return r_code;
}

#include "redirection.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * \brief Generic function for left redirection
 * @paramm append: if true redirection in append mode
 */
static int redir_left(struct ast *left, int fd, char *right, int append)
{
    if (fd == -1)
        fd = STDOUT_FILENO; // Default case: STDOUT
    int save_fd = dup(fd);

    int file_fd = 0;
    if (append == 1)
        file_fd = open(right, O_CREAT | O_APPEND | O_WRONLY, 0644);
    else
        file_fd = open(right, O_CREAT | O_TRUNC  | O_WRONLY, 0644);
    if (file_fd == -1)
        return -1;
    if (dup2(file_fd, fd) == -1)
        return -1;
    fcntl(fd, F_SETFD, FD_CLOEXEC);
    int r_code = ast_eval(left);
    fflush(stdout);
    if (dup2(save_fd, fd) == -1) // Restore file descriptor
        return -1;
    close(file_fd);
    close(save_fd);
    return r_code;
}

int redir_simple_left(struct ast *left, int fd, char *right)
{
    return redir_left(left, fd, right, 0);
}

int redir_simple_right(struct ast *left, int fd, char *right)
{
    if (fd == -1)
        fd = STDIN_FILENO; // Default case: STDIN
    int save_fd = dup(fd);

    int file_fd = open(right, O_RDONLY);
    if (file_fd == -1)
        return -1;
    if (dup2(file_fd, fd) == -1)
        return -1;
    fcntl(fd, F_SETFD, FD_CLOEXEC);
    int r_code = ast_eval(left);
    fflush(stdout);
    if (dup2(save_fd, fd) == -1) // Restore file descriptor
        return -1;
    close(file_fd);
    close(save_fd);
    return r_code;
}

int redir_double_left(struct ast *left, int fd, char *right)
{
    return redir_left(left, fd, right, 1);
}

int redir_ampersand_left(struct ast *left, int fd, char *right)
{
    int save_fd = dup(fd);
    int file_fd = strtol(right, NULL, 10);
    if (file_fd == -1)
        return -1;
    if (dup2(file_fd, fd) == -1)
        return -1;
    fcntl(fd, F_SETFD, FD_CLOEXEC);
    int r_code = ast_eval(left);
    fflush(stdout);
    if (dup2(save_fd, fd) == -1) // Restore file descriptor
        return -1;

    close(file_fd);
    close(save_fd);
    return r_code;
}

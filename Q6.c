#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFFER_SIZE 256
#define MAX_ARGUMENTS 16

const char WELCOME_MSG[] = "Welcome in the ENSEA Shell\n\rTo quit, type 'exit'\n\r";
char prompt[50] = "enseash %% ";

int main(void) {
    char buffer[BUFFER_SIZE];
    ssize_t nb_char;
    int status;
    char *argv[MAX_ARGUMENTS];

    write(STDOUT_FILENO, WELCOME_MSG, strlen(WELCOME_MSG));

    while (1) {
        write(STDOUT_FILENO, prompt, strlen(prompt));
        nb_char = read(STDIN_FILENO, buffer, BUFFER_SIZE);

        if (nb_char < 0) {
            break;
        } else if (nb_char == 0) {
            write(STDOUT_FILENO, "Bye bye...\n", 11);
            break;}

        buffer[nb_char-1] ='\0';

        if (strcmp(buffer, "exit") == 0) {
            write(STDOUT_FILENO, "Bye bye...\n", 11);
            break;}

        int argc = 0;
        char *argument_string = strtok(buffer, " ");
        while (argument_string != NULL && argc < MAX_ARGUMENTS - 1) {
            argv[argc++] = argument_string;
            argument_string = strtok(NULL, " ");
        }
        argv[argc] = NULL;

        pid_t pid = fork();

        if (pid == -1) {
            write(STDERR_FILENO, "Error fork\n", 12);
        } else if (pid == 0) {
            execvp(argv[0], argv);
            _exit(EXIT_FAILURE);
        } else {
            wait(&status);
            if (WIFEXITED(status)) {
                // progamm endend properly
                int exit_code = WEXITSTATUS(status);
                sprintf(prompt, "enseash [exit:%d] %% ", exit_code);
            } else if (WIFSIGNALED(status)) {
                // programm has been killed
                int signal_code = WTERMSIG(status);
                sprintf(prompt, "enseash [sign:%d] %% ", signal_code);
            }
        }
    }

    return 0;
}
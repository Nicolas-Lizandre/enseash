#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFFER_SIZE 1024

const char WELCOME_MSG[] = "Bienvenue dans le Shell ENSEA.\nPour quitter taper 'exit'\n";
    
char prompt[50] = "enseash % "; // prompt is now a variable

int main(void) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    int status;

    write(STDOUT_FILENO, WELCOME_MSG, strlen(WELCOME_MSG));
    
    while (1) {
        write(STDOUT_FILENO, prompt, strlen(prompt));

        bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE);

        if (bytes_read == 0) {
            write(STDOUT_FILENO, "\nBye bye...\n", 12);
            break;
        }

        if (bytes_read == 5 && strncmp(buffer, "exit", 4) == 0) {
            write(STDOUT_FILENO, "Bye bye...\n", 11);
            break;
        }

        pid_t pid = fork();

        if (pid == -1) {
            write(STDERR_FILENO, "Error fork\n", 12);
        } 
        else if (pid == 0) {
            execlp(buffer, buffer, (char *)NULL);
            _exit(EXIT_FAILURE); 
        } 
        else {
            wait(&status);
            // update prompt for next call
            if (WIFEXITED(status)) {
                // progamm endend properly
                int exit_code = WEXITSTATUS(status);
                sprintf(prompt, "enseash [exit:%d] %% ", exit_code); 
            } 
            else if (WIFSIGNALED(status)) {
                // programm has been killed
                int signal_code = WTERMSIG(status);
                sprintf(prompt, "enseash [sign:%d] %% ", signal_code);
            }
        }
    }

    return 0;
}
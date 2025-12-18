#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h> // needed for "open()"

#define BUFFER_SIZE 1024
#define MAX_ARGS 64

const char WELCOME_MSG[] = "Bienvenue dans le Shell ENSEA.\nPour quitter taper 'exit'\n";
    
int main(void) {
    char buffer[BUFFER_SIZE];
    char prompt[100] = "enseash % ";
    ssize_t bytes_read;
    int status;
    struct timespec start, end;
    char *argv[MAX_ARGS];

    write(STDOUT_FILENO, WELCOME_MSG, strlen(WELCOME_MSG));
    
    while (1) {
        write(STDOUT_FILENO, prompt, strlen(prompt));

        bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE);
        
        buffer[bytes_read - 1] = '\0';

        if (bytes_read == 0) {
            write(STDOUT_FILENO, "\nBye bye...\n", 12);
            break;
        }

        if (bytes_read == 5 && strncmp(buffer, "exit", 4) == 0) {
            write(STDOUT_FILENO, "Bye bye...\n", 11);
            break;
        }

        argv[0] = strtok(buffer, " \t");

        if (argv[0] == NULL) {
            continue; 
        }

        int i = 0;
        while (argv[i] != NULL && i < MAX_ARGS - 1) {
            i++;
            argv[i] = strtok(NULL, " \t");
        }

        clock_gettime(CLOCK_REALTIME, &start);

        pid_t pid = fork();

        if (pid == -1) {
            write(STDERR_FILENO, "Error fork\n", 12);
        } 
        else if (pid == 0) {
            // gestion of redirection
            // first we see if there is "<" or ">" in the list of arg
            for (int j = 0; argv[j] != NULL; j++) {
                // for output ">"
                if (strncmp(argv[j], ">", 1) == 0) {
                    char *filename = argv[j+1]; // the file name is the arg just after
                    if (filename == NULL) { // error if there is no name
                        write(STDERR_FILENO, "Error : filename is needed for '>'\n", 30);
                        exit(EXIT_FAILURE);
                    }
                    // opening the file (write only, creat if not exist, empty is exist)
                    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd == -1) { perror("open"); exit(EXIT_FAILURE); }
                    
                    // we use dup2 to remplace STDOUT_FILENO by fd in the execution
                    dup2(fd, STDOUT_FILENO);
                    close(fd);

                    // we stop the list of arg here, execvp should not read ">" and the filename
                    argv[j] = NULL;
                    j++;
                }

                // for input "<"
                if (strncmp(argv[j], "<", 1) == 0) {
                    char *filename = argv[j+1]; // the file name is the arg just after
                    if (filename == NULL) { // error if there is no name
                        write(STDERR_FILENO, "Error : filename is needed for '>'\n", 30);
                        exit(EXIT_FAILURE);
                    }
                    // opening the file (read only)
                    int fd = open(filename, O_RDONLY);
                    if (fd == -1) { perror("open"); exit(EXIT_FAILURE); }
                    
                    // we use dup2 to remplace STDIN_FILENO by fd in the execution
                    dup2(fd, STDIN_FILENO);
                    close(fd);

                    // we stop the list of arg here, execvp should not read ">" and the filename
                    argv[j] = NULL;
                    j++;
                }
            }

            execvp(argv[0], argv); // execute the command
            exit(EXIT_FAILURE);
        } 
        else {
            wait(&status);

            clock_gettime(CLOCK_REALTIME, &end);

            long seconds = end.tv_sec - start.tv_sec;
            long nanoseconds = end.tv_nsec - start.tv_nsec;
            long elapsed_ms = (seconds * 1000) + (nanoseconds / 1000000);

            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                sprintf(prompt, "enseash [exit:%d|%ld ms] %% ", exit_code, elapsed_ms); 
            } 
            else if (WIFSIGNALED(status)) {
                int signal_code = WTERMSIG(status);
                sprintf(prompt, "enseash [sign:%d|%ld ms] %% ", signal_code, elapsed_ms);
            }
        }
    }

    return 0;
}
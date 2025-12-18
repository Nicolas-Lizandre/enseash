#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024
#define MAX_ARGS 64

// function to manage redirection < and > (Q7)
void manage_redirections(char **argv) {
    for (int j = 0; argv[j] != NULL; j++) {
        if (strncmp(argv[j], ">", 1) == 0) {
            char *filename = argv[j+1];
            if (!filename) exit(EXIT_FAILURE);
            int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1) { perror("open >"); exit(EXIT_FAILURE); }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            argv[j] = NULL;
            j++;
        }
        else if (strncmp(argv[j], "<", 1) == 0) {
            char *filename = argv[j+1];
            if (!filename) exit(EXIT_FAILURE);
            int fd = open(filename, O_RDONLY);
            if (fd == -1) { perror("open <"); exit(EXIT_FAILURE); }
            dup2(fd, STDIN_FILENO);
            close(fd);
            argv[j] = NULL;
            j++;
        }
    }
}

// function to execute command with arg
pid_t exec_command(char **argv, int input_fd, int output_fd) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        
        // if input file and output file are not standart
        if (input_fd != STDIN_FILENO) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }

        if (output_fd != STDOUT_FILENO) {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        manage_redirections(argv);
        execvp(argv[0], argv);
        
        perror("execvp");
        exit(EXIT_FAILURE);
    }

    // return the pid to use it later
    return pid;
}

int main(void) {
    char buffer[BUFFER_SIZE];
    char prompt[100] = "enseash % ";
    char *argv[MAX_ARGS]; 
    struct timespec start, end;
    int status;

    write(STDOUT_FILENO, "Bienvenue dans le Shell ENSEA.\nPour quitter, tapez 'exit'.\n", 60);

    while (1) {
        write(STDOUT_FILENO, prompt, strlen(prompt));
        ssize_t bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE);

        if (bytes_read <= 0) break;
        buffer[bytes_read - 1] = '\0';
        if (strncmp(buffer, "exit", 4) == 0 && bytes_read == 5) break;

        // Parsing
        argv[0] = strtok(buffer, " \t");
        if (argv[0] == NULL) continue;
        int i = 0;
        while (argv[i] != NULL && i < MAX_ARGS - 1) {
            i++;
            argv[i] = strtok(NULL, " \t");
        }

        // search for the Pipe command |
        int pipe_index = -1;
        for (int k = 0; argv[k] != NULL; k++) {
            if (strncmp(argv[k], "|", 1) == 0) {
                pipe_index = k;
                break;
            }
        }

        clock_gettime(CLOCK_REALTIME, &start);

        if (pipe_index != -1) {
            // if pipe
            argv[pipe_index] = NULL; // command 1
            char **argv2 = &argv[pipe_index + 1]; // pointer to command 2
            
            int pipefd[2];
            pipe(pipefd); // creating pipe

            // executing command 1 : standart in -> output in pipe
            pid_t pid1 = exec_command(argv, STDIN_FILENO, pipefd[1]);
            close(pipefd[1]);

            //executing command 2 : input from the pipe -> standard output
            pid_t pid2 = exec_command(argv2, pipefd[0], STDOUT_FILENO);
            close(pipefd[0]);

            waitpid(pid1, &status, 0);
            waitpid(pid2, &status, 0);

        } else {
            // if not pipe
            pid_t pid = exec_command(argv, STDIN_FILENO, STDOUT_FILENO); // execute standart
            waitpid(pid, &status, 0);
        }

        clock_gettime(CLOCK_REALTIME, &end);
        long elapsed_ms = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;

        if (WIFEXITED(status)) {
            snprintf(prompt, sizeof(prompt), "enseash [exit:%d|%ldms] %% ", WEXITSTATUS(status), elapsed_ms);
        } else if (WIFSIGNALED(status)) {
            snprintf(prompt, sizeof(prompt), "enseash [sign:%d|%ldms] %% ", WTERMSIG(status), elapsed_ms);
        }
    }
    
    write(STDOUT_FILENO, "Bye bye...\n", 11);
    return 0;
}
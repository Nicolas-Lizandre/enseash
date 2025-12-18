#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h> //include time.h for clock_gettime

#define BUFFER_SIZE 1024

const char WELCOME_MSG[] = "Bienvenue dans le Shell ENSEA.\nPour quitter taper 'exit'\n";
    
char prompt[100] = "enseash % ";

int main(void) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    int status;

    struct timespec start, end; // new variable for time measure

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

        clock_gettime(CLOCK_REALTIME, &start); // start chrono

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

            clock_gettime(CLOCK_REALTIME, &end); // stop chrono

            // calcul the elapsed time (milisec + nanosec)
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
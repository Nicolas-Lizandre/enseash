#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

const char WELCOME_MSG[] = "Bienvenue dans le Shell ENSEA.\nPour quitter taper 'exit'\n";
const char PROMPT[] = "enseash % ";

int main(void) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    int status;

    // print the welcome message
    write(STDOUT_FILENO, WELCOME_MSG, strlen(WELCOME_MSG)); // STDOUT_FILENO is the file descriptor
    
    while (1) {
        // print prompt
        write(STDOUT_FILENO, PROMPT, strlen(PROMPT));

        // read the command
        // read stop the programme until 'enter'
        bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE);

        // Execution of the command
        pid_t pid = fork();

        if (pid == -1) {
            // Error for fork
            write(STDERR_FILENO, "Error fork\n", 12);
        } 
        else if (pid == 0) {

            // execlp find the program in PATH.
            execlp(buffer, buffer, (char *)NULL); // first argument is file name, second is program name (argv[0]), NULL ends arguments.
            
            _exit(EXIT_FAILURE); 
        } 
        else {
            wait(&status);
        }
    }

    return 0;
}
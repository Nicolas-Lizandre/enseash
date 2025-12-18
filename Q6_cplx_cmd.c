#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFFER_SIZE 256
#define MAX_ARGUMENTS 16


const char WELCOME_MSG[] = "Welcome in the ENSEA Shell\n\rTo quit, type 'exit'\n\r";
    
char prompt[50] = "enseash %% "; // prompt is now a variable

int main(void) {
    char buffer[BUFFER_SIZE]; 
    ssize_t nb_char;
    int status;
    char arguments[MAX_ARGUMENTS][BUFFER_SIZE]; //Should have gone directly for *argv, oh well...
    int arguments_length=0;
    int last_SPACE=0;

    char *argv[MAX_ARGUMENTS];

    write(STDOUT_FILENO, WELCOME_MSG, strlen(WELCOME_MSG));
    
    while (1) {
        write(STDOUT_FILENO, prompt, strlen(prompt));
        nb_char = read(STDIN_FILENO, buffer, BUFFER_SIZE);
        buffer[nb_char-1]='\0'; //len(buffer) = nb_buffer
        //ctrl+d <-> EOF which makes read() return 0 (\0 does too)
        if(nb_char < 0){break;} //if error
        else if (nb_char == 0) {
            write(STDOUT_FILENO, "Bye bye...\n", 11); 
            break;}
        else if (strcmp(buffer, "exit") == 0) {
            write(STDOUT_FILENO, "Bye bye...\n", 11);
            break;}
        
        for(int i =0; i<nb_char-1; i++){
            if(buffer[i]==' ' && buffer[i+1]!=' '){ //Must be active on a space before a caracter and at the end //In this case aSPACE
                for(int j =last_SPACE; j<i; j++){
                arguments[arguments_length][j-last_SPACE]=buffer[j];
                }
                write(STDOUT_FILENO, arguments[arguments_length]+'\0', 16); //--Aefff
                arguments[arguments_length][i-last_SPACE]='\0'; //Close the string else they ere BUFFER_SIZE
                last_SPACE = i+1;   //Ensure we take the argument between the two spaces
                arguments_length++; //To put each argument in a separate case
            }
            if(buffer[nb_char-2]!=' '){
                for(int j =last_SPACE; j<= nb_char-2; j++){
                arguments[arguments_length][j-last_SPACE]=buffer[j];
                }
                write(STDOUT_FILENO, arguments[arguments_length]+'\0', 16); //--Aefff
                arguments[arguments_length][i-last_SPACE]='\0'; //Close the string else they ere BUFFER_SIZE
                last_SPACE = i+1;   //Ensure we take the argument between the two spaces
                arguments_length++; 
            }
        }

        for (int i = 0; i <= arguments_length; i++) { //To make arguments into argv
            argv[i] = arguments[i];}
        argv[arguments_length+1] = NULL; //Must be declared inside the chain if a v is added at execXX

        last_SPACE=0;
        arguments_length=0;

        pid_t pid = fork();

        if (pid == -1) {
            write(STDERR_FILENO, "Error fork\n", 12);
        } 
        else if (pid == 0) {
            execvp(argv[0],argv); //terminate the process here if all has gone well
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
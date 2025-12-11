#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#define MAX_BUFFER 256 
//Question 3
int main(){
    //In fact printf CALLS write to do its system call (they are the same)
    write(STDOUT_FILENO,"Welcome in the ENSEA Shell\n\r",29);//<->printf("Welcome in the ENSEA Shell\n\r");
    //For convenience purposes, I will still be using printf
    printf("To quit, type 'exit'\n\r");
    char buffer[MAX_BUFFER]; //Will store the inputed instruction
    int nb_char=0;
    int exit=false;
    while(exit == false){
        //write was here necessary, to send a string to the shell printf needs a \n
        write(STDOUT_FILENO,"enseash %%",10); //%% for one %
        //To read what's on the keyboard dynamicly, we use STDIN_FILENO who acts like a
        //file (not .txt it its own) [the same for STDOUT_FILENO].

        nb_char=read(STDIN_FILENO,buffer,sizeof(buffer-1)); //Wait UNTIL the instruction is made
        buffer[nb_char]='\0';  //To cut the string at the end else we would compare "exit  ...  "and "exit"
        if(nb_char==-1){break;}
        //No strcmp to be done
        if(strcmp(buffer,"exit\n")==0){exit=true;}
    }
    return 0;
}

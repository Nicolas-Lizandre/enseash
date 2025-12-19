#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#define BUFFER_SIZE 128 //To use strcpy
#define PROMPT_MAX_SIZE 100
#define MAX_ARGS 64
#define MAX_JOBS 16
// Data structure for the // processes
typedef struct {
int job_id;       // Job Number
pid_t pid;        // Real PID
char cmd[128];    //command launched
int to_delete; }job;   //boolean
    
job jobs[MAX_JOBS]; //needs to be actualized by main and read by sigchld_handler

void job_initialization(void){
    for (int i = 0; i < MAX_JOBS; i++) {
        jobs[i].pid=0;}
    return;}


//The main issue (and reason why we use signal) is because at first glance only the main process
//can check the children's state (fork() can't). However, we want to interrupt our prompt if a child has finished.
//The tool (function) I have seen is called signal. Signal runs in // from the main process and has access to the 
//signal sent by the main process child. The sigchld_handler will be called at each signal sent by a child terminated.



int find_job_index(pid_t pid) { //Only for sigchld_handler
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].pid == pid) {
            return i;
        }
    }
    return -1; // not found
}

void sigchld_handler(int sig) {
    int status;
    pid_t pid;
    char buffer[BUFFER_SIZE];
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) { //
        int idx = find_job_index(pid);
        if (idx == -1) {
            continue;// Not found
        }


        if (WIFEXITED(status)) {
            snprintf(buffer, BUFFER_SIZE, "[%d]+ Ended: %s [exit:%d]\n", jobs[idx].job_id, jobs[idx].cmd, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            snprintf(buffer, BUFFER_SIZE, "[%d]+ Ended: %s [sign:%d]\n", jobs[idx].job_id, jobs[idx].cmd, WTERMSIG(status));        }
        //write is only to be done if it's process in // (called also job) then
        write(STDOUT_FILENO, buffer, strlen(buffer));
        jobs[idx].to_delete = 1;
    }
}

int nb_of_active_jobs(void){
    int nb=0;
    for(int i=0;i<MAX_JOBS;i++){
        if(jobs[i].pid!=0 &&jobs[i].to_delete!=1){//We initialize every pid at 0;
            nb++;
        }
    }
    return nb;
}

int new_job_management(job new_job){
    char buffer[BUFFER_SIZE];
    int placed = 0;

    for(int i = 0; i < MAX_JOBS; i++){
        if(jobs[i].pid == 0 || jobs[i].to_delete == 1){
            new_job.job_id = i + 1;
            jobs[i] = new_job;
            placed = 1;
            break;
        }
    }

    if(!placed) return 1;

    snprintf(buffer, BUFFER_SIZE, " [%d] %d\n", new_job.job_id, new_job.pid);
    write(STDOUT_FILENO, buffer, strlen(buffer));
    return 0;
}

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
    job_initialization();
    char buffer[BUFFER_SIZE];
    char prompt[PROMPT_MAX_SIZE] = "enseash % ";
    char *argv[MAX_ARGS]; 
    struct timespec start, end;
    int status;
    job j1;
    j1.job_id=0;
    j1.to_delete=0;

    signal(SIGCHLD, sigchld_handler); //Now at each SIGCHLD  (when child process ends)

    write(STDOUT_FILENO, "Bienvenue dans le Shell ENSEA.\nPour quitter, tapez 'exit'.\n", 60);

    int Job_overflow_error = 0;

    while (1) {//Change the prompt if processes in //
        if(nb_of_active_jobs()==0){snprintf(prompt,PROMPT_MAX_SIZE,"enseash %%");}
        else{snprintf(prompt,PROMPT_MAX_SIZE,"enseash[%d&] %%", nb_of_active_jobs());}


        write(STDOUT_FILENO, prompt, strlen(prompt));


        ssize_t bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE);
        
        if (bytes_read == -1 && errno == EINTR) { //If a write interrupt the read the program should continue 
        continue;}
        if (bytes_read <= 0) break;
        buffer[bytes_read - 1] = '\0';
        if (strncmp(buffer, "exit", 4) == 0 && bytes_read == 5) break;
        if(Job_overflow_error==1){write(STDOUT_FILENO, "ERROR : Job overflow\n", BUFFER_SIZE); break;}

        int ampersand_index = -1;
        size_t len = strlen(buffer);

        // search for the // process command &
        for (int k = 0; k < len; k++) {
        if (buffer[k] == '&') { 
        ampersand_index = k;
        break;
    }
}

        // delete & before the command (knowing & exists is enough)
        if (len > 0 && buffer[len - 1] == '&') {
        buffer[len - 1] = '\0';
        }
        

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
            if(ampersand_index==-1){
            waitpid(pid1, &status, 0);
            waitpid(pid2, &status, 0);
            }
            else{
                waitpid(pid1, &status, WNOHANG);
                waitpid(pid2, &status, WNOHANG);
                j1.pid=pid1;
                strncpy(j1.cmd, buffer, sizeof(j1.cmd));
                j1.cmd[sizeof(j1.cmd) - 1] = '\0'; //to convert into char[]
                Job_overflow_error=new_job_management(j1);
                j1.pid=pid2;
                strncpy(j1.cmd, buffer, sizeof(j1.cmd));
                j1.cmd[sizeof(j1.cmd) - 1] = '\0';
                Job_overflow_error= new_job_management(j1);

            }
        }
/*typedef struct {
int job_id;       // Job Number
pid_t pid;        // Real PID
char cmd[128];    //command launched
int to_delete; }job;   //boolean*/
        
        else {
            // if not pipe and &
            pid_t pid = exec_command(argv, STDIN_FILENO, STDOUT_FILENO); // execute standart
             if(ampersand_index==-1){

            waitpid(pid, &status, 0);}
            else{
            j1.pid=pid;
            strncpy(j1.cmd, buffer, sizeof(j1.cmd));
            j1.cmd[sizeof(j1.cmd) - 1] = '\0';
            Job_overflow_error= new_job_management(j1);
            }
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
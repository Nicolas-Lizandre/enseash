# TP de Synthèse : Ensea in the Shell

* POLONOWSKI Nicolas
* FADE Lizandre

**Group :** 2G1TD2TP4
**Date :** 11/12/2025

---
**Question 1 :**
* In order to print the welcome message, we use the command write with arguments STDOUT_FILENO, that is the file descriptor, and WELCOME_MSG, witch is define before as a 'const char'.

**Question 2 :**
* To read the command we use "read" with arguments STDIN_FILENO, file descriptor for input, and the buffer. Then we remplace '\n' by '\0' at the end of the buffer to end the lecture of the buffer and make a valid string. Then we do a fork and use execlp to execute the command.

**Question 3 :**
* To exit the shell when we type 'exit' or ctrl + D we read the buffer and the bytes_read. If bytes_read is null, it's because we type ctrl + D so break to exit the loop, else we compare string to see if buffer contains 'exit' and if bytes_read is exactly 5 bytes long (e, x, i, t, \n)

**Question 4 :**
* In order to show the return code or signal we read the status of the pid. We use WIFEXITED is the execution end properly, else we use WIFSIGNALED is the programm has been killed. Then we modify the prompt, adding the value of the status (prompt is now a variable)

**Question 5 :**
* In order to measure the time of execution we use time.h and clock_gettime with CLOCK_REALTIME. We get the time before the fork and after the wait, the we calcul the elapsed time and print it in the prompt

**Question 6 :**
* To execute a command with arguments we need to parse the buffer with strtok (separator are " " and '\t'). First we give buffer as argument to read the commands, the we give NULL because strtok know where it stops so with arg NULL it continues where it stops. The last argv is NULL.
* Then we use execvp with argv[0] in first arguments (it's the main command), then the list of argv.

**Question 7 :**
* To manage redirection, we check in the list of argv is there is a "<" or ">". If so, the next argument in the list is the file name. Then we open the file we right rules (read or write) and we switch the standart file STDIN_FILENO or STDOUT_FILENO with our file opened, then we close it.
When we execute the command, it use the file we choose.
To test if it work we execute the command : ls > test.txt, in the file test.txt we can see the list of our file. The we execute echo "Hello world" > hello.txt, and wc -w < hello.txt, it returns 2, it the number of word in the file hello.txt.

**Question 8 :**
To do the Q8, first we create two function, manage_redirection and exec_command. The first one is to manage redirection as Q7 and the second one takes the list of arguments and the input and output file to execute the command.

In order to pipe, we first check if there is "|" in the list argv. If so, we remplace "|" by "NULL", so when we excute the first command with argv it stops at NULL. We execute the command with the file created by the pipe.

---
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#define NUMCHLD 5 

void sigHandler(int signum){
    printf("I'm terminating...\n");
}

int main()
{
    int status;
    
    for(int i = NUMCHLD; i--;){
        pid_t p = fork();
        
        if(p == 0){
            signal(SIGCHLD, sigHandler);
            // kill(getpid(), SIGCHLD);
            printf("Child PID: %d\n", getpid());
            exit(i);
        }else if (p != 0){
            wait(&status);
            printf("Parent PID: %d, terminated child status: %d\n", getpid(), status>>8);
        }else if(p == -1){
            printf("Error on the fork.\n");
        }
    } 
    return 0;
}
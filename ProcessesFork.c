// Write a program that forks, waits for its child to successfully print a message to the screen, 
// and prints another message before exiting.
// Modify the previous program so that a given number of children 
// are generated on the command line. What happens to the other children?


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int main()
{
    int status;
    int numberOfChildren = 10;
    pid_t p = fork();
    
    if (p == 0){
        printf("Child pid: %d \n", getpid());
        exit(1);
    } else if (p > 0) {
        wait(&status);
        printf("Parent pid: %d, child ended with status %d \n", getpid(), status);
        
        // pid_t p2;
        // int status2;
        // for(int i = 0; i < numberOfChildren; i++){
        //     p2 = fork();
            
        //     if (p2 == 0){
        //       printf("Child %d with pid: %d \n", i,getpid());
        //       exit(0);
        //     }else if (p2 > 0){
        //         wait(&status2);
        //     }
        // }
        
        exit(0);
    } else if (p == -1){
        puts("Error on the fork.");
    }

    return 0;
}

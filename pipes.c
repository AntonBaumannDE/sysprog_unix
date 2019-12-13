/*--- Anton Baumann - Sysprog in Unix - Using Pipes for reading and writing 50 numbers ------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_NUMBER  50  //max count
#define READ  0
#define WRITE 1

int main( void )
{
    int pid;                    // child
    int wr2rd[2];               // pipe for write to read
    pipe(wr2rd);                // create pipe                   
    
    pid = fork();               // fork for reading process
                                // let's read
    if (pid == 0) {
        char bufferRD;          // buffer storage
        int i = 0;      
        close(wr2rd[WRITE]);    //close off writing end of pipe since we want to read
        while (read(wr2rd[READ], &bufferRD, 1) != -1 && bufferRD != -1) {   //READ PIPE IF ALLOWED/POSSIBLE
            printf(" %2d", bufferRD);
            ++i;
            if (i % 10 == 0)    //clean print statements down to ten in a row
                printf("\n");
        }

        printf("Reading is exiting...\n");
        exit(0);
    }
    else {
        char bufferWR;    // buffer storage for write -> read
        int number = 0;   
        close(wr2rd[READ]);  //close off reading end of pipe
        while (number < MAX_NUMBER) {
            bufferWR = ++number;
            write(wr2rd[WRITE], &bufferWR, 1); //WRITE DATA IN PIPE    
        }

        bufferWR = -1;
        write(wr2rd[WRITE], &bufferWR, 1);
        int status;
        waitpid(pid, &status, 0); //wait for reading to end
        printf("Writing is exiting...\n");
    }
    return 0;
}

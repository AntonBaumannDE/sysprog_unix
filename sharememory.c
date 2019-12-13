/*--- Anton Baumann - Sysprog in Unix - Using Shared Memory for reading and writing 50 numbers in ten steps ------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define MAX_NUMBER  50  // max count of numbers being transferred
#define MAY_READ  (shm->sync == 1) //self-explainatory status bytes
#define MAY_WRITE (shm->sync == 0)
#define MAY_RUN   (shm->sync != -1)

typedef struct {                // Building our SHMStruct
    volatile signed char sync;  // magic-synchronization byte 
    char data[10];              // data-array to store our ten numbers
} SHMStruct;

/*--- Some functions ------------------------------------------*/

void errorMsg(char *fkt) {
    perror(fkt);
    fprintf(stderr, "Exiting with status 1...\n");
    exit(1);
}

int main( void )
{
    int shmDes;               // init filedescriptor of shared memory
    int pid;                  // PID of child process
    SHMStruct *shm;           // so we can access the shared memory
    const char *name="/shm";  // name of shared memory

    shmDes = shm_open(name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); //create filedescriptor of shm                    
    ftruncate(shmDes, sizeof(SHMStruct)); // set size of shared memory
    shm = mmap(NULL, sizeof(SHMStruct), PROT_READ | PROT_WRITE , MAP_SHARED , shmDes, 0); // init shm into virtual storage
    shm->sync = 0; // In order to start writing we need to set the sync byte to 0
    pid = fork(); // fork in order to read
    if (pid == 0) {     // reading in childprocess
        while (MAY_RUN) {
            if (MAY_READ) {                 //reading
                int i;
                for (i = 0; i < 10; ++i)
                    printf(" %2d", shm->data[i]);
                printf("\n");
                shm->sync = 0;              //done reading, set sync byte to 0 for writing again
            }
        }
        printf("Reading process is exiting now...\n");
        munmap(shm, sizeof(SHMStruct));  //take shm out of virtual storage
        exit(0);
    }
    else {                                 
        int number = 1;   // Now we write...
        while (number < MAX_NUMBER) {
            if (MAY_WRITE) {  
                int i;   // sync-byte is 0 -> we can write our numbers into shm
                for (i = 0; i < 10; ++i) {
                    shm->data[i] = number++;
                }
                shm->sync = 1; //done writing, let's reset the byte
            }
        }
        while(shm->sync != 0) {/*NIL*/;} // Waiting until reading process is done with it's print-functions
        shm->sync = -1;                  // Wrapping things up: We're done with the last reading process and can end everything now
        printf("Write/Parent process is exiting...\n");
        munmap(shm, sizeof(SHMStruct)); //take shm out of virtual storage       
    }

    return 0;
}

/*
  To compile prog_1 ensure that gcc is installed and run the following command:
  gcc prog_1.c -o prog_1 -lpthread -lrt -Wall

  Note: this will not compile on MacOS as lrt is not available
*/
#include  <pthread.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <stdio.h>
#include  <sys/types.h>
#include  <fcntl.h>
#include  <string.h>
#include  <sys/stat.h>
#include  <semaphore.h>
#include  <sys/time.h>

#define END_OF_HEADER "end_header"

/* --- Structs --- */

typedef struct ThreadParams {
  int pipeFile[2];
  sem_t sem_read, sem_justify, sem_write;
  char message[255];
  pthread_mutex_t lock;
} ThreadParams;

/* --- Prototypes --- */

/* Initializes data and utilities used in thread params */
void initializeData(ThreadParams *params);

/* This thread reads data from data.txt and writes each line to a pipe */
void* ThreadA(void *params);

/* This thread reads data from pipe used in ThreadA and writes it to a shared variable */
void* ThreadB(void *params);

/* This thread reads from shared variable and outputs non-header text to src.txt */
void* ThreadC(void *params);

/* --- Main Code --- */
int main(int argc, char const *argv[])
{
  if(argc != 3)
  {
    printf("Please provide 2 arguments: a data file and a a source file\n");
    printf("e.g. %s data.txt src.txt\n", argv[0]);
    exit(-1);
  }

  struct timeval t1, t2;
  gettimeofday(&t1, NULL);  // Start Timer
  int err;
  pthread_t tid[3];
  pthread_attr_t attr;
  ThreadParams params;

  // Initialization
  initializeData(&params);
  pthread_attr_init(&attr);

  if(pipe(params.pipeFile) < 0)
  {
    perror("Pipe error");
    exit(-1);
  }

  // Create Threads
  if(err = pthread_create(&(tid[0]), &attr, &ThreadA, (void*)(&params)))
  {
	  perror("Error creating thread A");
    exit(-1);
  }
  
  if(err = pthread_create(&(tid[1]), &attr, &ThreadB, (void*)(&params)))
  {
	  perror("Error creating thread B");
    exit(-1);
  }

  if(err = pthread_create(&(tid[2]), &attr, &ThreadC, (void*)(&params)))
  {
	  perror("Error creating thread C");
    exit(-1);
  }
 
  // Wait on threads to finish
  if(pthread_join(tid[0], NULL) != 0)
    printf("Issue joining Thread A\n");
  if(pthread_join(tid[1], NULL) != 0)
    printf("Issue joining Thread B\n");
  if(pthread_join(tid[2], NULL) != 0)
    printf("Issue joining Thread C\n");

  return 0;
}

void initializeData(ThreadParams *params)
{
  // Initialize Sempahores
  if(sem_init(&(params->sem_read), 0, 1))
  {
    perror("Error initializing read semaphore.");
    exit(-1);
  }
  
  if(sem_init(&(params->sem_justify), 0, 0))
  {
    perror("Error initializing justify semaphore.");
    exit(-1);
  }
  
  if(sem_init(&(params->sem_write), 0, 0))
  {
    perror("Error initializing write semaphore.");
    exit(-1);
  }
}

void* ThreadA(void *params)
{
  //TODO: add your code
  
}

void* ThreadB(void *params)
{
  //TODO: add your code
  
}

void* ThreadC(void *params)
{
  //TODO: add your code

}

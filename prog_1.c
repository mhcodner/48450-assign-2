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

#define MESSAGE_SIZE 255
#define END_OF_HEADER "end_header"

/* --- Structs --- */

typedef struct ThreadParams {
  int pipeFile[2];
  sem_t sem_read, sem_justify, sem_write;
  char message[MESSAGE_SIZE];
  const char * srcFile;
  const char * dataFile;
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

  params.srcFile = argv[2];
  params.dataFile = argv[1];

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
  // Initialize Semaphores
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
  ThreadParams *parameters = params;
  char line[MESSAGE_SIZE];
    
  FILE* srcFile = fopen(parameters->srcFile, "r");
  if(!srcFile)
  {
    perror("Invalid File");
    exit(-1);
  }
  
  while(!sem_wait(&parameters->sem_read) && fgets(line, MESSAGE_SIZE, srcFile) != NULL)
  {
    write(parameters->pipeFile[1], line, strlen(line) + 1); // Write into the pipe
    sem_post(&parameters->sem_justify);
  }

  /* Close pipe and FILE* */
  close(parameters->pipeFile[1]);
  fclose(srcFile);
  return params;
}

void* ThreadB(void *params)
{
  ThreadParams *parameters = params;
  
  while(!sem_wait(&parameters->sem_justify))
  {
    read(parameters->pipeFile[0], parameters->message, MESSAGE_SIZE); // Read from the pipe
    sem_post(&parameters->sem_write);
  }
    
  close(parameters->pipeFile[0]); // Close pipe
  return params;
}

void* ThreadC(void *params)
{
  ThreadParams *parameters = params;

  FILE* dataFile = fopen(parameters->dataFile, "w");
  if(!dataFile)
  {
    perror("Invalid File");
    exit(-1);
  }

  int eoh_flag = 0; // End of header flag
  
  while(!sem_wait(&parameters->sem_write))
  {
    if(eoh_flag)
    {
      /* Put the contents of the shared buffer into file */
      fputs(parameters->message, dataFile);
    }
    else if(strstr(parameters->message, END_OF_HEADER)) // Check for the end of header
    {
      eoh_flag = 1;
    }

    sem_post(&parameters->sem_read);
  }
  
  fclose(dataFile); // Close FILE*
  return params;
}

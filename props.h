#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <semaphore.h>
#define CHUNK 100
#define SIZE 10000
#define TAKEN 1
#define FREE 0
#define NEW 2
#define READY 3
#define MAX 50
int flags[MAX];
struct Hndl{
     int msgNum;
     int state[CHUNK];
     int location[CHUNK];
     sem_t mutex;
     sem_t numLock;
};
struct Message
{
     pid_t pid;
     char message[SIZE];
};
struct Reply{
     int msgNum;
     int location[CHUNK];
     sem_t numLock;
};
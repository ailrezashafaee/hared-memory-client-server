#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <semaphore.h>
#include <pthread.h>

#define CHUNK 100
#define SIZE 10000
#define TAKEN 1
#define FREE 0
#define NEW 2
struct Hndl{
     int msgNum;
     int state[CHUNK];
     int location[CHUNK];
     sem_t *mutex;
};
struct Message
{
     pid_t pid;
     char message[SIZE];
};
size_t storageSize = CHUNK * sizeof(struct Message) + sizeof(struct Hndl);
int accept(struct Hndl* hdl , int start)
{
     while(1)
     {
          if(hdl->state[start] == NEW)
          {
               hdl->state[start] = TAKEN;
               return start;
          }
          start = (start+1)%CHUNK;
     }
}
int main()
{
     int fd_a;
     printf("%ld\n" , sizeof(struct Message));
     printf("%ld\n", storageSize);
     struct Hndl *hdlPtr;
     struct Message *mesAPtr;
     fd_a = shm_open("a" ,   O_RDWR |O_CREAT   , 0777);
     if(fd_a == -1)
     {
          perror("shm open failed in server program");
          return 10;
     }
     if(ftruncate(fd_a,storageSize)==-1)
     {
          perror("ftruncate");
          return 20;
     }
     hdlPtr = (struct Hndl*)mmap(NULL ,storageSize, PROT_READ | PROT_WRITE , MAP_SHARED,fd_a ,0); 
     if((void *)hdlPtr == MAP_FAILED)
     {
          perror("mmap failed in server program");
          return 30;
     }
     hdlPtr->msgNum = 0;
     for(int i =0 ;i < CHUNK ;i++)
     {
          hdlPtr->state[i] = 0;
          hdlPtr->location[i] = -1;
     }
     hdlPtr->state[0] = 1; 
     puts("server listening to clients...");
     int newId;
     int index = 0;
     void *test;
     while (1) 
     {
          newId = accept(hdlPtr , index);
          puts("new client connected");
          index = (index + 1)%CHUNK;
          test = (void*)hdlPtr;
          test+=sizeof(struct Hndl);
          test+=sizeof(struct Message)*newId;
          mesAPtr = (struct Message*)test;
          printf("%ld",mesAPtr->pid);
     } 
     
}   

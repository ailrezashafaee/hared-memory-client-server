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
struct Hndl{
     int msgNum;
     int state[CHUNK];
     int location[CHUNK];
     sem_t *mutex;
};
struct Message
{
     pid_t pid;
     char message[SIZE-sizeof(pid_t) - 1];
};
size_t storageSize = CHUNK * SIZE + sizeof(struct Hndl);
int accept(struct Hndl* hdl , int start)
{
     while()
     {
          if(hdl->state[start] == 1)
          {
               
          }
          
     }
}
int main()
{
     int fd_a;
     printf("%ld\n" , storageSize);
     struct Hndl *hdlPtr;
     fd_a = shm_open("a" ,   O_RDWR |O_CREAT   , 0666);
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
     hdlPtr = (struct Hndl*)mmap(NULL , storageSize , PROT_READ | PROT_WRITE , MAP_SHARED,fd_a ,0); 
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
     sem_init(hdlPtr->mutex , 1 , 1);
     puts("server listening to clients...");
     while(1)
     {
          
     }              
}
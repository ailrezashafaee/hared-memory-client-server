#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
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
int findIndex(struct Hndl *hdl)
{
     for(int i =0 ;i < CHUNK ; i++)
     {
          if(hdl->state[i] == 0)
          {
               return i;
          }
     }
}
size_t storageSize = CHUNK * sizeof(struct Message) + sizeof(struct Hndl);
int main(int argc , char *argv[])
{
     pid_t clientPid = getpid();
     int fd_a , len;
     void *test;
     if(argc !=2)
     {
          printf("please enter the format : %s <text>\n", argv[0]);
          return 1;
     }
     len = strlen(argv[1]);
     if(len >= (10000 - sizeof(pid_t)-1))
     {
          printf("message if too long !\n");
          return 1;
     }
     struct Hndl *hdl;
     struct Message *ms;
     fd_a = shm_open("a" , O_RDWR |O_APPEND ,0777);
     if(fd_a==-1)
     {
          perror("shm open failed in clinet program");
          return 1;
     }
     hdl = (struct Hndl*)mmap(NULL, storageSize , PROT_READ | PROT_WRITE ,MAP_SHARED, fd_a , 0);
     if(hdl == MAP_FAILED)
     {
          perror("mmap failed in client program");
          return 1;
     }
     int ind;
     ind = findIndex(hdl);
     printf("%d\n", ind);
     hdl->location[ind] = clientPid;
     test = (void *)hdl;
     test += sizeof(struct Hndl);
     test += sizeof(struct Message)*ind;
     ms = (struct Message*)test;
     ms->pid = clientPid;
     hdl->state[ind] = 2;
     return 0;
}
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#define CHUNK 100
#define SIZE 10000
struct Hndl{
     int msgNum;
     int state[CHUNK];
     pid_t location[CHUNK];
     sem_t *mutex;
};
struct Message
{
     pid_t pid;
     char message[SIZE-sizeof(pid_t) - 1];
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
size_t storageSize = CHUNK * SIZE + sizeof(struct Hndl);
int main(int argc , char *argv[])
{
     pid_t clientPid = getpid();
     int fd_a , len;
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
     fd_a = shm_open("a" , O_RDWR ,0666);
     if(fd_a==-1)
     {
          perror("shm open failed in clinet program");
          return 1;
     }
     hdl = (struct Hndl*)mmap(NULL, storageSize , PROT_READ  | PROT_WRITE ,MAP_SHARED, fd_a , 0);
     if(hdl == MAP_FAILED)
     {
          perror("mmap failed in client program");
          return 1;
     }
     int ind;
     ind = findIndex(hdl);
     printf("%d\n", ind);
     hdl->location[ind] = clientPid;
     ms =(struct Message*)(hdl + sizeof(struct Hndl));
     ms->pid = clientPid;
     for(int i =0 ;i < len; i++)
     {
          ms->message[i] = argv[1][i];
     }
     ms->message[len] = '\0';
     hdl->state[ind] = 2;
     return 0;
}
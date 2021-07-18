#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "props.h"
int findIndex(struct Hndl *hdl)
{
     for(int i =0 ;i < CHUNK ; i++)
     {
          if(hdl->state[i] == FREE)
          {
               return i;
          }
     }
}
size_t storageSize = CHUNK * sizeof(struct Message) + sizeof(struct Hndl);
int main(int argc , char *argv[])
{
     pid_t clientPid = getpid();
     int fd_a , len, fd_b;
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
     struct Reply *reply;
     struct Message *ms , *msb;
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
     sem_wait(&hdl->mutex);
     ind = findIndex(hdl);
     hdl->state[ind] = NEW;
     sem_post(&hdl->mutex);
     printf("%d\n", ind);
     hdl->location[ind] = clientPid;
     test = (void *)hdl;
     test += sizeof(struct Hndl);
     test += sizeof(struct Message)*ind;
     ms = (struct Message*)test;
     ms->pid = clientPid;
     for(int i =0 ;i < len;i++)
     {
          ms->message[i] = argv[1][i];
     }
     ms->message[len] = '\0';
     hdl->state[ind] = READY;
     printf("process id : %d\n",  clientPid);
     fd_b = shm_open("b" , O_RDWR |O_APPEND ,0777);
     reply = (struct Reply*)mmap(NULL, storageSize , PROT_READ | PROT_WRITE ,MAP_SHARED, fd_b , 0);
     test = (void *)reply;
     test += sizeof(struct Reply);
     test+= sizeof(struct Message)*ind;
     msb= (struct Message *)test;
     while(msb->pid != clientPid);
     printf("%s\n" , msb->message);
     hdl->location[ind] = -1;
     reply->location[ind] = -1;
     sem_wait(&hdl->numLock);
     hdl->msgNum--;
     sem_post(&hdl->numLock);
     
     hdl->state[ind] = FREE;
     return 0;
}
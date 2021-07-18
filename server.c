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
#include <string.h>
#include "props.h"
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
     int fd_a , fd_b;
     printf("%ld\n" , sizeof(struct Message));
     printf("%ld\n", storageSize);
     struct Hndl *hdlPtr;
     struct Reply *reply;
     struct Message *mesAPtr, *mesBPtr;
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
     fd_b = shm_open("b" ,   O_RDWR |O_CREAT   , 0777);
     ftruncate(fd_b , storageSize);
     reply=(struct Reply*)mmap(NULL ,storageSize, PROT_READ | PROT_WRITE , MAP_SHARED,fd_b ,0);
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
          reply->location[i] = -1;
     }
     hdlPtr->state[0] = 1; 
     puts("server listening to clients...");
     int newId;
     int index = 0;
     void *test;
     char ans[SIZE];
     while (1) 
     {
          newId = accept(hdlPtr , index);
          puts("new client connected");
          index = (index + 1)%CHUNK;
          test = (void*)hdlPtr;
          test+=sizeof(struct Hndl);
          test+=sizeof(struct Message)*newId;
          mesAPtr = (struct Message*)test;
          printf("%d\n %s\n",mesAPtr->pid, mesAPtr->message);
          sprintf(&ans , "Server Reply : \nThe lenght of message is : %ld\n The pid of client is : %d\n" , strlen(mesAPtr->message) ,mesAPtr->pid);
          printf("%s\n" , ans);
          reply->location[newId] = mesAPtr->pid;
          test = (void *)reply;
          test += sizeof(struct Reply);
          test += sizeof(struct Message)*newId;
          mesBPtr = (struct Message*)test;
          for(int i =0 ;i<strlen(ans) ; i++)
               mesBPtr->message[i] = ans[i];
          mesBPtr->message[strlen(ans)] = '\0';
          mesBPtr->pid = mesAPtr->pid;

          puts("client done");
     } 
     
}  
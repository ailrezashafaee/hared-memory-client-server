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
struct ThreadArgs{
     struct Hndl*hdlPtr;
     struct Reply* reply;
     int index;
};
size_t storageSize = CHUNK * sizeof(struct Message) + sizeof(struct Hndl);
int accept(struct Hndl* hdl , int start)
{
     while(1)
     {
          if(hdl->state[start] == READY)
          {
               hdl->state[start] = TAKEN;
               return start;
          }
          start = (start+1)%CHUNK;
     }
}
void *serverReply(void*arg)
{    
     struct ThreadArgs *args = arg;
     int index = args->index;
     struct Hndl*hdlPtr = args->hdlPtr;
     struct Reply*reply = args->reply;
     void *temp;
     char ans[SIZE];
     struct Message*mesAPtr , *mesBPtr;
     temp = (void*)hdlPtr;
     temp+=sizeof(struct Hndl);
     temp+=sizeof(struct Message)*index;
     mesAPtr = (struct Message*)temp;
     printf("%d\n %s\n",mesAPtr->pid, mesAPtr->message);
     sleep(10);
     sprintf(&ans , "Server Reply : \nThe lenght of message is : %ld\nThe pid of client is : %d\n" , strlen(mesAPtr->message) ,mesAPtr->pid);
     reply->location[index] = mesAPtr->pid;
     temp = (void *)reply;
     temp += sizeof(struct Reply);
     temp += sizeof(struct Message)*index;
     mesBPtr = (struct Message*)temp;
     for(int i =0 ;i<strlen(ans) ; i++)
          mesBPtr->message[i] = ans[i];
     mesBPtr->message[strlen(ans)] = '\0';
     mesBPtr->pid = mesAPtr->pid;
     sem_wait(&reply->numLock);
     reply->msgNum++;
     sem_post(&reply->numLock);
     puts("client reply sended");
     pthread_exit(0);
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
     reply->msgNum = 0;
     for(int i =0 ;i < CHUNK ;i++)
     {
          hdlPtr->state[i] = 0;
          hdlPtr->location[i] = -1;
          reply->location[i] = -1;
     }
     sem_init(&hdlPtr->mutex , 1 , 1);
     sem_init(&hdlPtr->numLock,1 , 1);  
     sem_init(&reply->numLock, 1, 1);
     puts("server listening to clients...");
     int newId;
     int index = 0;
     void *test;
     char ans[SIZE];
     pthread_t tid[50];
     int co =0;
     struct ThreadArgs args;
     while (1) 
     {
          newId = accept(hdlPtr , index);
          hdlPtr->msgNum+=1;
          printf("number of message : %d\n" , hdlPtr->msgNum);
          puts("new client connected");
          args.index = newId;
          args.reply = reply;
          args.hdlPtr = hdlPtr;
          if(pthread_create(&tid[co], NULL, serverReply, (void *)&args) == -1)
          {
               printf("pthread_create error in server");
               return 1;
          }
          index = (index + 1)%CHUNK;
     } 
     
}
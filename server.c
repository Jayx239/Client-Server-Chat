#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#define MAX_MESSAGE_SIZE 50
#define CLIENT_KEY 10

typedef struct{
        char message[MAX_MESSAGE_SIZE];
	int RW;
}str_t;


char* message;
char* address;
int main(int argc, char** argv)
{
	int shmid;
	shmid = shmget(CLIENT_KEY,MAX_MESSAGE_SIZE,0666);
	if(shmid < 0)
	{
		printf("%d",shmid);
		perror("Error with get");
	}
	str_t* send_message;
	send_message = (str_t*) shmat(shmid,NULL,0);
	if(send_message == -1)
	{
		perror("at failed");
		exit(-1);
	}
	//printf("%s",send_message->message);
	//strcpy(send_message->message,"");
	while(1){
                if(send_message->RW == 0){
                        printf("%s",send_message->message);
                        strcpy(send_message->message,"");
                        send_message->RW = 1;
                }
        }
}

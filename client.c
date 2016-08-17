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

char* message;
char* address;
#define MAX_MESSAGE_SIZE 50
#define CLIENT_KEY 10

typedef struct{
	char message[MAX_MESSAGE_SIZE];
	int RW;
}str_t;

int main(int argc, char** argv)
{
	//message = malloc(MAX_MESSAGE_SIZE);"hi there";
	key_t mem_key;
	mem_key = 1000;
	address = "/temp";
	int shmid;
	shmid = (int) shmget(CLIENT_KEY,MAX_MESSAGE_SIZE,IPC_CREAT|0666);
	if(shmid <0)
	{
		perror("Error with get");
		exit(-1);
	}
	str_t*  send_message;
	send_message = (str_t*) malloc(sizeof(str_t));
	send_message = (str_t*) shmat(shmid,NULL,0);
	if(send_message == -1)
	{
		perror("at fail");
		exit(-1);
	}
	send_message->RW = 1;
	strcpy(send_message->message,"Sending to server");
	printf("%s",send_message->message);
	
	send_message->RW = 0;
        while(1)
        {
                if(send_message->RW == 1){
                        fgets(send_message->message,MAX_MESSAGE_SIZE,stdin);
                        send_message->RW = 0;
                }
        }
	return 0;	
}

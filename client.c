/* Author: Naga Kandasamy
 *  * Last modified: 01/14/2014
 *   *
 *    *  Illustrates memory mapping and persistency, with POSIX objects.The client reads and displays a message left it in the 
 *     *  memory segment image by the server, a file been mapped from a memory segment.
 *      *
 *       *  Compile as follws: 
 *        *
 *         *  gcc -o client client_code.c -lrt
 *          *
 *           */

#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>
#include "data_types.h"


#define SHARED_OBJECT_PATH         "/messenger"      
/* maximum length of the content of the message */
#define MAX_MESSAGE_LENGTH      50
/* how many types of messages we recognize */
#define MESSAGE_TYPES               8

int Connected = 0;
/* message structure for messages in the shared segment */
/*struct msg_s {
    int type;
    char content[MAX_MESSAGE_LENGTH];
};*/


int main(int argc, char *argv[]) {
	char Uid[MAX_ID_LEN];
	//strcpy(Uid,argv[1]);
	strcpy(Uid,"jpg77");
	
    int fd;
    int shared_seg_size = (sizeof(msg_packet_t));   /* We want a shared segment capable of storing one message */
    msg_packet_t* shared_msg;      /* The shared segment */
    

    /* Open the shared memory object using shm_open()  in read-only mode */
    fd = shm_open(SHARED_OBJECT_PATH, O_RDWR, S_IRWXU | S_IRWXG);
    if (fd < 0) {
        perror("In shm_open()");
        exit(1);
    }
    printf("Opened shared memory object %s\n", SHARED_OBJECT_PATH);
    
    /* requesting the shared segment using mmap() */    
    shared_msg = (struct msg_packet_t*)mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_msg == NULL) {
        perror("In mmap()");
        exit(1);
    }


	
	//open_connection(Uid,&shared_msg);
	// Open Connection
	int is_connected;
	is_connected  = CONNECT;
        while(is_connected != CONNECTED)
	{
	pthread_mutex_lock(&shared_msg->mutex_lock);
	if(shared_msg->message_type != NULL_MESSAGE)
	{
		pthread_mutex_unlock(&shared_msg->mutex_lock);
		continue;
	}
	shared_msg->message_type = SERVER_MESSAGE;
        strcpy(shared_msg->sender_id, Uid);
        strcpy(shared_msg->receiver_id,"");
        shared_msg->connection = CONNECT;
	is_connected = CONNECTED;
        pthread_mutex_unlock(&shared_msg->mutex_lock);
	}
	

	
	// Close Connection
	while(is_connected == CONNECTED)
	{
	pthread_mutex_lock(&shared_msg->mutex_lock);
		if(shared_msg->message_type != NULL_MESSAGE)
		{
			pthread_mutex_unlock(&shared_msg->mutex_lock);
			continue;
		}
	shared_msg->message_type = SERVER_MESSAGE;
        strcpy(shared_msg->sender_id, Uid);
        strcpy(shared_msg->receiver_id,"");
        shared_msg->connection = DISCONNECT;
	is_connected = DISCONNECT;
        pthread_mutex_unlock(&shared_msg->mutex_lock);
	}

	//pthread_mutex_lock(&shared_msg->mutex_lock);
/*	int i;
	i= 0;
	while(1)
	{
		if(shared_msg->receiver_id == Uid)
		{
			printf("%s: %s\n",shared_msg->sender_id, shared_msg->message);
			strcpy(shared_msg->receiver_id,"");
			//strcpy(shared_msg->sender_id,Uid)
			i=SERVER_MESSAGE;
		}
		if(i==-1)
		{
			strcpy(shared_msg->receiver_id,"");
			strcpy(shared_msg->sender_id,Uid);
			
			
		}
	}
  */  //printf("Shared memory segment allocated correctly (%d bytes).\n", shared_seg_size);


//    printf("Message type is %s, content is: %s\n", (char*) shared_msg->sender_id, (char*) shared_msg->receiver_id);

	  /* [uncomment if you wish] requesting the removal of the shm object     --  shm_unlink() */
    /*if (shm_unlink(SHARED_OBJECT_PATH) != 0) {
        perror("In shm_unlink()");
        exit(1);
    }*/

    return 0;
}


void open_connection(char uid[MAX_ID_LEN], msg_packet_t* shared_msg)
{
	//while(shared_msg->message_type != NULL_MESSAGE)
	//{
		
	//}
	pthread_mutex_lock(shared_msg->mutex_lock);
	shared_msg->message_type = SERVER_MESSAGE;
	strcpy(shared_msg->sender_id, uid);
	strcpy(shared_msg->receiver_id,"");
	shared_msg->connection = CONNECT;
	pthread_mutex_unlock(shared_msg->mutex_lock);
	//while(shared_msg->connection == CONNECT)
	//{

	//}
	//if(shared_msg->connection == DISCONNECT)
	//{
		
//		shared_msg->connection = NULL;
	//}
}


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

void open_connection(char uid[MAX_ID_LEN], msg_packet_t* shared_msg);

void close_connection(char uid[MAX_ID_LEN], msg_packet_t* shared_msg);


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


	// Open Connection
	open_connection(Uid,shared_msg);
	
	/* 
 	Messaging code
 	while(condition)
	{
		
	}*/

	// Close connection
	close_connection(Uid,shared_msg);
	

    return 0;
}


void open_connection(char Uid[MAX_ID_LEN], msg_packet_t* shared_msg)
{
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

}

void close_connection(char Uid[MAX_ID_LEN], msg_packet_t* shared_msg)
{
	int is_connected;
	is_connected = CONNECTED;
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

}

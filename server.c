/*  Author: Naga Kandasamy
 *   *  Last modified: 01/14/2014
 *    *  
 *     *  server_code.c
 *      *  
 *       *  This code snippet shows the use of memory mapping and persistency with POSIX objects. The server process generates a message 
 *        *  and leaves it in a shared segment. The segment is mapped in a persistent object meant to be subsequently open by a shared 
 *         *  memory client. The shared object is created and stored in the file system under /dev/shm
 *          *
 *           *  Compile as follows: 
 *            *
 *             *  gcc -o server server_code.c -lrt
 *              *
 *               *
 *                */

#define _BSD_SOURCE

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
#define MAX_MESSAGE_LENGTH      50
#define MESSAGE_TYPES               8

int NumClients = 0;
char Clients[MAX_CLIENTS][MAX_ID_LEN];
/* message structure for messages in the shared segment */
/*struct msg_s {
    int type;
    char content[MAX_MESSAGE_LENGTH];
};*/


int main(int argc, char *argv[]) {
    int fd;
    int shared_seg_size = (sizeof(msg_packet_t));   /* We want a shared segment capable of storing one message */
    msg_packet_t* shared_msg;      /* the shared segment, and head of the messages list */
    

    /* Create the shared memory object using shm_open(). On Linux, by default it is created inside of /dev/shm. */
    fd = shm_open(SHARED_OBJECT_PATH, O_CREAT | O_EXCL | O_RDWR, S_IRWXU | S_IRWXG);
    if (fd < 0) {
        perror("In shm_open()");
        exit(1);
    }
    fprintf(stderr, "Created shared memory object %s\n", SHARED_OBJECT_PATH);
    
    /* Adjust mapped file size (make room for the whole segment to map) using ftruncate(). */
    ftruncate(fd, shared_seg_size);

    /* Request the shared segment using mmap(). */    
    shared_msg = (msg_packet_t*)mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_msg == NULL) {
        perror("In mmap()");
        exit(1);
    }
    fprintf(stderr, "Shared memory segment allocated correctly (%d bytes).\n", shared_seg_size);
	pthread_mutexattr_t* mutex_attr;
	pthread_mutexattr_init(&mutex_attr);
	pthread_mutexattr_setpshared(&mutex_attr,PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&shared_msg->mutex_lock,&mutex_attr);
	pthread_mutex_lock(&shared_msg->mutex_lock);
	shared_msg->message_type = NULL_MESSAGE;
	pthread_mutex_unlock(&shared_msg->mutex_lock);
	while(1)
	{
		
		pthread_mutex_lock(&shared_msg->mutex_lock);
		// New Client being added
		if(shared_msg->connection == CONNECT)
		{
			if(NumClients < 10)
			{
				strcpy(Clients[NumClients++],shared_msg->sender_id);
				//NumClients++;
			}
			int i;
			for(i=0; i<NumClients; i++)
				printf("New Connection, Client: %s\n",Clients[i]);
			shared_msg->connection = CONNECTED;
			shared_msg->message_type = NULL_MESSAGE;
			pthread_mutex_unlock(&shared_msg->mutex_lock);
			continue;
		}
	
		if(shared_msg->connection == DISCONNECT)
		{	
			int i;
			int client_found = -1;
			for(i=0; i<NumClients; i++)
			{
				if(strcmp(Clients[i],shared_msg->sender_id) == 0)
				{
					client_found = i;
					continue;
				}
				if(client_found > -1)
				{
					strcpy(Clients[i-1],Clients[i]);
				}
			}
			shared_msg->message_type = NULL_MESSAGE;
			if(--NumClients == 0)
			{
				pthread_mutex_unlock(&shared_msg->mutex_lock);
				break;
			}
		}
		
		pthread_mutex_unlock(&shared_msg->mutex_lock);		
	}
    
    /* [uncomment if you wish] requesting the removal of the shm object     --  shm_unlink() */
	// Remove shared memory

     if (shm_unlink(SHARED_OBJECT_PATH) != 0) {
        	perror("In shm_unlink()");
                exit(1);
	}

    return 0;
}



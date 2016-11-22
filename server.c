/*  Author: Jason Gallagher 
 *    *  ECEC-353 Systems Programming Assignment 3 - Summer 2016
 *     *  server.c
 *      *  
 *          *
 *           *  Compile as follows: 
 *            *
 *             *  gcc -o server server.c -lrt
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
#include <signal.h>

#define SHARED_OBJECT_PATH         "/messenger"
#define MAX_MESSAGE_LENGTH      50
#define MESSAGE_TYPES               8

int NumClients;
char Clients[MAX_CLIENTS][MAX_ID_LEN];
group_list_t Groups[MAX_CLIENTS];
int Keep_Alive;

void send_direct_message(msg_packet_t* shared_msg);
void send_group_message(msg_packet_t* shared_msg);
void wait_for_response(msg_packet_t* shared_msg);
int connect_client(msg_packet_t* shared_msg);
int disconnect_client(msg_packet_t* shared_msg);
int group_contains(int group_id);
void send_error_message(msg_packet_t* shared_msg, int error_type);
int valid_recipient(char receiver_id[MAX_ID_LEN]);
void clean_exit(int dum);


int main(int argc, char *argv[]) {
	int fd;
	int shared_seg_size = (sizeof(msg_packet_t));   /* We want a shared segment capable of storing one message */
	msg_packet_t* shared_msg;      /* the shared segment, and head of the messages list */
	NumClients = 0; 
	Keep_Alive = 1;
	signal(SIGINT,clean_exit);

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
	while(Keep_Alive)
	{

		pthread_mutex_lock(&shared_msg->mutex_lock);
		// New Client being added
		if(shared_msg->connection == CONNECT)
		{
			connect_client(shared_msg);	
			pthread_mutex_unlock(&shared_msg->mutex_lock);
			continue;
		}

		if(shared_msg->connection == DISCONNECT)
		{	
			disconnect_client(shared_msg);
			if(NumClients == 0)
			{
				pthread_mutex_unlock(&shared_msg->mutex_lock);
				break;
			}
		}
		// Message has been sent by client
		if(shared_msg->message_type != NULL_MESSAGE)
		{
			if(shared_msg->message_type == DIRECT_MESSAGE)
			{
				printf("Recieved message from %s for %s: %s",shared_msg->sender_id,shared_msg->receiver_id,shared_msg->message);
				if(valid_recipient(shared_msg->receiver_id) == 1)
				{
					pthread_mutex_unlock(&shared_msg->mutex_lock);
					send_direct_message(shared_msg);
					pthread_mutex_lock(&shared_msg->mutex_lock);
				}
				else
				{
					pthread_mutex_unlock(&shared_msg->mutex_lock);
					send_error_message(shared_msg,INVALID_RECIPIENT);
					pthread_mutex_lock(&shared_msg->mutex_lock);
				}
			}

			if(shared_msg->message_type == GROUP_MESSAGE)
			{
				printf("Recieved message from %s to group: %s",shared_msg->sender_id,shared_msg->message);
				pthread_mutex_unlock(&shared_msg->mutex_lock);
				send_group_message(shared_msg);
				pthread_mutex_lock(&shared_msg->mutex_lock);

			}
			if(shared_msg->message_type != SERVER_MESSAGE)
				shared_msg->message_type = NULL_MESSAGE;

		}

		pthread_mutex_unlock(&shared_msg->mutex_lock);		
	}

	// Remove shared memory
	if (shm_unlink(SHARED_OBJECT_PATH) != 0) {
		perror("In shm_unlink()");
		exit(1);
	}

	return 0;
}

int connect_client(msg_packet_t* shared_msg)
{
	if(NumClients < 10)
	{
		char group_status_msg[MAX_MESSAGE_LEN];
		if(group_contains(shared_msg->group_id) == 0)
		{
			snprintf(group_status_msg,MAX_MESSAGE_LEN,"Chat group %i not found...\nCreating group %i\n",shared_msg->group_id,shared_msg->group_id);
		}
		else
		{
			snprintf(group_status_msg,MAX_MESSAGE_LEN,"Chat group %i joined\n",shared_msg->group_id);
		}
		strcpy(Groups[NumClients].u_id,shared_msg->sender_id);
		Groups[NumClients].group_id = shared_msg->group_id;
		strcpy(Clients[NumClients++],shared_msg->sender_id);


		strcpy(shared_msg->receiver_id,shared_msg->sender_id);
		strcpy(shared_msg->sender_id,"SERV");
		strcpy(shared_msg->message,group_status_msg);

		printf("New Connection, Client: %s Group: %d\n",Clients[NumClients-1], shared_msg->group_id);
	}
	else
	{
		strcpy(shared_msg->receiver_id,shared_msg->sender_id);
		strcpy(shared_msg->sender_id,"SERV");		
		strcpy(shared_msg->message,SERVER_FULL_MESSAGE);
		printf("Connection Attemp, Client: %s Group: %d\nServer full... Connection Denied\n",Clients[NumClients-1], shared_msg->group_id);

	}
	int i;
	shared_msg->connection = CONNECTED;
	shared_msg->message_type = SERVER_MESSAGE;
}

int group_contains(int group_id)
{
	int i;
	for(i = 0; i<NumClients; i++)
	{
		if(Groups[i].group_id == group_id)
			return 1;
	}

	return 0;
}
int disconnect_client(msg_packet_t* shared_msg)
{
	int i;
	int client_found = -1;
	for(i=0; i<NumClients; i++)
	{
		if(strcmp(Clients[i],shared_msg->sender_id) == 0)
		{
			strcpy(Clients[i],"     ");
			strcpy(Groups[i].u_id, "     ");
			client_found = i;
			continue;
		}
		if(client_found > -1)
		{
			strcpy(Clients[i-1],Clients[i]);
			Groups[i-1] = Groups[i];
		}
	}
	shared_msg->connection = CONNECTED;
	shared_msg->message_type = NULL_MESSAGE;
	NumClients--;
}

void send_direct_message(msg_packet_t* shared_msg)
{
	printf("sending direct message\n");
	pthread_mutex_lock(&shared_msg->mutex_lock);
	printf("recip: %s sender: %s\n",shared_msg->receiver_id,shared_msg->sender_id);

	shared_msg->message_type = SERVER_MESSAGE;
	pthread_mutex_unlock(&shared_msg->mutex_lock);
	wait_for_response(shared_msg);
	printf("Response Received\n");
}

void send_group_message(msg_packet_t* shared_msg)
{
	pthread_mutex_lock(&shared_msg->mutex_lock);
	printf("Sending group message to group: %d\n",shared_msg->group_id);
	pthread_mutex_unlock(&shared_msg->mutex_lock);
	int i;
	for(i=0; i < NumClients; i++)
	{	
		pthread_mutex_lock(&shared_msg->mutex_lock);
		if(strcmp(shared_msg->sender_id,Clients[i]) == 0 || shared_msg->group_id != Groups[i].group_id)
		{	
			pthread_mutex_unlock(&shared_msg->mutex_lock);
			continue;
		}
		printf("Sent to %s... ", Clients[i]);
		shared_msg->message_type = SERVER_MESSAGE;
		strcpy(shared_msg->receiver_id,Clients[i]);
		pthread_mutex_unlock(&shared_msg->mutex_lock);
		wait_for_response(shared_msg);
		printf("Response Received\n");
	}
	printf("Sent group message\n");

}

void wait_for_response(msg_packet_t* shared_msg)
{
	while(Keep_Alive)
	{
		pthread_mutex_lock(&shared_msg->mutex_lock);
		if(shared_msg->message_type != SERVER_MESSAGE)
		{
			pthread_mutex_unlock(&shared_msg->mutex_lock);
			break;
		}
		pthread_mutex_unlock(&shared_msg->mutex_lock);
	}
	pthread_mutex_unlock(&shared_msg->mutex_lock);
}

int valid_recipient(char receiver_id[MAX_ID_LEN])
{
	int i;
	for(i=0; i<NumClients; i++)
	{
		if(strcmp(receiver_id,Clients[i]) == 0)
			return 1;
	}
	printf("Invalid Recipient: %s\n",receiver_id);
	return 0;
}


void send_error_message(msg_packet_t* shared_msg, int error_type)
{
	pthread_mutex_lock(&shared_msg->mutex_lock);
	printf("Sending Error Message: %d\n",error_type);
	if(error_type == INVALID_RECIPIENT)
	{
		char rec_id[MAX_ID_LEN];
		strcpy(rec_id,shared_msg->receiver_id);
		strncpy(shared_msg->receiver_id,shared_msg->sender_id,MAX_ID_LEN);
		strcpy(shared_msg->sender_id,"ERROR");
		shared_msg->message_type = SERVER_MESSAGE;
		snprintf(shared_msg->message,MAX_MESSAGE_LEN,"Invalid Recipient Entered: %s\n",rec_id);	
		pthread_mutex_unlock(&shared_msg->mutex_lock);

		wait_for_response(shared_msg);
		pthread_mutex_lock(&shared_msg->mutex_lock);
		printf("Response Received\n");
	}
	pthread_mutex_unlock(&shared_msg->mutex_lock);
}

void clean_exit(int dum)
{
	Keep_Alive = 0;
}

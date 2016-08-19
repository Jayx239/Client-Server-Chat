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
char user_message[MAX_MESSAGE_LEN];
char dm_rec[MAX_ID_LEN];
int Keep_Alive;
pthread_mutex_t* user_input_mutex;


void open_connection(char uid[MAX_ID_LEN], msg_packet_t* shared_msg);

void close_connection(char uid[MAX_ID_LEN], msg_packet_t* shared_msg);
void send_message(msg_packet_t* shared_msg,char user_message[MAX_MESSAGE_LEN],char sender_id[MAX_ID_LEN], int MESSAGE_TYPE, char recipient[MAX_ID_LEN]);
void* read_user_input(void* args);

/* message structure for messages in the shared segment */
/*struct msg_s {
    int type;
    char content[MAX_MESSAGE_LENGTH];
};*/


int main(int argc, char *argv[]) {
	char Uid[MAX_ID_LEN];
	if(argc >1)
		strcpy(Uid,argv[1]);
	else
		strcpy(Uid,"foofag");
	
	Keep_Alive = 1;
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
	pthread_t* user_input_thread;
	strcpy(user_message,"");
	
	pthread_mutex_init(&user_input_mutex,NULL);
	int rc = pthread_create(&user_input_thread,NULL,read_user_input,(void*) NULL);
//	pthread_join(&user_input_thread,NULL);
	int MESSAGE_TYPE;
	MESSAGE_TYPE = GROUP_MESSAGE;
 	//Messaging code
 	while(Keep_Alive)
	{
		// Listen for incoming messages
		pthread_mutex_lock(&shared_msg->mutex_lock);
		
		// direct message not setup, issue comparing user ids
		if(shared_msg->message_type == SERVER_MESSAGE)
		{
			//char rec_id[MAX_ID_LEN];
			//strcpy(rec_id,shared_msg->receiver_id);
			//int match;
			//match = 1;
			//int i;
			//for(i=0; i<MAX_ID_LEN; i++)
			//{
			//	if(shared_msg->receiver_id[i] != Uid[i])
			//		match = 0;
			//}
			//match = memcmp(shared_msg->receiver_id,Uid);
			//if(match == 0)
			//{
				printf("%s: %s",shared_msg->sender_id,shared_msg->message);
				shared_msg->message_type = RESPONSE_MESSAGE;
			//}
		}
		pthread_mutex_unlock(&shared_msg->mutex_lock);
		
		// Read user input and send message when completely entered
		pthread_mutex_lock(&user_input_mutex);
		if(strcmp(user_message,"") != 0)
		{
			send_message(shared_msg,user_message,Uid, MESSAGE_TYPE, dm_rec);
			strcpy(user_message,"");
		} 
		pthread_mutex_unlock(&user_input_mutex);	
	}

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

void send_message(msg_packet_t* shared_msg,char user_message[MAX_MESSAGE_LEN],char sender_id[MAX_ID_LEN], int MESSAGE_TYPE, char recipient[MAX_ID_LEN])
{
	int msg_type;
	msg_type  = SERVER_MESSAGE;
	while(msg_type != NULL_MESSAGE)
	{
		pthread_mutex_lock(&shared_msg->mutex_lock);
		if(shared_msg->message_type == NULL_MESSAGE)
		{
			shared_msg->message_type = MESSAGE_TYPE;
			break;
		}
		pthread_mutex_unlock(&shared_msg->mutex_lock);
	}	
	
	if(MESSAGE_TYPE == DIRECT_MESSAGE)
		strcpy(shared_msg->receiver_id,recipient); 
	
	strcpy(shared_msg->sender_id,sender_id);
	strcpy(shared_msg->message,user_message);
	pthread_mutex_unlock(&shared_msg->mutex_lock);
}

void* read_user_input(void* args)
{
	int user_message_set;
	int MESSAGE_TYPE;
	MESSAGE_TYPE = GROUP_MESSAGE;
	while(1)
	{
	char temp_message[MAX_MESSAGE_LEN];
	fgets(temp_message,MAX_MESSAGE_LEN,stdin);
	 if(temp_message[0]== '-' && temp_message[1] == 'e')
                        break;
                if(temp_message[0] == '-' && temp_message[1] == 'd' && temp_message[2] == 'm')
                {
                        printf("Enter recipient for direct message");
                        fgets(dm_rec,MAX_ID_LEN,stdin);
                        MESSAGE_TYPE = DIRECT_MESSAGE;
                }
                else
                        strcpy(dm_rec,"");
		user_message_set = 1;
		while(user_message_set){
			pthread_mutex_lock(&user_input_mutex);
			if(strcmp(user_message,"") == 0)
			{
				strcpy(user_message,temp_message);
				user_message_set = 0;
			}
			pthread_mutex_unlock(&user_input_mutex);
		}
	}
	Keep_Alive = 0;
}

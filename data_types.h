#ifndef DATA_TYPES_H_ 
#define DATA_TYPES_H_
#include <pthread.h> 

#define EXIT_COMMAND "-e 123"
#define DIRECT_MESSAGE_COMMAND "-dm"

#define MAX_ID_LEN 6
#define MAX_MESSAGE_LEN 50
#define MAX_CLIENTS 10


#define NULL_MESSAGE 0
#define DIRECT_MESSAGE 1
#define GROUP_MESSAGE 2
#define SERVER_MESSAGE 3
#define RESPONSE_MESSAGE 4

#define DISCONNECT -1
#define CONNECTED 0
#define CONNECT 1

typedef struct message_packet{
	int message_type;      // 1= group message 0 = direct message
	char sender_id[MAX_ID_LEN];
	char receiver_id[MAX_ID_LEN];
	char message[MAX_MESSAGE_LEN];
	int connection;	//Used for adding/removing clients
	pthread_mutex_t* mutex_lock;
}msg_packet_t;

#endif 

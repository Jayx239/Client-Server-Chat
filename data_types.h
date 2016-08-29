#ifndef DATA_TYPES_H_ 
#define DATA_TYPES_H_
#include <pthread.h> 

// User command fields
#define EXIT_COMMAND "-e"
#define EXIT_COMMAND_LEN 2
#define DIRECT_MESSAGE_COMMAND "-dm"
#define DIRECT_MESSAGE_COMMAND_LEN 3
// length fields
#define MAX_ID_LEN 6
#define MAX_MESSAGE_LEN 50
#define MAX_CLIENTS 10

// Message type keys
#define NULL_MESSAGE 0
#define DIRECT_MESSAGE 1
#define GROUP_MESSAGE 2
#define SERVER_MESSAGE 3
#define RESPONSE_MESSAGE 4

// Connection message keys
#define DISCONNECT -1
#define CONNECTED 0
#define CONNECT 1

// Error Message Keys
#define INVALID_RECIPIENT 0



typedef struct message_packet{
	int message_type;
	char sender_id[MAX_ID_LEN];
	char receiver_id[MAX_ID_LEN];
	char message[MAX_MESSAGE_LEN];
	int connection;	//Used for adding/removing clients
	pthread_mutex_t* mutex_lock;
}msg_packet_t;

typedef struct group_list{
	char u_id;
	int group_num;
}group_list_t;

#endif 

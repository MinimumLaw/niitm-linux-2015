#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
/* project include here */
#include <chat_proto.h>
#include <srv_client.h>
#include <message_fmt.h>
#include <srv_message_store.h>

messages_store*	chat_messages;
client_list*	clients;
chat_message*	system_head;
int	port;

/* FixMe: this is readconfig() function stub*/
int init_basic_params(void)
{
    chat_messages = NULL;
    system_head = NULL;

    if((chat_messages = (messages_store *)malloc(sizeof(messages_store)))
	    == NULL) {
	perror("malloc (message_store)");
	return -1;
    }
    
    chat_messages->messages = NULL; /* allocate later */
    system_head = NULL;
    
    if(pthread_mutex_init(&chat_messages->mutex,NULL)) {
	perror("pthread_mutex_init (chat_messages)");
	return -1;
    }
    chat_messages->store_len = MAX_HISTORY_LEN;

    if((clients = (client_list*)malloc(sizeof(client_list))) == NULL) {
	perror("malloc (client list)");
	return -1;
    }

    clients->list = NULL;

    if(pthread_mutex_init(&clients->mutex,NULL)) {
	perror("pthread_mutex_init (clients)");
	return -1;
    }

    return 0;
}

int allocate_required_memory(void)
{
    int i;
    chat_message* curr = NULL;
    chat_message* temp = NULL;

    for(i=0; i<chat_messages->store_len; i++) {
	chat_message* next;

	next = (chat_message *)malloc(sizeof(chat_message));
	if(next==NULL) {
	    perror("malloc (message ring buffer)");
	    return -1;
	}
	next->next = NULL;
	next->isEmpty = true;
	if(curr) {
	    curr->next = next;
	    next->prev = curr;
	    curr = next;
	} else {
	    next->prev = NULL;
	    next->next = NULL;
	    curr = next;
	}
    }

    temp = curr; /* curr - last message in buff */
    while(temp->prev) temp = temp->prev; /* temp - first */
    
    curr->next = temp;	/* create ring buffer for messages */
    temp->prev = curr;	/* with max deep = store_len */
    chat_messages->messages = curr;

    system_head = chat_messages->messages;

    return 0;
}

int main(int argc, char** argv, char** env)
{
    if(argc != 2) {
	printf("USAGE:\n\t%s <listen_port>\n", argv[0]);
	return -1;
    }

    port = atoi(argv[1]);
    if(port<1 || port>65535) {
	fprintf(stderr,"Port must be in range 1..65535\n");
	return -1;
    }

    if(init_basic_params()) {
	fprintf(stderr,"basic init failed\n");
	return -1;
    };

    if(allocate_required_memory()) {
	fprintf(stderr,"allocate required momory failed\n");
	return -1;
    };

    return 0;
}
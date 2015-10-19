#ifndef _INCLUDE_SVR_CLIENT_H_
#define _INCLUDE_SVR_CLIENT_H_

#include <sys/socket.h>
#include <message_fmt.h>

typedef struct tag_ClientList {
    struct tag_ClientList* next;
    struct tag_ClientList* prev;
    chat_message* head;
    char alias[ALIAS_LEN+1];
    int skt;
} chat_client;

typedef struct {
    chat_client* list;
    pthread_mutex_t	mutex;
} client_list;

extern client_list* clients;

int client_add_list(chat_client* client);
int client_remove_list(chat_client *client);

#endif

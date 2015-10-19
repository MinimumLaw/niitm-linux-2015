#include <srv_client.h>

client_list* clients;

int client_add_list(chat_client* client)
{
    chat_client* curr;

    pthread_mutex_lock(&clients->mutex);
    if(clients->list == NULL) {
	client->prev = NULL;
	client->next = NULL;
	clients->list = client;
    } else {
	curr = clients->list;
	while(curr->next) 
	    curr = curr->next;
	client->prev = curr;
	client->next = NULL;
	curr->next = client;
    }
    pthread_mutex_unlock(&clients->mutex);
    return 0;
}

int client_remove_list(chat_client *client)
{
    pthread_mutex_lock(&clients->mutex);
    if(client->next) { /* not last */
	client->next->prev = client->prev;
	if(client->prev) /* not first */
	    client->prev->next = client->next;
    } else if (client->prev) { /* last */
	client->prev->next = client->next; /* NULL */
    } else { /* single client next==NULL and prev==NULL */
	clients->list = NULL;
    }
    pthread_mutex_unlock(&clients->mutex);
    return 0;
}

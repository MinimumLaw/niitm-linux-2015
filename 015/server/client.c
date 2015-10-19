/*
 * Per client pthread
 */

#include <string.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <srv_client.h>
#include <srv_message_store.h>
#include <chat_proto.h>

static int max_msg_len = TEXT_LEN + ALIAS_LEN + sizeof(chat_msg_header);

#define DEFAULT_CLIENT_ALIAS "guest"

void* client_pthread(void* arg)
{
    char* buff;
    chat_client* client = (chat_client*)arg;
    /*
     * We got client allocated client structure with init socket and head
     */


    fprintf(stdout,"Client pthread: started\n");
    buff = malloc(max_msg_len);
    if(buff == NULL) {
	perror("malloc (client buffer)");
	return NULL;
    };
    
    strcpy(client->alias, DEFAULT_CLIENT_ALIAS);
    fprintf(stdout,"Client set default name ");
    fprintf(stdout,DEFAULT_CLIENT_ALIAS);
    fprintf(stdout,"\n");
    
    client_add_list(client);
    fprintf(stdout,"Client added to client list\n");

    while(true){
	int count;
	chat_msg_header* hdr;
	
	if(ioctl(client->skt, FIONREAD, &count) < 0) {
	    perror("ioctl (client)");
	    goto client_exit;
	}
	
	if(count>0) { /* have data from client */
	    read(client->skt, buff, max_msg_len);

	    hdr = (chat_msg_header*)buff;
	    if(hdr->proto < CHAT_PROTO_VERSION) {
		fprintf(stderr,"Unsupported client version: 0x%02X\n", 
		    hdr->proto);
		continue;
	    }
	    switch(hdr->cmd) {
	    case SET_ALIAS: {
		    msg_set_alias* req = (msg_set_alias*)buff;
		    msg_your* ans = (msg_your*)buff;
		    strcpy(client->alias,req->alias);
		    ans->header.cmd = YOUR;
		    strcpy(ans->alias, client->alias);
		    write(client->skt, buff, sizeof(msg_your));
		} break;
	    case SEND_MESSAGE: {
		    msg_send_message* req = (msg_send_message*)buff;
		    pthread_mutex_lock(&chat_messages->mutex);
		    strcpy(system_head->alias,client->alias);
		    strcpy(system_head->text,req->text);
		    system_head->isEmpty = false;
		    client->head = client->head->next;
		    system_head = system_head->next;
		    pthread_mutex_unlock(&chat_messages->mutex);
		} break;
	    case LIST_ALIASES: { /* who online */
		    chat_client* curr = clients->list;
		    pthread_mutex_lock(&clients->mutex);
		    while(curr){
			msg_online* ans = (msg_online*)buff;
			ans->header.cmd = ONLINE;
			strcpy(ans->alias, curr->alias);
			curr = curr->next;
		    }
		    pthread_mutex_unlock(&clients->mutex);
		} break;
	    case GET_ALIAS: { /* who am i */
		    msg_your* ans = (msg_your*)buff;
		    ans->header.cmd = YOUR;
		    strcpy(ans->alias, client->alias);
		    write(client->skt, buff, sizeof(msg_your));
		} break;
	    case HISTORY: {
		    msg_history* req = (msg_history*)buff;
		    if(req->deep > chat_messages->store_len)
			req->deep = chat_messages->store_len;
		    while(req->deep--)
			client->head = client->head->prev;
		} break;
	    case DISCONNECT: {
		    msg_text_message* ans = (msg_text_message*)buff;
		    ans->header.cmd = MESSAGE;
		    strcpy(ans->alias, "chat server");
		    strcpy(ans->text, "By! Have a nice day!\n");
		    write(client->skt, buff, sizeof(msg_text_message));
		    goto client_exit;
		} break;
	    default:
		fprintf(stderr,"Unsupported client cmd: 0x%02X\n", hdr->cmd);
		break;
	    };
	}
	
	if(client->head != system_head) { /* have data for client */
	    msg_text_message* ans = (msg_text_message*)buff;

	    if(!client->head->isEmpty){
		ans->header.proto = CHAT_PROTO_VERSION;
		ans->header.cmd = MESSAGE;
		strcpy(ans->alias, client->head->alias);
		strcpy(ans->text, client->head->text);
		if(write(client->skt, buff, sizeof(msg_text_message)) < 0)
		    goto client_exit;
	    }
	    client->head = client->head->next;
	}
    }

client_exit:

    /*
     * We disconnected from client or other clitical error happend
     */
    free(buff);
    client_remove_list(client);
    free(client);
    return 0;
}
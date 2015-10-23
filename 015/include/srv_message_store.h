#ifndef _INCLUDE_SRV_MESSAGE_STORE_H_
#define _INCLUDE_SRV_MESSAGE_STORE_H_

#include <message_fmt.h>

typedef struct tagMessageList {
    chat_message*	messages;
    pthread_mutex_t	mutex;
    size_t		store_len;
} messages_store;

extern messages_store*	chat_messages;
extern chat_message*	system_head;

#endif

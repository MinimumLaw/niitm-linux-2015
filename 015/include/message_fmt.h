#ifndef _INCLUDE_MESSAGE_FMT_H_
#define _INCLUDE_MESSAGE_FMT_H_

#include <pthread.h>
#include <stdbool.h>

#define ALIAS_LEN	63
#define TEXT_LEN	511

#define MAX_HISTORY_LEN	10

typedef struct tag_MessageFormat {
    struct tag_MessageFormat* next;
    struct tag_MessageFormat* prev;
    char alias[ALIAS_LEN+1];
    char text[TEXT_LEN+1];
    bool isEmpty;
} chat_message;

#endif

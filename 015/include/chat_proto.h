#ifndef _INCLUDE_CHAT_PROTO_H_
#define _INCLUDE_CHAT_PROTO_H_

#include <message_fmt.h>
#include <stdint.h>

#define CHAT_PROTO_VERSION (0x01)

enum {
    /* messages from client to server */
    SET_ALIAS = 0x01,
    SEND_MESSAGE = 0x02,
    LIST_ALIASES = 0x03,
    GET_ALIAS = 0x04,
    HISTORY = 0x05,
    DISCONNECT = 0x06,
    /* messages from server to client */
    ONLINE = 0x81,
    YOUR = 0x82,
    MESSAGE = 0x83,
};

/* common message header */
typedef struct {
    uint8_t	proto;
    uint8_t	cmd;
} chat_msg_header;

/*
 * client to server message format
 */

typedef struct {
    chat_msg_header header;
    char	alias[ALIAS_LEN+1];
} msg_set_alias;

typedef struct {
    chat_msg_header header;
    char	text[TEXT_LEN+1];
} msg_send_message;

typedef struct { /* LIST_ALIASES not requread any args */
    chat_msg_header header;
} msg_list_aliases;

typedef struct { /* GET_ALIAS not requiread any args */
    chat_msg_header header;
} msg_get_alias;

typedef struct { /* FixMe: max 255 message at this moment */
    chat_msg_header header;
    uint8_t	deep;
} msg_history;

typedef struct { /* DISCONNECT not requiread any args */
    chat_msg_header header;
} msg_disconnect;

/*
 * Server to client message format
 */

/* ONLINE answer for GET_ALIASES. Send once for any connected client */
typedef struct {
    chat_msg_header header;
    char	alias[ALIAS_LEN+1];
} msg_online;

/* YOUR send on SET_ALIAS and GET_ALIAS and contens server setted nickname */
typedef struct {
    chat_msg_header header;
    char	alias[ALIAS_LEN+1];
} msg_your;

/* TEXT_MESSAGE sended async, on new messages on server side detected */
typedef struct {
    chat_msg_header header;
    char	alias[ALIAS_LEN+1];
    char	text[TEXT_LEN+1];
} msg_text_message;

#endif
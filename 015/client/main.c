#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sched.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
/* project specific headers */
#include <chat_proto.h>

#ifdef DEBUG
#define debug(args...)	fprintf(stdout, ##args)
#else
#define debug(args...)
#endif
#define info(args...)	fprintf(stdout, ##args)
#define error(args...)	fprintf(stdout, ##args)

/* FixMe: 640kb should be enough for everyone :-) */
#define BUFFER_SIZE	(578)
char buff[BUFFER_SIZE];
char srv_buff[BUFFER_SIZE];
bool isConnected;
int skt;

const char delim[] = " \t\n";

int translate_user_message(char* user_data)
{
    char* cmd;
    char* arg;

    cmd = strtok(user_data,delim);
    if(cmd==NULL) cmd=user_data; /* single cmd, no args */
    if(!strncmp(cmd,"send",4)) {
	if(isConnected) {
	    msg_send_message* msg = (msg_send_message*)srv_buff;

	    msg->header.proto = CHAT_PROTO_VERSION;
	    msg->header.cmd = SEND_MESSAGE;

	    arg = strtok(NULL,"");
	    strncpy(msg->text, arg, TEXT_LEN);
	    write(skt,srv_buff,sizeof(msg_send_message));
	    debug("Send: %s\n", arg);
	} else {
	    info("Disconnected! Connect first!\n");
	}
    } else if (!strncmp(cmd,"connect",7)) {
	char* server;
	char* port;
	struct hostent *hp;
	int   port_nr;
	struct sockaddr_in srv_addr;

	server = strtok(NULL, delim);
	port = strtok(NULL, delim);

	if(server && port) {
	    port_nr = atoi(port); /* FixMe: sscanf must be here */

	    srv_addr.sin_family = AF_INET;
	    srv_addr.sin_port = htons(port_nr);
	    hp = gethostbyname(server);
	    if(hp) {
		bcopy( hp->h_addr, &(srv_addr.sin_addr.s_addr), hp->h_length);
		if(!connect(skt, (struct sockaddr*)&srv_addr,
			sizeof(srv_addr))) {
		    info("Connected!\n");
		    isConnected = true;
		} else {
		    isConnected = false;
		    perror("connect");
		    error("Connect to host %s on port %d failed!\n",
			server, port_nr);
		}
	    }else {
		info("Host %s not found!\n", server);
	    }
	} else 
	    info("server and port must be specifed\n");
    } else if (!strncmp(cmd,"disconnect",10)) {
	if(isConnected) {
	    msg_disconnect* msg = (msg_disconnect*)srv_buff;

	    msg->header.proto = CHAT_PROTO_VERSION;
	    msg->header.cmd = DISCONNECT;

	    write(skt,srv_buff,sizeof(msg_disconnect));
	    debug("Send disconnect request\n");
	} else {
	    info("Already disconnected!\n");
	}
    } else if (!strncmp(cmd,"alias",5)) {
	    msg_set_alias* msg = (msg_set_alias*)srv_buff;

	    msg->header.proto = CHAT_PROTO_VERSION;
	    msg->header.cmd = SET_ALIAS;

	    arg = strtok(NULL,delim);
	    if(arg) {
		strncpy(msg->alias, arg, ALIAS_LEN);
		if(isConnected){
		    write(skt,srv_buff,sizeof(msg_set_alias));
		    debug("Send new alias %s\n", arg);
		} else {
		    info("Connect first!\n");
		}
	    } else {
		info("Require new alias!\n");
	    }
    } else if (!strncmp(cmd,"history",7)) {
	    msg_history* msg = (msg_history*)srv_buff;
	    int nr;

	    msg->header.proto = CHAT_PROTO_VERSION;
	    msg->header.cmd = HISTORY;

	    arg = strtok(NULL,delim);
	    if(arg) {
		nr = atoi(arg); /* FixMe: sscanf must be here */

		if(nr>0) {
		    if(nr>MAX_HISTORY_LEN)
			nr=MAX_HISTORY_LEN;
		    msg->deep = nr;
		    if(isConnected){
			write(skt,srv_buff,sizeof(msg_history));
			debug("Send last %d message request\n", nr);
		    } else {
			info("Connect first!\n");
		    }
		} else {
		    info("Hystory argument error!\n");
		}
	    } else {
		info("Require number of messages!\n");
	    }
    } else if (!strncmp(cmd,"whoami",6)) {
	    msg_get_alias* msg = (msg_get_alias*)srv_buff;

	    msg->header.proto = CHAT_PROTO_VERSION;
	    msg->header.cmd = GET_ALIAS;

	    if(isConnected){
		write(skt,srv_buff,sizeof(msg_get_alias));
		debug("Send whoami request.\n");
	    } else {
		info("Connect first!\n");
	    }
    } else if (!strncmp(cmd,"online",6)) {
	    msg_list_aliases* msg = (msg_list_aliases*)srv_buff;

	    msg->header.proto = CHAT_PROTO_VERSION;
	    msg->header.cmd = LIST_ALIASES;

	    if(isConnected){
		write(skt,srv_buff,sizeof(msg_list_aliases));
		debug("Send online user list request.\n");
	    } else {
		info("Connect first!\n");
	    }
    } else if (!strncmp(cmd,"exit",4)) {
	_exit(0);
    } else if (!strncmp(cmd,"quit",4)) {
	_exit(0);
    } else if (!strncmp(cmd,"help",4)) {
	info("Supported commands:\n");
	info("\tconnect <server> <port>\n");
	info("\t\tconnect to <server> on <port>\n");
	info("\tdisconnect\n");
	info("\t\tdisconnect from server\n");
	info("\talias <new_alias>\n");
	info("\t\tset my alias on server to <new_alias>\n");
	info("\twhoami\n");
	info("\t\tshow my alias on server\n");
	info("\tonline\n");
	info("\t\tshow who is online\n");
	info("\thelp\n");
	info("\t\tshow this help message\n");
    } else {
	info("unknown command %s, try help ;-)\n", cmd);
    }
    return 0;
};

int translate_server_message(char* server_data)
{
    chat_msg_header* hdr = (chat_msg_header*)server_data;

    if(hdr->proto == CHAT_PROTO_VERSION) {
	switch(hdr->cmd) {
	case ONLINE: {
		msg_online* msg = (msg_online*)server_data;
		info("ONLINE: %s\n", msg->alias);
	    } break;
	case YOUR: {
		msg_your* msg = (msg_your*)server_data;
		info("ALIAS: %s\n", msg->alias);
	    } break;
	case KICK: {
		msg_kick_message* msg = (msg_kick_message*)server_data;
		info("%s: %s", msg->alias, msg->text);
		close(skt);
		skt = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(skt<0){
		    perror("socket");
		    return -1;
		};
		isConnected = false;
	    } break;
	case MESSAGE: {
		msg_text_message* msg = (msg_text_message*)server_data;
		info("%s: %s", msg->alias, msg->text);
	    } break;
	default:
	    fprintf(stdout, "Unsupported command 0x%02X\n",hdr->cmd);
	}
    } else
	fprintf(stdout,
		"Unsupported protocol version 0x%02X received!\n", hdr->proto);

    return 0;
}

int main(int argc, char** argv, char** env)
{
    isConnected = false;

/* FixMe: Don't want args parsing. Use connect command in client.
    if(argc != 4){
	printf("USAGE:\n\t%s <server_addr> <server_port> <name>\n", argv[0]);
	return -1;
    } */

    skt = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(skt<0){
	perror("socket");
	return -1;
    };

    write(STDOUT_FILENO,"CHAT > ", 7);

    while(true){
	int isUserInput = 0;
	int isServerInput = 0;

	if(isConnected) {
	    if (ioctl(skt, FIONREAD, &isServerInput) < 0) {
		perror("ioctl (is server data present)");
		sched_yield();
		continue;
	    }

	    if(isServerInput){
		int sz;
		/* FixMe: check read return value */
		sz = read(skt, srv_buff, BUFFER_SIZE);
		debug("Server send %d bytes\n", sz);
		translate_server_message(srv_buff);
	    }
	}

	if (ioctl(0, FIONREAD, &isUserInput) < 0) {
	    perror("ioctl (is stdin data present)");
	    sched_yield();
	    continue;
	}

	if(isUserInput){
	    fgets(buff,BUFFER_SIZE,stdin);
	    translate_user_message(buff);
	    write(STDOUT_FILENO,"CHAT > ", 7);;
	}
	sched_yield();
    }

    return 0;
}

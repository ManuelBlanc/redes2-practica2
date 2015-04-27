
#ifndef CLIENTFILE_H
#define CLIENTFILE_H

/* std */
#include <stdio.h>
/* posix */
#include <pthread.h>
#include <netinet/in.h>
/* usr */
#include "G-2301-05-P3-client.h"

typedef struct ClientRecv {
	char             	filename[50];
	struct in_addr   	ip;
	unsigned int     	port;
	unsigned long int	file_size;
	int              	active;
} ClientRecv;

int clientfile_set_connection_params(char* msg);
int clientfile_is_active(void);
void clientfile_delete_connection_params(void);

int clientfile_send(Client* cli, char* nick, char* path);
int clientfile_recv(void);

#endif /* CLIENTFILE_H */

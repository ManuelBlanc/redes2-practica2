
/* std */
#include <stdio.h>
#include <getopt.h>

/* unistd */
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

/* usr */
#include "G-2301-05-P2-config.h"
#include "G-2301-05-P2-user.h"
#include "G-2301-05-P2-channel.h"

int maxfd = 0; /*Maximo descriptor de socket abierto*/

static void usage(int code) {
	printf(stderr, "usage: %s [-hv]\n", "G-2301-05-P2-server")
	exit(code);
}

static void procesar_opciones(int argc, char * const *argv) {
	static struct option longopts[] = {
		{ "verbose",  	no_argument,      	NULL,      	'v'	},
		{ "help",     	no_argument,      	NULL,      	'h'	},
		{ "buffy",    	no_argument,      	NULL,      	'b'	},
		{ "fluoride", 	required_argument,	NULL,      	'f'	},
		{ "daggerset",	no_argument,      	&daggerset,	1  	},
		{NULL,0,NULL,0}
	};
	while (1) {
		int opt = getopt_long(argc, argv, "vh", NULL, NULL);
		switch (opt) {
			/* Opciones */
			case 'v': /* verbosity++;        	*/ break;
			case 'h': /* usage(EXIT_SUCCESS);	*/ break;
			case 'n': /*                     	*/ break;
			case 'f': /*                     	*/ break;
			/* Especiales */
			case -1: return;   /* argument list exhausted */
			case 0:  continue; /* flag option set */
			/* Errores */
			default:
			case ':': /* missing option argument */
			case '?': /* unknown or ambiguous option */
				usage(EXIT_FAILURE);
		}
	}
}

static void demonizar(void) {
	/* para el final, para que sea facil debugear ... */
}

Server* server_new(){
	Server* serv = malloc(sizeof(Server*));
	return serv;
}

void server_init(void) {
	struct sockaddr_in addr;

	Server* serv = server_new();

	addr.sin_family     	= AF_INET;
	addr.sin_addr.s_addr	= INADDR_ANY;
	addr.sin_port       	= 0;

	serv->sock = socket(AF_INET, SOCK_STREAM, 0);
	bind(sock, (struct sockaddr*) &addr, sizeof addr);
	listen(sock, 3); // Maximo 3 peticiones de conexion encoladas

	socklen_t len = sizeof addr;
	getsockname(sock, (struct sockaddr*) &addr, &len);

	pthread_create(&serv->select_thr, NULL, &server_select, serv);
	pthread_detach(serv->select_thr);

	while (1) {
		server_accept(serv);
	}
}

void server_add_new_sockdesc(Server* serv, int sock) {
	FD_SET(sock, &(serv->fd_read));
	if (sock > maxfd) maxfd = sock;
}

void server_remove_sockdesc(Server* serv, int sock) {
	FD_CLR(sock, &(serv->fd_read));
}

void server_select(Server* serv) {
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	int ret, i;
	User* usr = serv->usrs;
	FD_ZERO(&(serv->fd_read));
	while (1) {
		ret = select(maxfd + 1, &(serv->fd_read), NULL, NULL, &tv);
		if(ret == -1 || errno == EINTR) break;
		for(i = 0; i < ret; i++, usr = usr->next){
			if (FD_ISSET(user_get_socket(usr), &(serv->fd_read)) {
				exe_msg(serv, usr, /*comando que tiene que leer el server, de donde lo cogemos???*/);
      			}
		}
	}
}

int server_accept(Server* serv){
	struct sockaddr_in user_addr;
	socklen_t usrlen = sizeof user_addr;

	int sock = accept(serv->sock, (struct sockaddr*) &client_addr, &usrlen);
	User* user = user_new(serv, sock);
	server_add_user(serv, user);
	return OK;
}

int server_is_nick_used(Server* serv, const char* nick) {
	if(NULL == userlist_findByName(list, nick)) return ERR;
	return OK;
}

int server_add_user(Server* serv, User* user) {
   	userlist_insert(serv->usrs, user);
	server_add_new_sockdesc(serv, sock);/*getter del socket??????*/
	return OK;
}

int server_delete_user(Server* serv, const char* name) {
	/*cde?????????????*/
	User* usr = userlist_findByName(UserList list, const char* name);
	server_remove_sockdesc(serv, /*getter del socket de usuario*/);
	return OK;
}

int server_add_channel(Server* serv, const char* chan) {
   	channellist_insert(serv->chan, chan);
	return OK;
}

int main(int argc, char const *argv[])
{
	procesar_opciones(argc, argv);
	demonizar();
	server_init();

	return 0;
}

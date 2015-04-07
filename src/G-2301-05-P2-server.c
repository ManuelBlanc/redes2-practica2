
/* std */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

/* unistd */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* usr */
#include "G-2301-05-P2-config.h"
#include "G-2301-05-P2-util.h"
#include "G-2301-05-P2-server.h"
#include "G-2301-05-P2-user.h"
#include "G-2301-05-P2-channel.h"

struct Server {
	int            	sock;                         	/* Socket que recibe peticiones  */
	char           	hostname[SERVER_MAX_NAME_LEN];	/* Nombre del servidor           */
	User*          	usrs;                         	/* Lista de usuarios             */
	Channel*       	chan;                         	/* Lista de canales              */
	pthread_mutex_t	switch_mutex;                 	/* Hilo para la funcion select() */
	ServerAdmin    	admin_data;                   	/* Datos del administrador	*/
};

int maxfd = 0; /*Maximo descriptor de socket abierto*/

static void usage(int code) {
	fprintf(stderr, "usage: %s [-hv]\n", "G-2301-05-P2-server");
	exit(code);
}

static void procesar_opciones(int argc, char** argv) {
	static struct option longopts[] = {
		{ "verbose", 	no_argument,      	NULL,	'v'	},
		{ "help",    	no_argument,      	NULL,	'h'	},
		{ "buffy",   	no_argument,      	NULL,	'b'	},
		{ "fluoride",	required_argument,	NULL,	'f'	},
		{NULL,0,NULL,0}
	};
	while (1) {
		int opt = getopt_long(argc, argv, "vh", longopts, NULL);
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

Server* server_new() {
	Server* serv = ecalloc(1, sizeof *serv);
	pthread_mutex_init(&serv->switch_mutex, NULL);
	strncpy(serv->name,           	"GNB.himym", SERVER_MAX_NAME_LEN);
	strncpy(serv->admin_data.loc1,	"Nueva York, USA", 200);
	strncpy(serv->admin_data.loc2,	"Goliath National Bank", 200);
	strncpy(serv->admin_data.email, "barney@awesome.himym", 200);
	return serv;
}

void server_init(void) {
	struct sockaddr_in addr;

	Server* serv = server_new();

	addr.sin_family     	= AF_INET;
	addr.sin_addr.s_addr	= INADDR_ANY;
	addr.sin_port       	= 0;

	serv->sock = socket(AF_INET, SOCK_STREAM, 0);
	bind(serv->sock, (struct sockaddr*) &addr, sizeof addr);
	listen(serv->sock, 3); // Maximo 3 peticiones de conexion encoladas

	socklen_t len = sizeof addr;
	getsockname(serv->sock, (struct sockaddr*) &addr, &len);

	while (1) {
		LOG("Esperando peticiones de conexion");
		server_accept(serv);
	}
}

void server_down_semaforo(Server* serv) {
	if (serv == NULL) return;
	pthread_mutex_lock(&serv->switch_mutex);
}

void server_up_semaforo(Server* serv) {
	if (serv == NULL) return;
	pthread_mutex_unlock(&serv->switch_mutex);
}

int server_accept(Server* serv){
	struct sockaddr_in user_addr;
	socklen_t usrlen = sizeof user_addr;

	int sock = accept(serv->sock, (struct sockaddr*) &user_addr, &usrlen);
	User* user = user_new(serv, sock);
	return server_add_user(serv, user);
}

// Devuelve el topic.
int server_get_name(Server* serv, char** name) {
	if (serv == NULL) return ERR;
	*name = serv->name;
	return OK;
}

int server_get_admin(Server* serv, ServerAdmin* sa) {
	if (serv == NULL) return ERR;
	*sa = serv->admin_data;
	return OK;
}

UserList server_get_userlist(Server* serv) {
	if(NULL == serv) return NULL;
	return (&serv->usrs);
}

ChannelList server_get_channellist(Server* serv) {
	if(NULL == serv) return NULL;
	return (&serv->chan);
}

int server_is_nick_used(Server* serv, const char* nick) {
	if(NULL == userlist_findByName(&serv->usrs, nick)) return ERR;
	return OK;
}

int server_add_user(Server* serv, User* user) {
   	userlist_insert(&serv->usrs, user);
	return OK;
}

int server_delete_user(Server* serv, const char* name) {
	UserList usr = userlist_findByName(&serv->usrs, name);
	if (usr == NULL) return ERR;
	User* usr2 = userlist_extract(usr);
	user_delete(usr2);
	return OK;
}

int server_add_channel(Server* serv, const char* name) {
	ChannelList chan = channellist_findByName(&serv->chan, name);
	if (chan != NULL) return ERR;
	channellist_insert(&serv->chan, channel_new(serv, name));
	return OK;
}

int server_delete_channel(Server* serv, const char* name) {
	ChannelList chan = channellist_findByName(&serv->chan, name);
	if (chan == NULL) return ERR;
	Channel* chan2 = channellist_extract(chan);
	channel_delete(chan2);
	return OK;
}

int main(int argc, char** argv)
{
	LOG("Comenzando ejecucion");
	procesar_opciones(argc, argv);
	demonizar();
	server_init();

	return 0;
}

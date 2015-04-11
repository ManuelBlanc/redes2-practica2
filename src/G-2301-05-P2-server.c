
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
#include <arpa/inet.h>
#include <signal.h>

/* usr */
#include "G-2301-05-P2-config.h"
#include "G-2301-05-P2-util.h"
#include "G-2301-05-P2-server.h"
#include "G-2301-05-P2-user.h"
#include "G-2301-05-P2-channel.h"

struct Server {
	int 		num_chan; 			/* Numero de canales operativos	 */
	int 		num_users;			/* Numero de conexiones abiertas */
	int            	sock;                         	/* Socket que recibe peticiones  */
	char           	name[SERVER_MAX_NAME_LEN];	/* Nombre del servidor           */
	User*          	usrs;                         	/* Lista de usuarios             */
	User*          	out;                         	/* Usuarios desconectados	 */
	Channel*       	chan;                         	/* Lista de canales              */
	pthread_mutex_t	switch_mutex;                 	/* Mutex general		 */
	ServerAdmin    	admin_data;                   	/* Datos del administrador	 */
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
	serv->num_users = 0;
	serv->num_chan = 0;
	strncpy(serv->name,           	"GNB.himym", SERVER_MAX_NAME_LEN);
	strncpy(serv->admin_data.loc1,	"Nueva York, USA", 200);
	strncpy(serv->admin_data.loc2,	"Goliath National Bank", 200);
	strncpy(serv->admin_data.email, "barney@awesome.himym", 200);
	return serv;
}

int socket_temp_segv = -1;
static void on_segmentation_fault(int sig) {
	UNUSED(sig);
	LOG("Recibido fallo de segmentacion. Cerrando el socket... %i", close(socket_temp_segv));
	exit(EXIT_FAILURE);
}

void server_init(void) {
	int ret;
	struct sockaddr_in addr;

	Server* serv = server_new();

	signal(SIGSEGV, on_segmentation_fault);
	signal(SIGINT, on_segmentation_fault);
	signal(SIGTERM, on_segmentation_fault);

	addr.sin_family     	= AF_INET;
	addr.sin_addr.s_addr	= INADDR_ANY;
	addr.sin_port       	= htons(6667);

//CDE para socket bind y listen

	serv->sock = socket_temp_segv = socket(AF_INET, SOCK_STREAM, 0);
	LOG("Creado socket() -> %i", serv->sock);

	while (-1 == (ret = bind(serv->sock, (struct sockaddr*) &addr, sizeof addr))) {
		LOG("Fallado el bind a %s:%i, reintentando en un segundo.",
				inet_ntoa(addr.sin_addr),
				ntohs(addr.sin_port));
		sleep(1);
	}
	ret = listen(serv->sock, 3); // Maximo 3 peticiones de conexion encoladas

	socklen_t len = sizeof addr;
	getsockname(serv->sock, (struct sockaddr*) &addr, &len);

	LOG("Escuchando conexiones por %s:%i",
		inet_ntoa(addr.sin_addr),
		ntohs(addr.sin_port));

	while (1) {
		server_accept(serv);
		LOG("Aceptado una conexion!");
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
	if(sock == -1){
		return ERR;
	}
	// Si sock es -1 y errno es algo entonces hay que repetir
	if(SERVER_MAX_USERS <= serv->num_users) {
		send(sock, "Servidor al maximo de su capacidad, intentelo mas tarde.", 58, 0);
		return ERR;
	}

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

UserList server_get_disconnectlist(Server* serv) {
	if(NULL == serv) return NULL;
	return (&serv->out);
}

ChannelList server_get_channellist(Server* serv) {
	if(NULL == serv) return NULL;
	return (&serv->chan);
}

int server_is_nick_used(Server* serv, char* nick) {
	if(NULL == userlist_findByNickname(&serv->usrs, nick)) return ERR;
	return OK;
}

int server_get_num_users(Server* serv) {
	return serv->num_users;
}

int server_get_num_channels(Server* serv) {
	return serv->num_chan;
}

int server_add_user(Server* serv, User* user) {
	userlist_insert(&serv->usrs, user);
	serv->num_users++;
	return OK;
}

int server_delete_user(Server* serv, char* name) {
	UserList usr = userlist_findByNickname(&serv->usrs, name);
	if (usr == NULL) return ERR;
	User* usr2 = userlist_extract(usr);
	userlist_insert(&serv->out, usr2);
	//user_delete(usr2);
	serv->num_users--;
	return OK;
}

int server_add_channel(Server* serv, char* name) {
	ChannelList chan = channellist_findByName(&serv->chan, name);
	if (chan != NULL) return ERR;
	channellist_insert(&serv->chan, channel_new(serv, name));
	serv->num_chan++;
	return OK;
}

int server_delete_channel(Server* serv, char* name) {
	ChannelList chan = channellist_findByName(&serv->chan, name);
	if (chan == NULL) return ERR;
	Channel* chan2 = channellist_extract(chan);
	channel_delete(chan2);
	serv->num_chan--;
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

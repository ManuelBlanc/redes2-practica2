
/* std */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

/* unistd */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

/* redes2 */
#include <redes2/irc.h>
/* usr */
#include "G-2301-05-P3-ssock.h"
#include "G-2301-05-P3-config.h"
#include "G-2301-05-P3-util.h"
#include "G-2301-05-P3-server.h"
#include "G-2301-05-P3-user.h"
#include "G-2301-05-P3-channel.h"

struct Server {
	int                  	 num_chan;                 	/* Numero de canales operativos        	*/
	int                  	 num_users;                	/* Numero de conexiones abiertas       	*/
	int                  	 num_out;                  	/* Numero users desconectados guardados	*/
	char                 	 name[SERVER_MAX_NAME_LEN];	/* Nombre del servidor                 	*/
	User*                	 usrs;                     	/* Lista de usuarios                   	*/
	User*                	 out;                      	/* Usuarios desconectados              	*/
	Channel*             	 chan;                     	/* Lista de canales                    	*/
	pthread_mutex_t      	 switch_mutex;             	/* Mutex general                       	*/
	ServerAdmin          	 admin_data;               	/* Datos del administrador             	*/
	pthread_t            	 ping_thr;                 	/* Hilo para ver actividad en los users	*/
	Redes2_SSL_CTX*      	 ssl_ctx;                  	/* Contexto SSL                        	*/
	Redes2_SSL_CTX_config	 ssl_conf;                 	/* Configuracion de las conexiones SSL 	*/
};

//Envia pings periodicos a los usuarios para comprobar su actividad
static void* server_periodic_ping(void* serv_ptr) {
	Server* serv = (Server *)serv_ptr;
	while(1) {
		UserList ulist = &serv->usrs;
		server_down_semaforo(serv);
		while (1) {
			User* usr = userlist_head(ulist);
			ulist = userlist_tail(ulist);
			if (NULL == usr) break;
			char* name;
			user_get_name(usr, &name);
			LOG("Haciendo ping a %s", name);
			user_ping(usr);
			free(name);
		}
		server_up_semaforo(serv);
		sleep(10);
	}
	return NULL;
}

Server* server_new(void) {
	Server* serv = ecalloc(1, sizeof *serv);
	pthread_mutex_init(&serv->switch_mutex, NULL);
	serv->num_users = 0;
	serv->num_chan = 0;
	serv->num_out = 0;
	strncpy(serv->name,            	"GNB.himym",            	SERVER_MAX_NAME_LEN);
	strncpy(serv->admin_data.loc1, 	"Nueva York, USA",      	200);
	strncpy(serv->admin_data.loc2, 	"Goliath National Bank",	200);
	strncpy(serv->admin_data.email,	"barney@awesome.himym", 	200);
	pthread_create(&serv->ping_thr, NULL, server_periodic_ping, serv);
	pthread_detach(serv->ping_thr);

	serv->ssl_conf = (Redes2_SSL_CTX_config) {
		/* ca_file  */ "cert/root.pem",
		/* ca_path  */ NULL,
		/* key_file */ "cert/ana.key",
		/* pem_file */ "cert/ana.pem",
	};
	serv->ssl_ctx = fijar_contexto_SSL(serv->ssl_conf);

	return serv;
}

static int serverP_accept(Server* serv, int serv_sock, int secure) {
	struct sockaddr_in user_addr;
	socklen_t usrlen = sizeof user_addr;

	int sock = accept(serv_sock, (struct sockaddr*) &user_addr, &usrlen);
	if (sock == -1) return ERR;

	// Si sock es -1 y errno es algo entonces hay que repetir
	if (SERVER_MAX_USERS <= serv->num_users) {
		close(sock);
		LOG("Se ha recibido una conexion cunado el servidor estaba al maximo de su capacidad.");
		return ERR;
	}

	SSock* ss;
	if (secure) {
		Redes2_SSL* ssl = aceptar_canal_seguro_SSL(serv->ssl_ctx, sock);
		if (ERR == evaluar_post_connectar_SSL(ssl)) {
			close(sock);
			LOG("Error al intentar establecer una conexion segura.");
			return ERR;
		}
		ss = ssock_secure_new(sock, ssl);
	}
	else {
		ss = ssock_new(sock);
	}

	server_down_semaforo(serv);
		User* user = user_new(serv, ss);
		server_add_user(serv, user);
	server_up_semaforo(serv);
	return OK;
}

struct ServerListen {
	Server*   serv;
	uint16_t  port;
	int       secure;
};
static void* serverP_listen(void* sl_ptr) {
	struct ServerListen* sl = sl_ptr;
	Server*   serv   = sl->serv;
	uint16_t  port   = sl->port;
	int       secure = sl->secure;
	int ret;

	struct sockaddr_in addr;

	addr.sin_family     	= AF_INET;
	addr.sin_addr.s_addr	= INADDR_ANY;
	addr.sin_port       	= htons(port);

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	LOG("Creado socket() -> %i", sock);

	//int yes = 1;
	//setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes));

	while (ERR == (ret = bind(sock, (struct sockaddr*) &addr, sizeof addr))) {
		LOG("Fallado el bind a %s:%i, reintentando en 3 segundos.",
			inet_ntoa(addr.sin_addr),
			ntohs(addr.sin_port));
		sleep(3);
	}
	ret = listen(sock, 3); // Maximo 3 peticiones de conexion encoladas

	socklen_t len = sizeof addr;
	getsockname(sock, (struct sockaddr*) &addr, &len);

	LOG("Escuchando conexiones por %s:%i de manera %s.",
		inet_ntoa(addr.sin_addr),
		ntohs(addr.sin_port),
		secure ? "segura" : "no segura");

	while (1) {
		serverP_accept(serv, sock, secure);
		LOG("Aceptado una conexion!");
	}
	return OK;
}

int server_listen(Server* serv, uint16_t port, int secure) {
	struct ServerListen* sl = emalloc(sizeof *sl);
	sl->serv   = serv;
	sl->port   = port;
	sl->secure = secure;

	pthread_t hilo;
	if (0 > pthread_create(&hilo, 0, serverP_listen, sl)) return ERR;

	pthread_detach(hilo);
	return OK;
}

void server_down_semaforo(Server* serv) {
	if (serv == NULL) return;
	pthread_mutex_lock(&serv->switch_mutex);
}

void server_up_semaforo(Server* serv) {
	if (serv == NULL) return;
	pthread_mutex_unlock(&serv->switch_mutex);
}

// Devuelve el topic.
int server_get_name(Server* serv, char** name) {
	if (serv == NULL) return ERR;
	*name = estrdup(serv->name);
	return OK;
}

int server_get_admin(Server* serv, ServerAdmin** sa) {
	if (NULL == serv) return ERR;
	*sa = emalloc(sizeof **sa);
	memcpy(*sa, &serv->admin_data, sizeof **sa);
	return OK;
}

UserList server_get_userlist(Server* serv) {
	if (NULL == serv) return NULL;
	return (&serv->usrs);
}

UserList server_get_disconnectlist(Server* serv) {
	if (NULL == serv) return NULL;
	return (&serv->out);
}

ChannelList server_get_channellist(Server* serv) {
	if (NULL == serv) return NULL;
	return (&serv->chan);
}

int server_is_nick_used(Server* serv, char* nick) {
	return (NULL != userlist_findByNickname(&serv->usrs, nick)) ? OK : ERR;
}

int server_get_num_users(Server* serv) {
	if (NULL == serv) return ERR;
	return serv->num_users;
}

int server_get_num_channels(Server* serv) {
	if (NULL == serv) return ERR;
	return serv->num_chan;
}

int server_get_motd(char** motd_path) {
	*motd_path = estrdup("MOTD.txt");
	return OK;
}

int server_add_user(Server* serv, User* user) {
	userlist_insert(&serv->usrs, user);
	serv->num_users++;
	return OK;
}

int server_delete_user(Server* serv, char* name) {
	UserList usr = userlist_findByNickname(&serv->usrs, name);
	if (NULL == usr) return ERR;
	User* usr2 = userlist_extract(usr);
	//mirara los canales y sacar al usr si esta presente
	ChannelList chan = &serv->chan;
	while(channellist_head(chan)) {
		channel_remove_user(channellist_head(chan), usr2);
		chan = channellist_tail(chan);
	}
	user_kill(usr2);
	server_add_disconnect(serv, usr2);
	serv->num_users--;
	return OK;
}

int server_add_disconnect(Server* serv, User* usr) {
	if (serv->num_out == SERVER_MAX_OUT){
		User* aux = userlist_extract(&serv->out);
		user_delete(aux);
	} else {
		serv->num_out++;
	}
	userlist_insert(&serv->out, usr);
	return OK;
}

long server_add_or_create_channel(Server* serv, char* name, ChannelList chan) {
	// Comprobamos argumentos
	if (serv == NULL || chan == NULL) return ERR;
	if (name == NULL || name[0] == '\0') return ERR_NOSUCHCHANNEL;

	// Buscamos el canal y lo devolvemos si ya existe
	*chan = channellist_head(channellist_findByName(&serv->chan, name));
	if (*chan != NULL) return OK;

	// No existe, lo creamos
	// Primero comprobamos que el nombre sea valido
	if (NULL == strchr("#!&+", name[0])) return ERR_NOSUCHCHANNEL;

	// Creacion e insercion en la lista
	*chan = channel_new(serv, name);
	if (chan == NULL) return ERR_UNAVAILRESOURCE;
	channellist_insert(&serv->chan, *chan);
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

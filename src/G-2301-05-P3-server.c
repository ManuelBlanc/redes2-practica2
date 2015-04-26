
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
	int                  	num_chan;                 	/* Numero de canales operativos        	*/
	int                  	num_users;                	/* Numero de conexiones abiertas       	*/
	int                  	num_out;                  	/* Numero users desconectados guardados	*/
	int                  	sock;                     	/* Socket que recibe peticiones        	*/
	char                 	name[SERVER_MAX_NAME_LEN];	/* Nombre del servidor                 	*/
	User*                	usrs;                     	/* Lista de usuarios                   	*/
	User*                	out;                      	/* Usuarios desconectados              	*/
	Channel*             	chan;                     	/* Lista de canales                    	*/
	pthread_mutex_t      	switch_mutex;             	/* Mutex general                       	*/
	ServerAdmin          	admin_data;               	/* Datos del administrador             	*/
	pthread_t            	ping_thr;                 	/* Hilo para ver actividad en los users	*/
	Redes2_SSL_CTX*      	ssl_ctx;                  	/* Contexto SSL                        	*/
	Redes2_SSL_CTX_config	ssl_conf;                 	/* Configuracion de las conexiones SSL 	*/
};

static void usage(int code) {
	fprintf(stderr, "usage: %s [-hv]\n", "G-2301-05-P3-server");
	fprintf(stderr, "\t%s : %s\n", "verbose",	"Pone el programa en un modo mas verboso.");
	fprintf(stderr, "\t%s : %s\n", "help",   	"Muestra esta ayuda.");
	fprintf(stderr, "\t%s : %s\n", "demonio",	"Arranca el programa como un demonio.");
	exit(code);
}

int demonio = 0;
static void procesar_opciones(int argc, char** argv) {
	static struct option longopts[] = {
		{ "verbose", 	no_argument,      	NULL,	'v'	},
		{ "help",    	no_argument,      	NULL,	'h'	},
		{ "demonio",   	no_argument,      	NULL,	'd'	},
		{NULL,0,NULL,0}
	};
	while (1) {
		int opt = getopt_long(argc, argv, "vh", longopts, NULL);
		switch (opt) {
			/* Opciones */
			case 'v': /* verbosity++;        	*/ break;
			case 'h': /* usage(EXIT_SUCCESS);	*/ break;
			case 'd': demonio = 1; break;
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
	// 1.– Crear un proceso hijo y terminar el proceso padre
	pid_t pid = fork();
	if (pid > 0) exit(EXIT_SUCCESS);

	// 2.– Crear una nueva sesion de tal forma que el proceso pase a ser el lider de sesion
	setsid();

	// 3.– Cambiar la mascara de modo de ficheros para que sean accesibles a cualquiera
	umask(0777);

	// 4.– Establecer el directorio raiz / como directorio de trabajo
	int dummy = chdir("/");
	UNUSED(dummy);

	// 5.– Cerrar todos los descriptores de fichero que pueda haber abiertos
	int i, fd_max = getdtablesize();
	for (i = 0; i < fd_max; ++i) close(i);

	// 6.– Redirigir stdin, stdout, stderr a /dev/null
	open("/dev/null", O_RDONLY); // 0 - stdin
	open("/dev/null", O_WRONLY); // 1 - stdout
	open("/dev/null", O_WRONLY); // 2 - stderr

	// 7.– Abrir el log del sistema para su uso posterior
	//openlog(const char *ident, int logopt, int facility);
	//setlogmask(int maskpri);
	//syslog(int priority, const char *message, ...);
	//closelog(void);
}

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
			if(user_ping(usr) == 0) {
				server_delete_user(serv, name);
			}
			free(name);
		}
		server_up_semaforo(serv);
		sleep(10);
	}
	return NULL;
}

Server* server_new() {
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

	serv->ssl_conf = (Redes2_SSL_CTX_config){
		/* ca_file  */ "cert/root.pem",
		/* ca_path  */ NULL,
		/* key_file */ "cert/ana.key",
		/* pem_file */ "cert/ana.pem",
	};
	serv->ssl_ctx = fijar_contexto_SSL(serv->ssl_conf);

	return serv;
}

int socket_temp_segv = -1;
static void on_segmentation_fault(int sig) {
	UNUSED(sig);
	LOG("Recibido fallo de segmentacion. Cerrando el socket... %i", close(socket_temp_segv));
	exit(EXIT_FAILURE);
}

void server_init(void) {
	int ret, yes = 1;
	struct sockaddr_in addr;

	inicializar_nivel_SSL();
	Server* serv = server_new();

	signal(SIGSEGV, on_segmentation_fault);
	signal(SIGINT, on_segmentation_fault);
	signal(SIGTERM, on_segmentation_fault);

	addr.sin_family     	= AF_INET;
	addr.sin_addr.s_addr	= INADDR_ANY;
	addr.sin_port       	= htons(6697);

	serv->sock = socket_temp_segv = socket(AF_INET, SOCK_STREAM, 0);
	LOG("Creado socket() -> %i", serv->sock);

	//setsockopt(serv->sock, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes));
	UNUSED(yes);

	while (ERR == (ret = bind(serv->sock, (struct sockaddr*) &addr, sizeof addr))) {
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

int server_accept(Server* serv) {
	struct sockaddr_in user_addr;
	socklen_t usrlen = sizeof user_addr;

	int sock = accept(serv->sock, (struct sockaddr*) &user_addr, &usrlen);
	if (sock == -1) return ERR;

	Redes2_SSL* ssl = aceptar_canal_seguro_SSL(serv->ssl_ctx, sock);
	// Si sock es -1 y errno es algo entonces hay que repetir
	if(SERVER_MAX_USERS <= serv->num_users) {
		close(sock);
		free(ssl);
		LOG("Se ha recibido una conexion cunado el servidor estaba al maximo de su capacidad.");
		return ERR;
	} else if (ERR == evaluar_post_connectar_SSL(ssl)) {
		close(sock);
		free(ssl);
		LOG("Error al intentar establecer una conexion segura.");
		return ERR;
	}

	SSock* ss = ssock_secure_new(sock, ssl);

	server_down_semaforo(serv);
	User* user = user_new(serv, ss);
	server_add_user(serv, user);
	server_up_semaforo(serv);
	return OK;
}

// Devuelve el topic.
int server_get_name(Server* serv, char** name) {
	if (serv == NULL) return ERR;
	*name = estrdup(serv->name);
	return OK;
}

int server_get_admin(Server* serv, ServerAdmin** sa) {
	if (serv == NULL) return ERR;
	*sa = emalloc(sizeof **sa);
	memcpy(*sa, &serv->admin_data, sizeof **sa);
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
	if (NULL == userlist_findByNickname(&serv->usrs, nick)) return ERR;
	return OK;
}

int server_get_num_users(Server* serv) {
	return serv->num_users;
}

int server_get_num_channels(Server* serv) {
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
	if (usr == NULL || userlist_head(usr) == NULL) return ERR;
	User* usr2 = userlist_extract(usr);
	//mirara los canales y sacar al usr si esta presente
	ChannelList chan = &serv->chan;
	while(channellist_head(chan)) {
		channel_remove_user(channellist_head(chan), usr2);
		chan = channellist_tail(chan);
	}
	userE_die(usr2);
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

int main(int argc, char** argv)
{
	LOG("Comenzando ejecucion");
	procesar_opciones(argc, argv);
	if (demonio) demonizar();
	server_init();

	return 0;
}

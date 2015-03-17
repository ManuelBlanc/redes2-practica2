
/* std */
#include <stdio.h>
#include <getopt.h>
/* unistd */

/* usr */
#include <G-2301-05-P2-config.h>

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
	return malloc(sizeof(Server*));
}

int server_accept(Server serv){
	struct sockaddr_in user_addr;
	socklen_t usrlen = sizeof user_addr;
	
	accept(serv->sock, (struct sockaddr*) &client_addr, &usrlen);
	User* user = user_new(serv, sock);
	server_add_user(serv, user);

}

void iniciar_servidor(void) {
	struct sockaddr_in addr;

	Server* serv = server_new();

	addr.sin_family     	= AF_INET;
	addr.sin_addr.s_addr	= INADDR_ANY;
	addr.sin_port       	= 0;

	serv->sock = socket(AF_INET, SOCK_STREAM, 0);
	bind(sock, (struct sockaddr*) &addr, sizeof addr);
	listen(sock, 3); // Maximo 3 peticiones de conexion encoladas

	socklen_t len = sizeof clif->addr;
	getsockname(sock, (struct sockaddr*) &clif->addr, &len);

	while (1) {
		int sock = server_accept(serv);

	}
}

int main(int argc, char const *argv[])
{
	procesar_opciones(argc, argv);
	demonizar();
	iniciar_servidor();

	return 0;
}

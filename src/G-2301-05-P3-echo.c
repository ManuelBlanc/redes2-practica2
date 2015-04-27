/* usr */
#include "G-2301-05-P3-util.h"
#include "G-2301-05-P3-ssl_functions.h"
#include "G-2301-05-P3-ssock.h"
/* std */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
/* posix */
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
/* net */
#undef  _BSD_SOURCE
#define  _BSD_SOURCE 1
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


static void* echoP_secure_client_thread(void* ss_ptr) {
	SSock* ss = ss_ptr;
	LOG("Empezado thread");

	while (1) {
		char buffer[BUFSIZ];
		ssize_t bytes = ssock_recv(ss, buffer, sizeof buffer);
		if (-1 == bytes) {
			// Si se aborto por interrupcion, continuamos
			if (EINTR == errno) continue;
			else break;
		}
		ssock_send(ss, buffer, bytes);
	}

	LOG("Cerrado conexion");
	ssock_close(ss);
	return NULL;
}

static int echoP_create_socket(in_addr_t ip, uint16_t port) {

	// Attributos de la direccion al a que nos conectamos
	struct sockaddr_in addr;
	addr.sin_family     	= AF_INET;
	addr.sin_addr.s_addr	= ip;
	addr.sin_port       	= port;

	LOG("Creado el socket del servidor");
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	LOG("Enlazado el socket");
	bind(sock, (struct sockaddr*) &addr, sizeof addr);
	listen(sock, 3); // Maximo 3 peticiones de conexion encoladas


	socklen_t len = sizeof addr;
	getsockname(sock, (struct sockaddr*) &addr, &len);
	LOG("Escuchando por %s:%i",
		inet_ntoa(addr.sin_addr),
		ntohs(addr.sin_port));

	return sock;
}

SSock* echoP_secure_accept_client(int root_sock, Redes2_SSL_CTX* ctx)
{
	struct sockaddr_in addr;
	socklen_t usrlen = sizeof addr;

	// Aceptamos la conexion
	int cli_sock = accept(root_sock, (struct sockaddr*) &addr, &usrlen);
	if (ERR == cli_sock) {
		LOG("Error al aceptar la conexion: %s", strerror(errno));
		return NULL;
	}

	// Decoramos el socket
	SSock* ss = ssock_secure_new(cli_sock, aceptar_canal_seguro_SSL(ctx, cli_sock));
	if (NULL == ss) {
		close(cli_sock);
		return NULL;
	}

	// Exito!
	LOG("Abierto conexion con %s:%i",
		inet_ntoa(addr.sin_addr),
		ntohs(addr.sin_port));

	return ss;
}


int main(int argc, char** argv) {

	UNUSED(argc);
	UNUSED(argv);

	int root_sock = echoP_create_socket(INADDR_ANY, 0);

	Redes2_SSL_CTX_config cnf = {
		/* ca_file  */ "cert/root.pem",
		/* ca_path  */ NULL,
		/* key_file */ "cert/server.key",
		/* pem_file */ "cert/server.pem",
	};

	// Inicializacion de la libreria y el contexto
	inicializar_nivel_SSL();
	Redes2_SSL_CTX* ctx = fijar_contexto_SSL(cnf);

	while (1) {
		SSock* ss = echoP_secure_accept_client(root_sock, ctx);
		if (NULL == ss) {
			LOG("Error al aceptar una conexion: %s", strerror(errno));
			break;
		}

/*		if (OK != evaluar_post_connectar_SSL(ss)) {
  			cerrar_canal_SSL(ss);
  			continue;
  		}*/

		pthread_t thread;
		int t_code = pthread_create(&thread, 0, echoP_secure_client_thread, ss);
		if (OK != t_code) {
			LOG("Error al crear al hilo: %s", strerror(t_code));
			ssock_close(ss);
			return t_code;
		}
		pthread_detach(thread);
	}

	close(root_sock);
	LOG("Cerrando el servidor");
	exit(EXIT_SUCCESS);
}

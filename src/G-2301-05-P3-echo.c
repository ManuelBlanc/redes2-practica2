/* usr */
#include "G-2301-05-P3-util.h"
#include "G-2301-05-P3-ssl_functions.h"
/* std */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
/* posix */
#include <errno.h>
#include <pthread.h>
/* net */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


static void* echoP_client_thread(void* ssl_ptr) {
	Redes2_SSL* ssl = ssl_ptr;
	LOG("Empezado thread");

	while (1) {
		char buffer[BUFSIZ];
		ssize_t bytes = recibir_datos_SSL(ssl, buffer, sizeof buffer);
		if (-1 == bytes) {
			// Si se aborto por interrupcion, continuamos
			if (EINTR == errno) continue;
			else break;
		}
		enviar_datos_SSL(ssl, buffer, bytes);
	}

	LOG("Cerrado conexion");
	cerrar_canal_SSL(ssl);
	return NULL;
}

static int echoP_create_socket(struct in_addr ip, uint16_t port) {

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
		addr.sin_addr ? inet_ntoa(addr.sin_addr) : "*.*.*.*"
		ntohs(addr.sin_port));

	return sock;
}

int echoP_secure_accept_client(int sock, Redes2_SSL_CTX* ctx)
{
	struct sockaddr_in user_addr;
	socklen_t usrlen = sizeof user_addr;

	// Aceptamos la conexion
	int cli_sock = accept(sock, (struct sockaddr*) &user_addr, &usrlen);
	if (-1 == cli_sock) {
		LOG("Error al aceptar la conexion: %s", strerror(errno));
		return -1;
	}

	// Decoramos el socket
	Redes2_SSL* ssl = aceptar_canal_seguro_SSL(ctx, sock);
	if (NULL == ssl) return close(cli_sock);

	// Exito!
	LOG("Abierto conexion con %s:%i",
		inet_ntoa(addr.sin_addr),
		ntohs(addr.sin_port));

	return ssl;
}


int main(int argc, char** argv) {

	int root_sock = echoP_create_socket(INADDR_ANY, 0);

	// Inicializacion de la libreria y el contexto
	inicializar_nivel_SSL();
	Redes2_SSL_CTX ctx;
	fijar_contexto_SSL(&ctx);

	while (1) {
		int cli_sock = echoP_accept_client(root_sock, echoP_client_thread);

		Redes2_SSL* ssl = aceptar_canal_seguro_SSL(&ctx, cli_sock);

		if (ERR == evaluar_post_conectar_SSL(ssl)) {
			cerrar_canal_SSL();
		}

		if (ERR == cli_sock) {
			LOG("Error al aceptar una conexion: ", strerror(errno));
			break;
		}

		pthread_t thread;
		int t_code = pthread_create(&thread, 0, echoP_secure_client_thread, ssl);
		if (-1 == t_code) {
			LOG("Error al crear al hilo: %s", strerror(t_code));
			free(sock_ptr);
			close(sock);
			return t_code;
		}
		pthread_detach(thread);
	}

	LOG("Cerrando el servidor");
	exit(EXIT_SUCCESS);
}


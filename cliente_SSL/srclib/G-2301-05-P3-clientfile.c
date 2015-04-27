
/* std */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* posix */
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
/* lib */
#include <glib.h>
#include <gtk/gtk.h>
/* redes2 */
#include <redes2/irc.h>
#include <redes2/chat.h>
/* usr */
#include "G-2301-05-P3-util.h"
#include "G-2301-05-P3-client.h"
#include "G-2301-05-P3-clientcmd.h"
#include "G-2301-05-P3-clientfile.h"


ClientRecv receive = {{0}, {0}, 0, 0, 0};

typedef struct ClientFile {
	Client*           	cli;
	pthread_t         	thread;
	char*             	path;
	char              	nick[IRC_MAX_NICK_LEN+1];
	struct sockaddr_in	addr;
} ClientFile;

int clientfile_set_connection_params(char* msg) {
	char buf[50 + 1];
	int items = sscanf(msg, "DCC SEND %50s %30s %u %lu", receive.filename, buf, &receive.port, &receive.file_size);
	receive.ip.s_addr = inet_addr(buf);
	if (items != 4) return ERR;
	//mirar que el nombre dl fichero no sea mas de 50
	receive.port = htons(receive.port);
	receive.active = 1;
	return OK;
}

int clientfile_is_active() {
	return receive.active;
}

void clientfile_delete_connection_params() {
	receive.active = 0;
}

/* ======================================================================================
       ENVIO
   ====================================================================================== */
static char* get_my_ip(void) {
	struct ifaddrs *addrs, *cur;

	getifaddrs(&addrs);
	cur = addrs;

	for (cur = addrs; cur; cur = cur->ifa_next) {
		if (cur->ifa_addr && cur->ifa_addr->sa_family == AF_INET) {
			struct sockaddr_in *pAddr = (struct sockaddr_in*) cur->ifa_addr;
			LOG("Asociado a la interfaz \"%s\" con la IP %s", cur->ifa_name, inet_ntoa(pAddr->sin_addr));
			freeifaddrs(addrs);
			return inet_ntoa(pAddr->sin_addr);
		}
	}
	freeifaddrs(addrs);
	return NULL;
}

static void* file_send_function(void* clif_ptr) {
	ClientFile* clif = (ClientFile*) clif_ptr;
	FILE* fich;

	LOG("Iniciado hilo de envio");

	clif->addr.sin_family     	= AF_INET;
	clif->addr.sin_addr.s_addr	= INADDR_ANY;
	clif->addr.sin_port       	= 0;

	LOG("Comprobando el tamaño del fichero...");

	fich = fopen(clif->path, "rb");
	fseek(fich, 0L, SEEK_END);
	size_t file_size = ftell(fich);
	fseek(fich, 0L, SEEK_SET);

	LOG("Preparando socket de escucha...");

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	bind(sock, (struct sockaddr*) &clif->addr, sizeof clif->addr);
	listen(sock, 1); // Maximo 1 conexion

	socklen_t len = sizeof clif->addr;
	getsockname(sock, (struct sockaddr*) &clif->addr, &len);

	LOG("Avisando al par ...");

	gdk_threads_enter();
	clientcmdf(clif->cli,
		"/msg %s DCC SEND %s %s %u %lu",
		clif->nick,
		basename(clif->path),
		get_my_ip(),
		ntohs(clif->addr.sin_port),
		(unsigned long int)file_size);
	gdk_threads_leave();

	// Aceptamos su conexion (ignorando quien se conecta, no lo podemos verificar)
	{
		int tmp_socket;
		struct sockaddr_in client_addr;
		socklen_t clilen = sizeof client_addr;
		tmp_socket = accept(sock, (struct sockaddr*) &client_addr, &clilen);
		close(sock);
		sock = tmp_socket;
		LOG("Conexion desde %s:%i aceptada!",
			inet_ntoa(client_addr.sin_addr),
			(int)ntohs(client_addr.sin_port));
	}


	// Le enviamos el fichero
	while (1) {
		unsigned char buffer[BUFSIZ];
		size_t bytes_read = fread(buffer, 1, sizeof buffer, fich);
		if (bytes_read== 0) break; // EOF o error
		sendto(sock/*_send*/, buffer, bytes_read, 0, NULL, 0);
	}

	LOG("Envio de fichero finalizado");

	// Cerramos el socket y fichero
	close(sock);
	fclose(fich);
	free(clif);
	return NULL;
}

int clientfile_send(Client* cli, char* nick, char* path) {

	ClientFile* clif = malloc(sizeof *clif);
	clif->cli = cli;

	strncpy(clif->nick, nick, IRC_MAX_NICK_LEN);

	clif->path = malloc(strlen(path) + 1);
	strcpy(clif->path, path);

	pthread_create(&clif->thread, NULL, &file_send_function, clif);
	pthread_detach(clif->thread);
	return OK;
}


/* ======================================================================================
       RECEPCION
   ====================================================================================== */
static void* file_recv_function(void* clif_ptr) {
	ClientFile* clif = (ClientFile*) clif_ptr;
	FILE* fich;
	ssize_t len;
	char buf[BUFSIZ];

	LOG("Thread de recepcion creado. Abriendo conexion...");

	// Abrimos un socket y nos conectamos a clif->addr
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	fich = fopen(receive.filename, "wb");
	connect(sock, (struct sockaddr*) &clif->addr, sizeof clif->addr);

	LOG("Conectado! Recibiendo fichero...");

	// Leemos en un bucle y guardamos a fichero
	while(1) {
		len = recvfrom(sock, buf, BUFSIZ, 0, NULL, NULL);
		if (len < 0) {
			gdk_threads_enter();
			error_text(current_page(), "Ha ocurrido un error al recibir");
			gdk_threads_leave();
			close(sock);
			fclose(fich);
			free(clif);
			return NULL;
		}
		if (len == 0) break;
		fwrite(buf, 1, len, fich);
	}

	LOG("Recepcion finalizada");
	clientfile_delete_connection_params();

	close(sock);
	fclose(fich);
	free(clif);
	return NULL;
}


int clientfile_recv(void) {
	if (!clientfile_is_active()) return ERR;
	ClientFile* clif = malloc(sizeof *clif);

	clif->addr.sin_family	= AF_INET;
	clif->addr.sin_addr  	= receive.ip;
	clif->addr.sin_port  	= receive.port;

	pthread_create(&clif->thread, NULL, &file_recv_function, clif);
	pthread_detach(clif->thread);
	return OK;
}

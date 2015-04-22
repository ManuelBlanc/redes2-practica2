
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <errno.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ATTRIBUTE(attr) __attribute__(attr)

#define	LOG(...)	(_log(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__))
void _log(const char* file, int line, const char* func, char* fmt, ...)
ATTRIBUTE((format(printf, 4, 5)));

#define BOLD(str)  	"\033[1m"  str "\033[0m"
#define RED(str)   	"\033[31m" str "\033[0m"
#define GREEN(str) 	"\033[32m" str "\033[0m"
#define YELLOW(str)	"\033[33m" str "\033[0m"
#define PINK(str)  	"\033[1;35m" str "\033[0m"

static void normalize_whitespace(char* str) {
	while (*str) {
		if (isspace(*str)) *str = ' ';
		str++;
	}
}

//static FILE* log_file;
void _log(const char* file, int line, const char* func, char* fmt, ...) {
	static char log_buffer[512];
	int len;
	va_list argptr;

	len = snprintf(log_buffer, sizeof log_buffer, GREEN("%s()")"@"YELLOW("%s")":"RED("%i")": ", func, file, line);
	if (len < 0) return; // Ha ocurrido un error
	if (len > (int)(sizeof log_buffer)) len = sizeof log_buffer;

	va_start(argptr, fmt);
	vsnprintf(log_buffer + len, sizeof log_buffer - len, fmt, argptr);
	va_end(argptr);

	normalize_whitespace(log_buffer);

	// Imprimimos por consola
	fprintf(stdout, "%s\n", log_buffer);
	fflush(stdout);
}


static void* client_echo_thread(void* sock_ptr) {
	int sock = *(int*)sock_ptr;
	free(sock_ptr);

	while (1) {
		char buffer[BUFSIZ];
		ssize_t bytes = recv(sock, buffer, sizeof buffer, 0);
		if (-1 == bytes) break;
		send(sock, buffer, bytes, 0);
	}

	return NULL;
}


int main(int argc, char** argv) {

	// Attributos de la direccion al a que nos conectamos
	struct sockaddr_in addr;
	addr.sin_family     	= AF_INET;
	addr.sin_addr.s_addr	= INADDR_ANY;
	addr.sin_port       	= 0;


	LOG("Creado el socket del servidor");
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	LOG("Enlazado el socket");
	bind(sock, (struct sockaddr*) &addr, sizeof addr);
	listen(sock, 3); // Maximo 3 peticiones de conexion encoladas


	socklen_t len = sizeof addr;
	getsockname(sock, (struct sockaddr*) &addr, &len);
	LOG("Escuchando conexiones por %s:%i",
		inet_ntoa(addr.sin_addr),
		ntohs(addr.sin_port));

	while (1) {
		struct sockaddr_in user_addr;
		socklen_t usrlen = sizeof user_addr;

		int cli_sock = accept(sock, (struct sockaddr*) &user_addr, &usrlen);
		if (-1 == cli_sock) {
			LOG("Error al aceptar la conexion: %s", strerror(errno));
			break;
		}

		LOG("Abierto conexion con %s:%i",
			inet_ntoa(addr.sin_addr),
			ntohs(addr.sin_port));

		int* sock_ptr = malloc(sizeof *sock_ptr);
		*sock_ptr = cli_sock;

		pthread_t thread;
		int t_code = pthread_create(&thread, 0, client_echo_thread, sock_ptr);
		if (-1 == t_code) {
			LOG("Error al crear al hilo: %s", strerror(t_code));
			free(sock_ptr);
			close(sock);
		}
		pthread_detach(thread);
	}

	exit(EXIT_SUCCESS);
}


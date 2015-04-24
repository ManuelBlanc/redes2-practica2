
#include "G-2301-05-P3-ssock.h"
#include "G-2301-05-P3-util.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

struct SSock {
	SSType type;
	int fd;
	Redes2_SSL* ssl;
};

SSock* ssock_new(int fd) {
	// Comprueba si es valido el descriptor de ficheroq
	if (-1 == fcntl(fd, F_GETFD)) return NULL;

	SSock* ss = emalloc(sizeof *ss);

	ss->type = SS_NORMAL;
	ss->fd   = fd;

	return ss;
}

SSock* ssock_secure_new(int fd, Redes2_SSL* ssl) {
	// Comprueba si es valido el descriptor de ficheroq
	if (-1 == fcntl(fd, F_GETFD)) return NULL;
	if (NULL == ssl) return NULL;

	SSock* ss = emalloc(sizeof *ss);

	ss->type = SS_SECURE;
	ss->fd   = fd;
	ss->ssl  = ssl;

	return ss;
}

SSType sssock_get_type(SSock* ss) {
	if (NULL == ss) return SS_INVALID;
	return ss->type;
}

ssize_t ssock_send(SSock* ss, void* buf, size_t len) {
	switch (sssock_get_type(ss)) {
		case SS_SECURE:
			return enviar_datos_SSL(ss->ssl, buf, len);
		case SS_NORMAL:
			return send(ss->fd, buf, len, 0);
		case SS_INVALID:
			return ERR;
	}
}

ssize_t ssock_recv(SSock* ss, void* buf, size_t len) {
	switch (sssock_get_type(ss)) {
		case SS_SECURE:
			return recibir_datos_SSL(ss->ssl, buf, len);
		case SS_NORMAL:
			return recv(ss->fd, buf, len, 0);
		case SS_INVALID:
			return ERR;
	}
}

int ssock_close(SSock* ss) {
	if (NULL == ss) return ERR;

	if (SS_SECURE == ss->type) {
		cerrar_canal_SSL(ss->ssl);
	}
	close(ss->fd);

	free(ss);
	return OK;
}


#ifndef SSOCK_H
#define SSOCK_H

#include "G-2301-05-P3-ssl_functions.h"

typedef struct SSock SSock;
typedef enum SSType {
	SS_INVALID,
	SS_NORMAL,
	SS_SECURE,
} SSType;

// Creacion de un s-socket generico
SSock* ssock_new(int fd);
SSock* ssock_secure_new(int fd, Redes2_SSL* ssl);

SSType sssock_get_type(SSock* ss);

// Las 3 operaciones comunes: enviar, recibir y cerrar
ssize_t ssock_send(SSock* ss, void* buf, size_t len);
ssize_t ssock_recv(SSock* ss, void* buf, size_t len);
int ssock_close(SSock* ss);

#endif /* SSOCK_H */

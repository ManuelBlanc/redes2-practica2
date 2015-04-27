
#ifndef CLIENT_H
#define CLIENT_H

#include <pthread.h>
#include "G-2301-05-P3-ssock.h"

#define IRC_MAX_CHANNEL_LEN	(200)	// RFC1459 1.3
#define IRC_MAX_NICK_LEN   	(9)  	// RFC1459 1.2
#define IRC_MAX_PRE_LEN    	(64) 	// Decision arbitraria
#define IRC_MAX_CMD_LEN    	(512)	//
#define NUM_MAX_IGN        	(20) 	// Numero maximo de personas ignoradas
#define SECURE_PORT        	     	(6697)  // Puerto para conexiones seguras

// Ordenamos los campos de mayor a menor para evitar fragmentacion
typedef struct Client {
	char        	buf[IRC_MAX_CMD_LEN+1];                  	/* Buffer de recepcion               	*/
	char        	cmd[IRC_MAX_CMD_LEN+1];                  	/* Buffer para el comando            	*/
	char        	pre[IRC_MAX_PRE_LEN+1];                  	/* Prefijo                           	*/
	char        	nick[IRC_MAX_NICK_LEN+1];                	/* Nickname                          	*/
	char*       	serv;                                    	/* Servidor                          	*/
	unsigned int	port;                                    	/* Puerto                            	*/
	char*       	name;                                    	/* Nombre                            	*/
	char*       	rname;                                   	/* Nombre real                       	*/
	char        	ignored[NUM_MAX_IGN][IRC_MAX_NICK_LEN+1];	/* Lista de ignorados                	*/
	SSock*      	sock;                                    	/* Estructura socket seguro/no seguro	*/
	pthread_t   	thr;                                     	/* Hilo                              	*/
} Client;

/** Crea un nuevo cliente.
 * @returns Una estructura Cliente inicializada, o NULL
 */
Client* client_new();

/** Destruye un cliente.
 * @param cli Cliente
 */
void client_delete(Client* cli);

/** Abre una conexion con el servidor.
 * @param cli Cliente
 * @returns Indicador de error
 */
int client_connect(Client* cli);
/**
 * @param cli Cliente
 * @returns Indicador de error
 */
int client_disconnect(Client* cli);

/**
 * @param cli Cliente
 * @param server Servidor
 * @param server2 Servidor segundo
 * @returns Indicador de error
 */
int client_send_pong(Client* cli, char* server, char* server2);

/**
 * @param cli Cliente
 * @param nick Persona a ignorar
 * @return Indicador de error
 */
int client_add_ignore(Client* cli, char* nick);

/**
 * @param cli Cliente
 * @param nick Persona a dejar de ignorar
 * @return Indicador de error
 */
int client_remove_ignore(Client* cli, char* nick);

/**
 * @param cli Cliente
 * @param nick Nick a buscar en la lista de ignorados
 * @return Indicador de error
 */
int client_lookfor_ignore(Client* cli, char* nick);

/**
 * @param cli Cliente
 * @param serv Nombre del servidor
 * @returns Indicador de error
 */
int client_set_server(Client* cli, char* serv);
/**
 * @param cli Cliente
 * @param port Puerto
 * @returns Indicador de error
 */
int client_set_port(Client* cli, unsigned int port);
/**
 * @param cli Cliente
 * @param name);
 * @returns Indicador de error
 */
int client_set_name(Client* cli, char* name);
/**
 * @param cli Cliente
 * @param rname Nombre real
 * @returns Indicador de error
 */
int client_set_real_name(Client* cli, char* rname);
/**
 * @param cli Cliente
 * @param nick Pseudonimo
 * @returns Indicador de error
 */
int client_set_nick(Client* cli, char* nick);
/**
 * @param cli Cliente
 * @param pre Prefijo
 * @returns Indicador de error
 */
int client_set_prefix(Client* cli, char* pre);

/**
 * @param cli Cliente
 * @returns Indicador de error
 */
int client_send_cmd(Client* cli);

#endif /* CLIENT_H */

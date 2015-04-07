
/* std */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>
/* redes2 */
#include <redes2/irc.h>
/* usr */
#include "G-2301-05-P2-user.h"
#include "G-2301-05-P2-config.h"
#include "G-2301-05-P2-switches.h"

typedef enum UserConnState {
	USERCS_RECEIVED_PASS = (1<<0),
	USERCS_RECEIVED_NICK = (1<<1),
	USERCS_RECEIVED_USER = (1<<3)
} UserConnState;

struct User {
	char         	buffer_recv[IRC_MAX_CMD_LEN+1];	/* Buffer de recepcion               	*/
	char         	prefix[USER_MAX_PRE_LEN+1];    	/* Prefijo                           	*/
	char         	nick[USER_MAX_NICK_LEN+1];     	/* Nickname                          	*/
	char         	name[USER_MAX_NAME_LEN+1];     	/* Nombre                            	*/
	char         	rname[USER_MAX_RNAME_LEN+1];   	/* Nombre real                       	*/
	char         	away_msg[USER_MAX_AWAY_LEN+1]; 	/* Mensaje de away                   	*/
	int          	sock_fd;                       	/* Descriptor del socket             	*/
	UserConnState	conn_state;                    	/* Flag para los comandos de registro	*/
	Server*      	server;                        	/* Servidor al que pertenece         	*/
	struct User* 	next;                          	/* Puntero al siguiente usuario      	*/
	pthread_t    	thread;                        	/* Hilo                              	*/
};

static int connection_switch(Server* serv, User* usr, char* cmd) {
	char buf[IRC_MAX_CMD_LEN+1];
	switch (IRC_CommandQuery(cmd)) {

		case PASS: /* 1 */
			if ((USERCS_RECEIVED_PASS | USERCS_RECEIVED_NICK) & usr->conn_state) {
				// Se ha recibido un pass cuando no se esperaba
				return ERR;
			}
			usr->conn_state |= USERCS_RECEIVED_PASS;
			return exec_cmd_pass(serv, usr, buf, NULL, NULL, cmd);


		case NICK: /* 2a */
			if (USERCS_RECEIVED_NICK & usr->conn_state) {
				// Se ha recibido un nick cuando no se esperaba
				return ERR;
			}
			usr->conn_state |= USERCS_RECEIVED_NICK;
			return exec_cmd_nick(serv, usr, buf, NULL, NULL, cmd);

		case SERVICE: /* 2a */
			// No aceptamos conexiones de otros servidores!
			return ERR;

		case USER: /* 3 */
			if (USERCS_RECEIVED_USER & usr->conn_state) {
				// Se ha recibido un nick cuando no se esperaba
				return ERR;
			}
			usr->conn_state |= USERCS_RECEIVED_USER;
			return exec_cmd_user(serv, usr, buf, NULL, NULL, cmd);

		default:
			return (USERCS_RECEIVED_USER & usr->conn_state) ? OK : ERR;
	}
}


// Procesa los comandos en una cadena, bajo el mutex global.
static int userP_process_commands(User* usr, char* str) {
	char* cmd;
	int more_commands = 1;

	while (more_commands) {
		switch (IRC_UnPipelineCommands(str, &cmd)) {
			case IRC_ENDPIPE:
				usr->buffer_recv[0] = '\0';
				more_commands = 0;
			case IRC_OK:
				server_down_semaforo(usr->server);

				if (!usr->conn_state) connection_switch(usr->server, usr, cmd);
				else                  action_switch(usr->server, usr, cmd);

				server_up_semaforo(usr->server);
				str = NULL;
				break;
			case IRC_EOP:
				memset(usr->buffer_recv, 0, sizeof(usr->buffer_recv));
				strncpy(usr->buffer_recv, cmd, sizeof(usr->buffer_recv));
				return OK;
		}
	}
	return OK;
}



// Funcion que ejecuta el hilo lector.
static void* userP_reader_thread(void* data) {
	User* usr = data;
	ssize_t len;
	size_t len_buf;

	while (1) {
		len_buf = strlen(usr->buffer_recv);
		len = recv(usr->sock_fd, usr->buffer_recv+len_buf, IRC_MAX_CMD_LEN-len_buf, 0);
		if (len <= 0) return NULL; // Se cierra la conexion
		usr->buffer_recv[len+len_buf] = '\0';
		userP_process_commands(usr, usr->buffer_recv);
	}
}

// Crea una nueva estructura usuario a partir de un socket.
User* user_new(Server* serv, int sock) {
	User* usr = ecalloc(1, sizeof *usr);
	usr->server  = serv;
	usr->sock_fd = sock;
	usr->conn_state = 0;
	if (OK != pthread_create(&usr->thread, NULL, userP_reader_thread, usr)) {
		free(usr);
		return NULL;
	}
	pthread_detach(usr->thread);
	return usr;
}

// Destruye la estructura y cierra el socket.
void user_delete(User* usr) {
	if (usr == NULL) return;
	close(usr->sock_fd);
	free(usr);
}

int user_init_prefix(User* usr) {
	char* host;
	if (usr == NULL) return ERR;

	{
		// Buscamos nuestra IP
		struct sockaddr_in address;
		socklen_t addr_len = sizeof address;
		getsockname(usr->sock_fd, (struct sockaddr*)&address, &addr_len);
		host = inet_ntoa(address.sin_addr);
	}

	snprintf(usr->prefix, USER_MAX_PRE_LEN, "%9s!%9s@%63s", usr->nick, usr->name, host);
	return OK;
}

int user_get_prefix(User* usr, char** prefix) {
	if (usr == NULL) return ERR;
	*prefix = usr->prefix;
	return OK;
}

// Envia un comando al usuario.
int user_send_cmd(User* usr, const char* str) {
	if (usr == NULL) return ERR;
	ssize_t bytesSent = send(usr->sock_fd, str, strlen(str), 0);
	if (bytesSent < -1) return -1;
	return OK;
}

// Envia un comando con formato a un usuario.
int user_send_cmdf(User* usr, const char* fmt, ...) {
	if (usr == NULL) return ERR;
	char buffer[IRC_MAX_CMD_LEN+1];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buffer, sizeof buffer, fmt, ap);
	int ret = user_send_cmd(usr, buffer);
	va_end(ap);
	return ret;
}

// Devuelve el nick del usuario.
int user_get_nick(User* usr, char** nick) {
	if (usr == NULL) return ERR;
	*nick = usr->nick;
	return OK;
}

// Cambia el nick del usuario.
int user_set_nick(User* usr, const char* nick) {
	if (usr == NULL) return ERR;
	strncpy(usr->nick, nick, USER_MAX_NICK_LEN);
	return OK;
}

// Devuelve el nombre del usuario.
int user_get_name(User* usr, char** name) {
	if (usr == NULL) return ERR;
	*name = usr->name;
	return OK;
}

// Cambia el nombre del usuario.
int user_set_name(User* usr, const char* name) {
	if (usr == NULL) return ERR;
	strncpy(usr->name, name, USER_MAX_NAME_LEN);
	return OK;
}

// Devuelve el nombre real del usuario.
int user_get_rname(User* usr, char** rname) {
	if (usr == NULL) return ERR;
	*rname = usr->rname;
	return (rname == NULL);
}

// Cambia el nombre real del usuario.
int user_set_rname(User* usr, const char* rname) {
	if (usr == NULL) return ERR;
	strncpy(usr->rname, rname, USER_MAX_RNAME_LEN);
	return OK;
}

// Devuelve el mensaje de away (si esta away).
int user_get_away(User* usr, char** away_msg) {
	if (usr == NULL) return ERR;
	*away_msg = usr->away_msg;
	return (away_msg == NULL);
}

// Cambia el estado de away del usuario.
int user_set_away(User* usr, const char* away_msg) {
	if (usr == NULL) return ERR;
	strncpy(usr->away_msg, away_msg, USER_MAX_AWAY_LEN);
	return OK;
}

// ============================================================================
//   Funciones de listas
// ============================================================================

// Macros
#define userlist_head(list)	(*(list))
#define userlist_tail(list)	(&(userlist_head(list)->next))

// Inserta un elemento en la lista.
int userlist_insert(UserList list, User* usr) {
	if (list == NULL || usr == NULL) return ERR;

	// Comprobamos que NO este ya en una lista
	if (usr->next != NULL) return ERR;

	usr->next = userlist_head(list);
	*list = usr;
	return OK;
}

// Extrae el primer elemento de una lista.
User* userlist_extract(UserList list) {
	User* usr;
	if (list == NULL) return NULL;

	usr = userlist_head(list);
	*list = usr->next;
	usr->next = NULL;
	return usr;
}

// Busca un elemento por su nombre.
UserList userlist_findByName(UserList list, const char* name) {
	if (list == NULL || name == NULL) return NULL;

	while (1) {
		User* usr = userlist_head(list);
		if (usr == NULL) break;
		if (strncmp(name, usr->name, USER_MAX_NAME_LEN)) break;
		list = userlist_tail(list);
	}

	return list;
}

// Libera todos los elementos de la lista.
void userlist_deleteAll(UserList list) {
	User* usr;
	if (list == NULL) return;

	usr = userlist_head(list);
	while (usr != NULL) {
		list = userlist_tail(list);
		free(usr);
		usr = userlist_head(list);
	}
}

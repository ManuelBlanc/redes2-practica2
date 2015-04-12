
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
	USERCS_RECEIVED_USER = (1<<2)
} UserConnState;

typedef enum UserFlags {
	UF_AWAY      	= (1<<0), // a - user is flagged as away;
	UF_INVISIBLE 	= (1<<1), // i - marks a users as invisible;
	UF_WALLOP    	= (1<<2), // w - user receives wallops;
	UF_RESTRICTED	= (1<<3), // r - restricted user connection;
	UF_OPERATOR  	= (1<<4), // o - operator flag;
	UF_LOPERATOR 	= (1<<5), // O - local operator flag;
	UF_SERVNOTICE	= (1<<6), // s - marks a user for receipt of server notices.
} UserFlags;

struct User {
	char         	buffer_recv[IRC_MAX_CMD_LEN+1];	/* Buffer de recepcion               	*/
	char         	prefix[USER_MAX_PRE_LEN+1];    	/* Prefijo                           	*/
	char         	nick[USER_MAX_NICK_LEN+1];     	/* Nickname                          	*/
	char         	name[USER_MAX_NAME_LEN+1];     	/* Nombre                            	*/
	char         	rname[USER_MAX_RNAME_LEN+1];   	/* Nombre real                       	*/
	char         	away_msg[USER_MAX_AWAY_LEN+1]; 	/* Mensaje de away                   	*/
	int          	sock_fd;                       	/* Descriptor del socket             	*/
	UserFlags    	flags;                         	/* Banderas del usuario              	*/
	UserConnState	conn_state;                    	/* Flag para los comandos de registro	*/
	Server*      	server;                        	/* Servidor al que pertenece         	*/
	struct User* 	next;                          	/* Puntero al siguiente usuario      	*/
	pthread_t    	thread;                        	/* Hilo                              	*/
};

static long connection_switch(Server* serv, User* usr, char* cmd) {
	char buf[IRC_MAX_CMD_LEN+1];
	LOG("Switch inicial de conexion");
	switch (IRC_CommandQuery(cmd)) {

		case PASS: /* 1 */
			LOG("Recibido PASS inicial");
			return exec_cmd_PASS(serv, usr, buf, NULL, NULL, cmd);

		case NICK: /* 2a */
			LOG("Recibido NICK inicial");
			return exec_cmd_NICK(serv, usr, buf, NULL, NULL, cmd);

		case SERVICE: /* 2a */
			LOG("Recibido SERVICE inicial");
			// No aceptamos conexiones de otros servidores!
			return ERR;

		case USER: /* 3 */
			LOG("Recibido USER inicial");
			usr->conn_state |= USERCS_RECEIVED_USER;
			return exec_cmd_USER(serv, usr, buf, NULL, NULL, cmd);

		default:
			LOG("Comando no reconcido en el handshake");
			if (!strncmp(cmd, "CAP ", 4)) return OK;
			return (USERCS_RECEIVED_USER & usr->conn_state) ? OK : ERR;
	}
}


// Procesa los comandos en una cadena, bajo el mutex global.
static long userP_process_commands(User* usr, char* str) {
	char* cmd;
	int more_commands = 1;

	while (more_commands) {
		switch (IRC_UnPipelineCommands(str, &cmd)) {
			case IRC_ENDPIPE:
				usr->buffer_recv[0] = '\0';
				more_commands = 0;
			case IRC_OK:
				server_down_semaforo(usr->server);

				LOG("Procesando el comando %s", cmd);

				if (0 == (USERCS_RECEIVED_USER & usr->conn_state)) {
					if (OK != connection_switch(usr->server, usr, cmd)) {
						LOG("Handshake inicial fallido");
					}
				}
				else {
					action_switch(usr->server, usr, cmd);
				}
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
	char* nick;

	while (1) {
		len_buf = strlen(usr->buffer_recv);
		len = recv(usr->sock_fd, usr->buffer_recv+len_buf, IRC_MAX_CMD_LEN-len_buf, 0);
		if (len <= 0) break; // Se cierra la conexion
		usr->buffer_recv[len+len_buf] = '\0';
		userP_process_commands(usr, usr->buffer_recv);
	}
	user_get_nick(usr, &nick);
	LOG("Hilo de usuario %s ha muerto", nick);
	return NULL;
}

// Crea una nueva estructura usuario a partir de un socket.
User* user_new(Server* serv, int sock) {
	User* usr = ecalloc(1, sizeof *usr);
	usr->server     = serv;
	usr->sock_fd    = sock;
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
	if (NULL == usr) return;
	close(usr->sock_fd);
	free(usr);
}

static void userP_lock(User* usr) {
	ASSERT(NULL != usr, "Bloqueando usuario nulo");
	pthread_mutex_lock(&usr->mutex);
}
static void userP_unlock(User* usr) {
	ASSERT(NULL != usr, "Desbloqueando usuario nulo");
	pthread_mutex_unlock(&usr->mutex);
}

long user_init_prefix(User* usr) {
	char* host;
	if (NULL == usr) return ERR;

	{
		// Buscamos nuestra IP
		struct sockaddr_in address;
		socklen_t addr_len = sizeof address;
		getsockname(usr->sock_fd, (struct sockaddr*)&address, &addr_len);
		host = inet_ntoa(address.sin_addr);
	}

	free(usr->prefix);
	IRC_Prefix(&usr->prefix, usr->nick, usr->name, host, host);
	return OK;
}

long user_get_prefix(User* usr, char** prefix) {
	if (NULL == usr) return ERR;
	*prefix = usr->prefix;
	return OK;
}

// Envia un comando al usuario.
long user_send_cmd(User* usr, char* str) {
	if (NULL == usr) return ERR;
	ssize_t bytesSent = send(usr->sock_fd, str, strlen(str), 0);
	if (bytesSent < -1) return -1;
	return OK;
}

// Envia un comando con formato a un usuario.
long user_send_cmdf(User* usr, char* fmt, ...) {
	if (NULL == usr) return ERR;
	char buffer[IRC_MAX_CMD_LEN+1];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buffer, sizeof buffer, fmt, ap);
	int ret = user_send_cmd(usr, buffer);
	va_end(ap);
	return ret;
}

// Devuelve el nick del usuario.
long user_get_nick(User* usr, char** nick) {
	if (NULL == usr) return ERR;
	*nick = usr->nick;
	return OK;
}

// Cambia el nick del usuario.
long user_set_nick(User* usr, char* nick) {
	if (NULL == usr) return ERR;
	strncpy(usr->nick, nick, USER_MAX_NICK_LEN);
	return OK;
}

// Devuelve el nombre del usuario.
long user_get_name(User* usr, char** name) {
	if (NULL == usr) return ERR;
	*name = usr->name;
	return OK;
}

// Cambia el nombre del usuario.
long user_set_name(User* usr, char* name) {
	if (NULL == usr) return ERR;
	strncpy(usr->name, name, USER_MAX_NAME_LEN);
	return OK;
}

// Devuelve el nombre real del usuario.
long user_get_rname(User* usr, char** rname) {
	if (NULL == usr) return ERR;
	*rname = usr->rname;
	return (NULL == rname);
}

// Cambia el nombre real del usuario.
long user_set_rname(User* usr, char* rname) {
	if (NULL == usr) return ERR;
	strncpy(usr->rname, rname, USER_MAX_RNAME_LEN);
	return OK;
}

// Devuelve el mensaje de away (si esta away).
long user_get_away(User* usr, char** away_msg) {
	if (NULL == usr) return ERR;
	*away_msg = usr->away_msg;
	return (NULL == away_msg);
}

// Cambia el estado de away del usuario.
long user_set_away(User* usr, char* away_msg) {
	if (NULL == usr) return ERR;
	if (NULL == away_msg)
	strncpy(usr->away_msg, away_msg, USER_MAX_AWAY_LEN);
	return OK;
}

// Cambia de una letra de flag de usuario a su mascara correspondiente.
static UserFlags userP_char_to_flag(char flag) {
	switch (flag) {
		//case 'a': return UF_AWAY; <-- Solo se puede cambiar con _set_away
		case 'i': return UF_INVISIBLE;
		case 'w': return UF_WALLOP;
		case 'r': return UF_RESTRICTED;
		case 'o': return UF_OPERATOR;
		case 'O': return UF_LOPERATOR;
		case 's': return UF_SERVNOTICE;
		default: return 0;
	}
}

// Comprueba si un usuario tiene una flag.
long user_has_flag(User* usr, char flag) {
	UserFlags flag_mask;
	if (NULL == usr) return ERR_NEEDMOREPARAMS;

	// Convertimos la flag a una mascara
	flag_mask = userP_char_to_flag(flag);
	if (0 == flag_mask) return ERR_UMODEUNKNOWNFLAG;

	return (flag_mask & usr->flags) != 0;
}

// Pone una flag a un usuario.
long user_set_flag(User* usr, char flag, User* actor) {
	UserFlags flag_mask;
	if (NULL == chan || NULL == usr) return ERR_NEEDMOREPARAMS;

	// Convertimos la flag a una mascara
	flag_mask = userP_char_to_flag(flag);
	if (0 == flag_mask) return ERR_UMODEUNKNOWNFLAG;

	// Exito!
	usr->flags |= flag_mask;
	return OK;
}

// Quita una flag de un usuario.
long user_unset_flag(User* usr, char flag, User* actor) {
	UserFlags flag_mask;
	if (NULL == chan || NULL == usr) return ERR_NEEDMOREPARAMS;

	// Convertimos la flag a una mascara
	flag_mask = userP_char_to_flag(flag);
	if (0 == flag_mask) return ERR_UMODEUNKNOWNFLAG;

	// Exito!
	usr->flags &= ~flag_mask;
	return OK;
}

// ============================================================================
//   Funciones de listas
// ============================================================================


#define userlistP_head(list)	(*(list))
#define userlistP_tail(list)

User userlist_head(UserList list) {
	return (NULL != list) ? userlistP_head(list) : NULL;
}
UserList userlist_tail(UserList list) {
	return (NULL != list) ? userlistP_tail(list) : NULL;
}

// Inserta un elemento en la lista.
int userlist_insert(UserList list, User* usr) {
	if (list == NULL || usr == NULL) return ERR;

	// Comprobamos que NO este ya en una lista
	while(1) {
		if (userlistP_head(list) == NULL) break;
		if (userlistP_head(list) == usr)  return ERR;
		list = userlistP_tail(list);
	}

	usr->next = userlistP_head(list);
	*list = usr;
	return OK;
}

// Extrae el primer elemento de una lista.
User* userlist_extract(UserList list) {
	User* usr;
	if (NULL == list) return NULL;

	usr = userlistP_head(list);
	*list = usr->next;
	usr->next = NULL;
	return usr;
}

// Busca un elemento por su nombre.
UserList userlist_findByNickname(UserList list, char* name) {
	if (NULL == list || NULL == name) return NULL;

	while (1) {
		User* usr = userlistP_head(list);
		if (NULL == usr) break;
		if (0 == strncmp(name, usr->nick, USER_MAX_NICK_LEN)) break;
		list = userlistP_tail(list);
	}

	return list;
}

// Busca un elemento por su nombre.
UserList userlist_findByUsername(UserList list, char* name) {
	if (NULL == list || NULL == name) return NULL;

	while (1) {
		User* usr = userlistP_head(list);
		if (NULL == usr) break;
		if (0 == strncmp(name, usr->name, USER_MAX_NAME_LEN)) break;
		list = userlistP_tail(list);
	}

	return list;
}

// Libera todos los elementos de la lista.
void userlist_deleteAll(UserList list) {
	User* usr;
	if (NULL == list) return;

	usr = userlistP_head(list);
	while (NULL != usr) {
		list = userlistP_tail(list);
		free(usr);
		usr = userlistP_head(list);
	}
}

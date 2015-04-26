
/* std */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include <pthread.h>
/* redes2 */
#include <redes2/irc.h>
/* usr */
#include "G-2301-05-P3-user.h"
#include "G-2301-05-P3-config.h"
#include "G-2301-05-P3-switches.h"

typedef enum UserState {
	US_RECEIVED_PASS	= (1<<0),
	US_RECEIVED_NICK	= (1<<1),
	US_RECEIVED_USER	= (1<<2),
	US_ALIVE        	= (1<<3),
	US_PING         	= (1<<4),
} UserState;

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
	char        	buffer_recv[IRC_MAX_CMD_LEN+1];	/* Buffer de recepcion               	*/
	char*       	prefix;                        	/* Prefijo                           	*/
	char        	nick[USER_MAX_NICK_LEN+1];     	/* Nickname                          	*/
	char        	name[USER_MAX_NAME_LEN+1];     	/* Nombre                            	*/
	char        	host[64+1];                    	/* Nombre                            	*/
	char        	rname[USER_MAX_RNAME_LEN+1];   	/* Nombre real                       	*/
	char        	away_msg[USER_MAX_AWAY_LEN+1]; 	/* Mensaje de away                   	*/
	SSock*      	ss;                            	/* Descriptor del socket             	*/
	UserFlags   	flags;                         	/* Banderas del usuario              	*/
	UserState   	conn_state;                    	/* Flag para los comandos de registro	*/
	Server*     	server;                        	/* Servidor al que pertenece         	*/
	struct User*	next;                          	/* Puntero al siguiente usuario      	*/
	pthread_t   	thread;                        	/* Hilo                              	*/
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
			long ret = exec_cmd_USER(serv, usr, buf, NULL, NULL, cmd);
			if (OK == ret) usr->conn_state |= US_RECEIVED_USER;
			return ret;

		default:
			LOG("Comando no reconcido en el handshake");
			char* sprefix;
			server_get_name(serv, &sprefix);
			IRC_ErrNotRegisterd(buf, sprefix, "*");
			user_send_cmd(usr, buf);
			free(sprefix);
			return OK;
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

				if (0 == (US_RECEIVED_USER & usr->conn_state)) {
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

	while (1) {
		len_buf = strlen(usr->buffer_recv);
		len = ssock_recv(usr->ss, usr->buffer_recv+len_buf, IRC_MAX_CMD_LEN-len_buf);
		if (!(US_ALIVE | usr->flags)) break; // Debemos morirnos si teniamos una peticion pendiente
		if (0 > len) {
			if (EAGAIN == errno || EINTR == errno) continue; // timeout o interrupcion
			break;
		}
		usr->buffer_recv[len+len_buf] = '\0';
		userP_process_commands(usr, usr->buffer_recv);
	}

	userE_die(usr);
	return NULL; // Nunca llega aqui
}

// Crea una nueva estructura usuario a partir de un socket.
User* user_new(Server* serv, SSock* ss) {
	User* usr = ecalloc(1, sizeof *usr);

	// Generamos el hostname
	struct sockaddr_in address;
	socklen_t addr_len = sizeof address;
	getsockname(ssock_get_fd(ss), (struct sockaddr*)&address, &addr_len);
	strncpy(usr->host, inet_ntoa(address.sin_addr), USER_MAX_HOST_LEN);

	// Ponemos un timeout de 3 al recv()
	struct timeval tv;
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	setsockopt(ssock_get_fd(ss), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);

	strcpy(usr->name, "*");
	strcpy(usr->nick, "*");

	usr->server     = serv;
	usr->ss         = ss;
	usr->conn_state = US_ALIVE | US_PING;
	if (OK != pthread_create(&usr->thread, NULL, userP_reader_thread, usr)) {
		free(usr);
		return NULL;
	}
	pthread_detach(usr->thread);
	return usr;
}

// Peticion de que muera el hilo
long user_kill(User* usr) {
	if (NULL == usr) return ERR;
	if (!(US_ALIVE & usr->conn_state)) return OK;
	usr->conn_state &= ~US_ALIVE;
	return OK;
}
// Mata el usuario INMEDIATAMENTE. Usar con precaucion. No devuelve si funciona
long userE_die(User* usr) {
	if (NULL == usr) return ERR;
	if (!pthread_equal(pthread_self(), usr->thread)) return ERR;

	LOG("Fin del usuario");
	ssock_close(usr->ss);
	pthread_exit(NULL);
	if (usr->conn_state & US_RECEIVED_USER) server_delete_user(usr);
}

long user_ping(User* usr) {
	if (NULL == usr) return ERR;

	// Si ya esta muerto, no hay nada que hacer
	if (!(US_ALIVE & usr->conn_state)) return 0; // Ya esta muerto

	if (US_PING & usr->conn_state) {
		// Esta a 1, lo ponemos a 0
		usr->conn_state &= ~US_PING;
		// y le mandamos un ping
		char buf[IRC_MAX_CMD_LEN+1];
		char* serv_name;
		server_get_name(usr->server, &serv_name);
		IRC_Ping(buf, NULL, serv_name, NULL);
		user_send_cmd(usr, buf);
		free(serv_name);

		return 1;
	}
	else {
		// Esta a 0, no se recibio el pong asi que le matamos
		user_kill(usr);
		return 0;
	}
}
long user_pong(User* usr) {
	if (NULL == usr) return ERR;
	usr->conn_state |= US_PING;
	return OK;
}

// Destruye la estructura y cierra el socket.
void user_delete(User* usr) {
	if (NULL == usr) return;
	ssock_close(usr->ss);
	free(usr);
}

long user_init_prefix(User* usr) {
        char prefix_temporal[1000];
        if (NULL == usr) return ERR;
        free(usr->prefix);
        //IRC_Prefix(&usr->prefix, usr->nick, usr->name, usr->host, usr->host);
        snprintf(prefix_temporal, sizeof prefix_temporal, "%s!~%s@%s", usr->nick, usr->name, usr->host);
        usr->prefix = estrdup(prefix_temporal);
        return OK;
}

long user_get_prefix(User* usr, char** prefix) {
	if (NULL == usr || NULL == prefix) return ERR;
	if (NULL == usr->prefix) {
		*prefix = NULL;
		return ERR;
	}
	*prefix = estrdup(usr->prefix);
	return OK;
}

// Envia un comando al usuario.
long user_send_cmd(User* usr, char* str) {
	if (NULL == usr || NULL == str) return ERR;
	ssize_t bytesSent = ssock_send(usr->ss, str, strlen(str));
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

// Devuelve el host del usuario.
long user_get_host(User* usr, char** host) {
        if (NULL == usr || NULL == host) return ERR_NEEDMOREPARAMS;

        *host = strdup(usr->host);
        return OK;
}

// Devuelve el nick del usuario.
long user_get_nick(User* usr, char** nick) {
	if (NULL == usr || NULL == nick) return ERR_NEEDMOREPARAMS;

	*nick = estrdup(usr->nick);
	return OK;
}

// Cambia el nick del usuario.
long user_set_nick(User* usr, char* nick) {
	if (NULL == usr || NULL == nick) return ERR_NEEDMOREPARAMS;

	// Comprobamos la validez
	//long ret = IRC_IsValid(nick, 0, NULL, IRC_USER);
	//if (IRC_OK != ret) return ret;

	strncpy(usr->nick, nick, USER_MAX_NICK_LEN);
	user_init_prefix(usr);
	return OK;
}

// Devuelve el nombre del usuario.
long user_get_name(User* usr, char** name) {
	if (NULL == usr || NULL == name) return ERR_NEEDMOREPARAMS;

	*name = estrdup(usr->name);
	return OK;
}

// Cambia el nombre del usuario.
long user_set_name(User* usr, char* name) {
	if (NULL == usr || NULL == name) return ERR_NEEDMOREPARAMS;

	// Comprobamos la validez
	//long ret = IRC_IsValid(name, 0, NULL, IRC_USER);
	//if (IRC_OK != ret) return ret;

	strncpy(usr->name, name, USER_MAX_NAME_LEN);
	return OK;
}

// Devuelve el nombre real del usuario.
long user_get_rname(User* usr, char** rname) {
	if (NULL == usr || NULL == rname) return ERR_NEEDMOREPARAMS;

	*rname = estrdup(usr->rname);
	return (NULL == rname);
}

// Cambia el nombre real del usuario.
long user_set_rname(User* usr, char* rname) {
	if (NULL == usr || NULL == rname) return ERR_NEEDMOREPARAMS;

	strncpy(usr->rname, rname, USER_MAX_RNAME_LEN);
	return OK;
}

// Devuelve el mensaje de away (si esta away).
long user_get_away(User* usr, char** away_msg) {
	if (NULL == usr) return ERR;
	if (UF_AWAY & usr->flags) {
		*away_msg = estrdup(usr->away_msg);
		return 1;
	}
	else {
		*away_msg = NULL;
		return 0;
	}
}

// Cambia el estado de away del usuario.
long user_set_away(User* usr, char* away_msg) {
	if (NULL == usr) return ERR;
	if (NULL == away_msg) {
		usr->flags &= ~UF_AWAY;
	}
	else {
		usr->flags |= UF_AWAY;
		strncpy(usr->away_msg, away_msg, USER_MAX_AWAY_LEN);
	}
	return OK;
}

// Cambia de una letra de flag de usuario a su mascara correspondiente.
static UserFlags userP_char_to_flag(char flag) {
	switch (flag) {
		case 'a': return UF_AWAY;
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
	UNUSED(actor);
	UserFlags flag_mask;
	if (NULL == usr) return ERR_NEEDMOREPARAMS;

	// No puedes cambiar tu flag de away con MODE
	if ('a' == flag) return ERR_NOPRIVILEGES;

	// No puedes hacerte operador!!
	if ('o' == flag || 'O' == flag) return ERR_NOPRIVILEGES;

	// Convertimos la flag a una mascara
	flag_mask = userP_char_to_flag(flag);
	if (0 == flag_mask) return ERR_UMODEUNKNOWNFLAG;

	// Exito!
	usr->flags |= flag_mask;
	return OK;
}

// Quita una flag de un usuario.
long user_unset_flag(User* usr, char flag, User* actor) {
	UNUSED(actor);
	UserFlags flag_mask;
	if (NULL == usr) return ERR_NEEDMOREPARAMS;

	// No puedes cambiar tu flag de away con MODE
	if ('a' == flag) return ERR_NOPRIVILEGES;
	// No puedes quitarte la flag de restricted
	if ('r' == flag) return ERR_NOPRIVILEGES;

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
#define userlistP_tail(list)	(&userlistP_head(list)->next)

User* userlist_head(UserList list) {
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
	if (NULL == usr) return NULL;
	*list = usr->next;
	usr->next = NULL;
	return usr;
}

// Busca un elemento por su nombre.
UserList userlist_findByNickname(UserList list, char* name) {
	if (NULL == list || NULL == name) return NULL;

	while (1) {
		User* usr = userlistP_head(list);
		if (NULL == usr) return NULL;
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
		if (NULL == usr) return NULL;
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


/* std */
#include <stdarg.h>
#include <string.h>
/* redes2 */
#include <redes2/irc.h>
/* usr */
#include "G-2301-05-P2-user.h"

struct User {
	char        	buffer_recv[IRC_MAX_CMD_LEN+1];	/* Buffer de recepcion         	*/
	char        	pre[USER_MAX_PRE_LEN+1];       	/* Prefijo                     	*/
	char        	nick[USER_MAX_NICK_LEN+1];     	/* Nickname                    	*/
	char*       	name[USER_MAX_NAME_LEN+1];     	/* Nombre                      	*/
	char*       	rname[USER_MAX_RNAME_LEN+1];   	/* Nombre real                 	*/
	char*       	awaymsg[USER_MAX_AWAY_LEN+1];  	/* Mensaje de away             	*/
	int         	sock_fd;                       	/* Descriptor del socket       	*/
	Server*     	server;                        	/* Servidor al que pertenece   	*/
	struct User*	next;                          	/* Puntero al siguiente usuario	*/
	pthread_t   	thread;                        	/* Hilo                        	*/
};

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
			action_switch(usr, cmd);
			server_up_semaforo(usr->server);
			str = NULL;
			break;

		case IRC_EOP:
			memset(usr->buffer_recv, sizeof(usr->buffer_recv), 0);
			strncpy(usr->buffer_recv, cmd, sizeof(usr->buffer_recv));
			return OK;
		}
	}
	return OK;
}

// Funcion que ejecuta el hilo lector.
static void* userP_reader_thread(void* data) {
	User* usr = data;
	char buffer[512];
	ssize_t len;
	size_t len_buf;

	while (1) {
		len_buf = strlen(usr->buffer_recv);
		len = recvfrom(usr->sock_fd, usr->buffer_recv+len_buf, IRC_MAX_CMD_LEN-len_buf, 0);
		if (len <= 0) return NULL; // Se cierra la conexion
		usr->buffer_recv[len+len_buf] = '\0';
	}
}

// Crea una nueva estructura usuario a partir de un socket.
User* user_new(Server* serv, int sock) {
	User* usr = ecalloc(1, sizeof *usr);
	usr->server  = serv;
	usr->sock_fd = sock;
	if (OK != pthread_create(usr->thread, NULL, userP_reader_thread, usr)) {
		free(usr);
		return NULL;
	}
	pthread_detach(usr->thread);
	return usr;
}

// Destruye la estructura y cierra el socket.
void user_delete(User* usr) {
	if (usr == NULL) return;
	close(sock_fd);
	free(usr);
}

void user_init_prefix(User* usr) {
	(void)usr;
}

// Envia un comando al usuario.
int user_send_cmd(User* usr, const char* str) {
	if (usr == NULL) return ERR;
	ssize_t bytesSent = send(usr->sock_fd, str, strlen(str), 0);
	if (bytesSent < -1) return -1;
}

// Envia un comando con formato a un usuario.
int user_send_cmdf(User* usr, const char* fmt, ...) {
	if (usr == NULL) return ERR;
	char buffer[IRC_MAX_CMD_LEN+1];
	va_list ap;
	va_start(ap, fmt);
	vsnprint(buffer, sizeof buffer, fmt, ap);
	int ret = user_send_cmd(usr, buffer);
	va_end(ap);
	return ret;
}

// Devuelve el nick del usuario.
int user_get_nick(User* usr, const char** nick) {
	if (usr == NULL) return ERR;
	*nick = user->nick;
	return OK;
}

// Cambia el nick del usuario.
int user_set_nick(User* usr, const char* nick) {
	if (usr == NULL) return ERR;
	strcpy(usr->nick, nick, USER_MAX_NICK_LEN);
	return OK;
}

// Devuelve el nombre del usuario.
int user_get_name(User* usr, const char** name) {
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
int user_get_rname(User* usr, const char** rname) {
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
	usr->away_msg = away_msg;
	return OK;
}

// ============================================================================
//   Funciones de listas
// ============================================================================

// Macros
#define userlist_head(list)	(*(list))
#define userlist_tail(list)	(&(*(list)->next))

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
User userlist_extract(UserList list) {
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

	while (userlist_head(list) != NULL) {
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

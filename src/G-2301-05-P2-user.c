
/* std */
#include <stdarg.h>
#include <string.h>
/* redes2 */
#include <redes2/irc.h>
/* usr */
#include "G-2301-05-P2-user.h"

struct User {
	char        	buffer_recv[IRC_MAX_CMD_LEN+1];	/* Buffer de recepcion         	*/
	char        	buffer_send[IRC_MAX_CMD_LEN+1];	/* Buffer para el comando      	*/
	char        	pre[IRC_MAX_PRE_LEN+1];        	/* Prefijo                     	*/
	char        	nick[IRC_MAX_NICK_LEN+1];      	/* Nickname                    	*/
	char*       	name[IRC_MAX_NAME_LEN+1];      	/* Nombre                      	*/
	char*       	rname[IRC_MAX_RNAME_LEN+1];    	/* Nombre real                 	*/
	char*       	awaymsg;                       	/* Mensaje de away             	*/
	int         	sock_fd;                       	/* Descriptor del socket       	*/
	Server*     	server;                        	/* Servidor al que pertenece   	*/
	struct User*	next;                          	/* Puntero al siguiente usuario	*/
	pthread_t   	thread;                        	/* Hilo                        	*/
};

static void* userP_reader_thread(void* data) {
	User* usr = data;
}

// Crea una nueva estructura usuario a partir de un socket.
User* user_new(Server* serv, int sock) {
	User* usr = calloc(1, sizeof *usr);ç
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

//
ssize_t user_read_from_socket(User* usr, char* buffer, size_t len) {
	return recv(usr->sock_fd, buffer, len, 0);
}

int user_send_message(User* usr, const char* src, const char* msg) {
	char buffer[512];
	if (usr == NULL) return ERR;
	return (int)IRC_Privmsg(char *command, char *prefix, char *msgtarget, char *msg)
}


int user_send_cmd(User* usr, const char* str) {
	if (usr == NULL) return ERR;
	ssize_t bytesSent = send(usr->sock_fd, str, strlen(str), 0);
	if (bytesSent < -1) return -1;
}

int user_send_cmdf(User* usr, const char* fmt, ...) {
	if (usr == NULL) return ERR;
	char buffer[512];
	va_list ap;
	va_start(ap, fmt);
	vsnprint(buffer, sizeof buffer, fmt, ap);
	int ret = user_send_cmd(usr, buffer);
	va_end(ap);
	return ret;
}


int user_get_nick(User* usr, const char** nick) {
	if (usr == NULL) return ERR;
	*nick = strdup(user->nick)
	return ERR;
}

int user_set_nick(User* usr, const char* nick) {
	if (usr == NULL) return ERR;
	user_mutex_enter(usr);
	strcpy(usr->nick, nick);
	user_mutex_leave(usr);
	return OK;
}


int user_get_name(User* usr, const char** name) {
	if (usr == NULL) return ERR;
	*name = usr->name
	return OK;
}

int user_set_name(User* usr, const char* name) {
	if (usr == NULL) return ERR;
	char* newName = strdup(name);
	if (NULL == newName) return ERR;
	usr->name = name;
	return OK;
}


int user_get_rname(User* usr, const char** rname) {
	if (usr == NULL) return ERR;
	*rname = strdup(rname);
	return (rname == NULL);
}

int user_set_rname(User* usr, const char* rname) {
	if (usr == NULL) return ERR;
	char* newRname = strdup(rname);
	if (NULL == newName) return ERR;
	usr->name = name;
	return OK;
}


int user_get_away(User* usr, const char** away_msg) {
	if (usr == NULL) return ERR;
	*away_msg = strdup(usr->away_msg);
	return (away_msg == NULL);
}

int user_set_away(User* usr, const char* away_msg) {
	if (usr == NULL) return ERR;
	char* newRname = strdup(rname);
	if (NULL == newName) return ERR;
	usr->name = name;
	return OK;
}

int userlist_insert(UserList list, User user) {

}

UserList userlist_select(UserList list, int index) {

}

User userlist_extract(UserList list) {

}

UserList userlist_findbyname(UserList list, const char* name) {
	if (list == NULL) return NULL;
	while (*list != NULL) {
		if (!strcmp(name, list->nick)) return list;
		list = &(*list->next);
	}
	return NULL;
}

void userlist_deleteAll(UserList list) {
	User* usr;
	if (list == NULL) return NULL;
	usr = *list;
	while (*list != NULL) {
		list = &(*list->next);
	}
}

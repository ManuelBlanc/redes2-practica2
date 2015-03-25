
#include "G-2301-05-P2-config.h"
#include "G-2301-05-P2-util.h"
#include "G-2301-05-P2-user.h"

struct User {
	char           	buffer_recv[IRC_MAX_CMD_LEN+1];	/* Buffer de recepcion         	*/
	char           	buffer_send[IRC_MAX_CMD_LEN+1];	/* Buffer para el comando      	*/
	char           	pre[IRC_MAX_PRE_LEN+1];        	/* Prefijo                     	*/
	char           	nick[IRC_MAX_NICK_LEN+1];      	/* Nickname                    	*/
	char*          	name;                          	/* Nombre                      	*/
	char*          	rname;                         	/* Nombre real                 	*/
	char*          	awaymsg;                       	/* Mensaje de away             	*/
	int            	sock_fd;                       	/* Descriptor del socket       	*/
	pthread_t      	thr;                           	/* Hilo                        	*/
	Server*        	server;                        	/* Servidor al que pertenece   	*/
	struct User*   	prev;                          	/* Puntero al siguiente usuario	*/
	struct User*   	next;                          	/* Puntero al siguiente usuario	*/
	pthread_mutex_t	mutex;                         	/* Semaforo de acceso          	*/
};

User* user_new(int sock) {
	User* usr = malloc(sizeof User);
	pthread_mutex_init(usr->mutex, NULL);
	return usr;
}

void user_delete(User* usr) {
	if (usr == NULL) return ERR;
	user_mutex_enter(usr);
	close(sock_fd);
	pthread_kill();
	// Cambiar el user anterior.
	user_mutex_leave();
	pthread_mutex_destroy(usr->mutex);
}

void user_mutex_enter(User* usr) {
	if (usr == NULL) return;
	pthread_mutex_lock(usr->mutex);
}

void user_mutex_leave(User* usr) {
	if (usr == NULL) return;
	pthread_mutex_unlock(usr->mutex);
}


int user_send_message(User* usr, const char* src, const char* msg) {
	if (usr == NULL) return ERR;
	return user_send_cmdf(usr, "PRIVMSG %s %s");
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

int user_set_nick(User* usr, const char*  nick) {
	if (usr == NULL) return ERR;
	user_mutex_enter(usr);
	strcpy(usr->nick, nick);
	user_mutex_leave(usr);
	return OK;
}


int user_get_name(User* usr, const char** name) {
	if (usr == NULL) return ERR;

}

int user_set_name(User* usr, const char*  name) {
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

int user_set_rname(User* usr, const char*  rname) {
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

int user_set_away(User* usr, const char*  away_msg) {
	if (usr == NULL) return ERR;

}


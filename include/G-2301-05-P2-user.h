#ifndef USER_H
#define USER_H

#define IRC_MAX_CHANNEL_LEN	(200)	// RFC1459 1.3
#define IRC_MAX_NICK_LEN   	(9)  	// RFC1459 1.2
#define IRC_MAX_PRE_LEN    	(64) 	// Decision arbitraria
#define IRC_MAX_CMD_LEN    	(512)	//

// Ordenamos los campos de mayor a menor para evitar fragmentacion
typedef struct User {
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
} User;

User* user_new(int sock);
void user_delete(User* usr);
int user_create_prefix(User* usr);

int user_mutex_enter(User* usr);
int user_mutex_leave(User* usr);

int user_send_message(User* usr, const char* str);

int user_send_cmd(User* usr, const char* str);
int user_send_cmdf(User* usr, const char* fmt, ...);

int user_get_nick(User* usr, const char** nick);
int user_set_nick(User* usr, const char*  nick);

int user_get_name(User* usr, const char** name);
int user_set_name(User* usr, const char*  name);

int user_get_rname(User* usr, const char** rname);
int user_set_rname(User* usr, const char*  rname);

int user_get_away(User* usr, const char** message);
int user_set_away(User* usr, const char*  message);

/*
** ==============================================
**     FUNCIONES DE LISTA
** ==============================================
*/

/*
 * Los objetos tienen un campo 'next' asi que los objetos
 * son los propios nodos de la lista, y una lista
 * es simplemente un puntero al primero de la lista.
 */
typedef struct User**   UserList;

/* Convierte un monstruo en la cabeza de una lista para que
 * se puedan insertar y extraer elementos.
 */
#define userlist(chan)      (&(chan))


int     	userlist_insert(UserList list, User chan);
UserList	userlist_select(UserList list, int index);
User    	userlist_extract(UserList list);
UserList	userlist_findByName(UserList list, const char* name);
void    	userlist_deleteAll(UserList list);

#endif USER_H

#ifndef USER_H
#define USER_H

#define IRC_MAX_CHANNEL_LEN	(200)	// RFC1459 1.3
#define IRC_MAX_NICK_LEN   	(9)  	// RFC1459 1.2
#define IRC_MAX_NAME_LEN   	(20) 	// Decisiones arbitrarias
#define IRC_MAX_RNAME_LEN  	(30) 	// Decisiones arbitrarias
#define IRC_MAX_PRE_LEN    	(64) 	// Decision arbitraria
#define IRC_MAX_CMD_LEN    	(512)	//

// Ordenamos los campos de mayor a menor para evitar fragmentacion
typedef struct User* User;

User* user_new(int sock);
void user_delete(User* usr);

int user_init_prefix(User* usr);

int user_read_from_socket(User* usr, char* buffer, size_t len);
int user_get_socket(User* usr);

int user_send_message(User* usr, const char* who, const char* str);

int user_send_cmd(User* usr, const char* str);
int user_send_cmdf(User* usr, const char* fmt, ...);

int user_get_nick(User* usr, const char** nick);
int user_set_nick(User* usr, const char*  nick);

int user_get_name(User* usr, const char** name);
int user_set_name(User* usr, const char*  name);

int user_get_rname(User* usr, const char** rname);
int user_set_rname(User* usr, const char*  rname);

//
int user_get_away(User* usr, const char** message);
int user_set_away(User* usr, const char*  message);

/*
** ==============================================
**     FUNCIONES DE LISTA
** ==============================================
*/

typedef struct User**   UserList;

/* Convierte un monstruo en la cabeza de una lista para que
 * se puedan insertar y extraer elementos.
 */
#define userlist(user)      (&(user))


int     	userlist_insert(UserList list, User user);
UserList	userlist_select(UserList list, int index);
User    	userlist_extract(UserList list);
UserList	userlist_findByName(UserList list, const char* name);
void    	userlist_deleteAll(UserList list);

#endif /*USER_H*/

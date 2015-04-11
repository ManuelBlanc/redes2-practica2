#ifndef USER_H
#define USER_H

typedef struct User   User;
typedef struct User** UserList;

#include "G-2301-05-P2-util.h"
#include "G-2301-05-P2-server.h"

// Crear y destruir estructuras
User* user_new(Server* serv, int sock);
void user_delete(User* usr);

int user_init_prefix(User* usr);
int user_get_prefix(User*usr, char** prefix);

// Mandar comandos a un usuario
int user_send_cmd(User* usr, char* str);
int user_send_cmdf(User* usr, char* fmt, ...)
ATTRIBUTE((format(printf, 2, 3)));

// Cambiar el nick
int user_get_nick(User* usr, char** nick);
int user_set_nick(User* usr, char*  nick);

// Cambiar el nombre
int user_get_name(User* usr, char** name);
int user_set_name(User* usr, char*  name);

// Cambiar el nombre real
int user_get_rname(User* usr, char** rname);
int user_set_rname(User* usr, char*  rname);

// Cambiar el mensaje de away
int user_get_away(User* usr, char** message);
int user_set_away(User* usr, char*  message);

/*
** ==============================================
**     FUNCIONES DE LISTA
** ==============================================
*/

int     	userlist_insert(UserList list, User* user);
User*   	userlist_extract(UserList list);
UserList	userlist_findByUsername(UserList list, char* name);
UserList	userlist_findByNickname(UserList list, char* name);
void    	userlist_deleteAll(UserList list);

#endif /* USER_H */

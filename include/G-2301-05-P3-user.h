#ifndef USER_H
#define USER_H

typedef struct User   User;
typedef struct User** UserList;

#include "G-2301-05-P3-util.h"
#include "G-2301-05-P3-server.h"
#include "G-2301-05-P3-ssock.h"

// Crear y destruir estructuras
User* user_new(Server* serv, SSock* sock);
void user_delete(User* usr);

long user_init_prefix(User* usr);
long user_get_prefix(User*usr, char** prefix);

// Mandar comandos a un usuario
long user_send_cmd(User* usr, char* str);
long user_send_cmdf(User* usr, char* fmt, ...)
ATTRIBUTE((format(printf, 2, 3)));

long user_get_host(User* usr, char** host);

// Para cerrar una conexion con un usuario
long userE_die(User* usr);

// Funciones de flags
long user_has_flag(User* usr, char flag);
long user_set_flag(User* usr, char flag, User* actor);
long user_unset_flag(User* usr, char flag, User* actor);

// Ping-pong
long user_pong(User* usr);

// Cambiar el nick
long user_get_nick(User* usr, char** nick);
long user_set_nick(User* usr, char*  nick);

// Cambiar el nombre
long user_get_name(User* usr, char** name);
long user_set_name(User* usr, char*  name);

// Cambiar el nombre real
long user_get_rname(User* usr, char** rname);
long user_set_rname(User* usr, char*  rname);

// Cambiar el mensaje de away
long user_get_away(User* usr, char** message);
long user_set_away(User* usr, char*  message);

/*
** ==============================================
**     FUNCIONES DE LISTA
** ==============================================
*/

User*    userlist_head(UserList list);
UserList userlist_tail(UserList list);
int      userlist_insert(UserList list, User* user);
User*    userlist_extract(UserList list);
UserList userlist_findByUsername(UserList list, char* name);
UserList userlist_findByNickname(UserList list, char* name);
void     userlist_deleteAll(UserList list);

#endif /* USER_H */

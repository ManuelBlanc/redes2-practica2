#ifndef SERVER_H
#define SERVER_H

typedef struct Server Server;

// Orden importante para que no haya recurrencia ciclica
#include "G-2301-05-P2-user.h"
#include "G-2301-05-P2-channel.h"

typedef struct Server {
        int             sock;           /* Socket que recibe peticiones                 */
        UserList   	usrs;	        /* Lista de usuarios                            */
        ChannelList	chan;           /* Lista de canales                             */
        pthread_t       select_thr;     /* Hilo para la funcion select()                */
        fd_set          fd_read;        /* Descriptores de socket                       */
} Server;

/* Funciones solo de servidor */

/**
 * Crea una estructura para un servidor
 * @return estructura servidor
 */
Server* server_new(void);

/**
 * Arranca el servidor y se pone a la escucha de peticiones
 */
void server_init(void);

/**
 * Acepta peticiones de conexion y pre-registra al nuevo usuario
 * @param serv servidor
 * @return codigo de error
 */
int server_accept(Server* serv);

/**
 * Hilo que ejecuta la funcion select
 * @param serv servidor
 */
void server_select(Server* serv);

/**
 * Añade un descriptor a la lista en la que se hace sondeo
 * @param serv servidor
 * @param sock descriptor de socket nuevo
 */
void server_add_new_sockdesc(Server* serv, int sock);

/**
 * Elimina un descriptor de la lista en la que se hace sondeo
 * @param serv servidor
 * @param sock descriptor de socket a eliminar
 */
void server_remove_sockdesc(Server* serv, int sock);

/* Funciones relacionadas con la lista de usuarios */

/**
 * Comprueba si existe un usuario con ese nick
 * @param nick nick a buscar en la lista de usuarios
 * @return OK si lo encontro y ERR si no
 */
int server_is_nick_used(const char* nick);

/**
 * Añade un nuevo usuario a la lista de usuarios conectados
 * @param serv servidor
 * @param user nuevo usuario
 * @return codigo de error
 */
int server_add_user(Server* serv, User* user);

/**
 * Elimina un usuario de la lista de usuarios conectados
 * @param serv servidor
 * @param name nombre del usuario a eliminar
 * @return codigo de error
 */
int server_delete_user(Server* serv, const char* name);

/* Funciones relacionadas con la lista de canales */

/**
 * Añade un nuevo canal a la lista de canales existentes
 * @param serv servidor
 * @param chan nuevo canal
 * @return codigo de error
 */
int server_add_channel(Server* serv, Channel* chan);

/**
 * Elimina un canal de la lista de canales existentes
 * @param serv servidor
 * @param chan nombre del canal a eliminar
 * @return codigo de error
 */
int server_delete_channel(Server* serv, const char* chan);



/* Necesitamos un semaforo para proteger el recorrido de las listas
 porque si eliminan o insertan un nodo cuando la estas recoriendo:muerte y destruccion*/

/* Otro semaforo (en cada hilo probablemente) para evitar que cuando queremos enviar a un usuario
 se desconete en ese momento y pete*/

#endif /* SERVER_H */

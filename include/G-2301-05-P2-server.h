#ifndef SERVER_H
#define SERVER_H

typedef struct Server Server;

// Orden importante para que no haya recurrencia ciclica
#include "G-2301-05-P2-user.h"
#include "G-2301-05-P2-channel.h"

typedef struct ServerAdmin {
	char loc1[200]; 	/* Datos del administrador */
	char loc2[200]; 	/* Datos del administrador */
	char email[200];	/* Datos del administrador */
} ServerAdmin;

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
 * Hace down del semaforo que controla la ejecucion de comandos
 * @param serv servidor
 */
void server_down_semaforo(Server* serv);

/**
 * Hace up del semaforo que controla la ejecucion de comandos
 * @param serv servidor
 */
void server_up_semaforo(Server* serv);


/* Funciones relacionadas con las listas de usuarios o canales */

int server_get_admin(Server* serv, ServerAdmin** sa);

/**
 * Devuelve el nombre del servidor
 * @param serv servidor
 * @param name donde se almacenara el nombre
 * @return OK o ERR
 */
int server_get_name(Server* serv, char** name);

/**
 * Devuelve la lista de usuarios
 * @param serv servidor
 * @return lista de usuarios
 */
UserList server_get_userlist(Server* serv);

/**
 * Devuelve la lista de usuarios desconectados
 * @param serv servidor
 * @return lista de usuarios desconectados
 */
UserList server_get_disconnectlist(Server* serv);

/**
 * Devuelve la lista de canales
 * @param serv servidor
 * @return lista de canales
 */
ChannelList server_get_channellist(Server* serv);

/**
 * Devuelve el numero de usuarios conectados
 * @param serv servidor
 * @return numero de usuarios
 */
int server_get_num_users(Server* serv);

/**
 * Devuelve el numero de canales que existen
 * @param serv servidor
 * @return numero de canales
 */
int server_get_num_channels(Server* serv);

/**
 * Guarda en un argumento el nombre del fichero donde se encuentra el motd
 * @param motd_path nombre del fichero
 * @return OK
 */
int server_get_motd(char** motd_path);

/**
 * Añade un nuevo usuario a la lista de usuarios conectados
 * @param serv servidor
 * @param usr nuevo usuario
 * @return codigo de error
 */
int server_add_user(Server* serv, User* usr);

/**
 * Elimina un usuario de la lista de usuarios conectados
 * @param serv servidor
 * @param name nombre del usuario a eliminar
 * @return codigo de error
 */
int server_delete_user(Server* serv, char* name);

/**
 * Añade un nuevo usuario a la lista de usuarios desconectados
 * @param serv servidor
 * @param usr usuario desconectado
 * @return codigo de error
 */
int server_add_disconnect(Server* serv, User* usr);

/* Funciones relacionadas con la lista de canales */

/**
 * Añade un nuevo canal a la lista de canales existentes
 * @param serv servidor
 * @param name nombre del nuevo canal
 * @return codigo de error
 */
int server_add_channel(Server* serv, char* name);

/**
 * Elimina un canal de la lista de canales existentes
 * @param serv servidor
 * @param chan nombre del canal a eliminar
 * @return codigo de error
 */
int server_delete_channel(Server* serv, char* name);



/* Necesitamos un semaforo para proteger el recorrido de las listas
 porque si eliminan o insertan un nodo cuando la estas recoriendo:muerte y destruccion*/

/* Otro semaforo (en cada hilo probablemente) para evitar que cuando queremos enviar a un usuario
 se desconete en ese momento y pete*/

#endif /* SERVER_H */

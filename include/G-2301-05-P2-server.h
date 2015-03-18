#ifndef SERVER_H
#define SERVER_H

typedef struct Server {
        int             sock;           /* Socket que recibe peticiones                 */
        UserList   	usrs;	        /* Lista de usuarios                            */
        ChannelList	chan;           /* Lista de canales                             */
        pthread_mutex_t	usr_mutex;      /* Semaforo de acceso a la lista de usuarios    */
        pthread_mutex_t	chan_mutex;     /* Semaforo de acceso a la lista de canales     */
} Server;

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
int server_accept(Server serv);

/**
 * Comprueba si existe un usuario con ese nick
 * @param nick nick a buscar en la lista de usuarios
 * @return OK si lo encontro y ERR si no 
 */
int server_is_nick_used(const char* nick);

/* Necesitamos un semaforo para proteger el recorrido de las listas
 porque si eliminan o insertan un nodo cuando la estas recoriendo:muerte y destruccion*/

/* Otro semaforo (en cada hilo probablemente) para evitar que cuando queremos enviar a un usuario
 se desconete en ese momento y pete*/

#endif /* SERVER_H *

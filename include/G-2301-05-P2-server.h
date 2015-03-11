#ifndef SERVER_H
#define SERVER_H

typedef struct Server {
    UserList   	usrs;	/* Lista de usuarios	*/
    ChannelList	chan;	/* Lista de canales 	*/
} Server;

Server* server_new(void);

int server_accept(Server serv);

int server_is_nick_used(const char* nick);

/* Necesitamos un semaforo para proteger el recorrido de las listas
 porque si eliminan o insertan un nodo cuando la estas recoriendo:muerte y destruccion*/

/* Otro semaforo (en cada hilo probablemente) para evitar que cuando queremos enviar a un usuario
 se desconete en ese momento y pete*/

#endif /* SERVER_H *

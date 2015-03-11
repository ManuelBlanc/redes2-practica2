#ifndef CHANNEL_H
#define CHANNEL_H

#include "G-2301-05-P2-user.h"

O - give "channel creator" status;
o - give/take channel operator privilege;
v - give/take the voice privilege;

a - toggle the anonymous channel flag;
i - toggle the invite-only channel flag;
m - toggle the moderated channel;
n - toggle the no messages to channel from clients on the
   outside;
q - toggle the quiet channel flag;
p - toggle the private channel flag;
s - toggle the secret channel flag;
r - toggle the server reop channel flag;
t - toggle the topic settable by channel operator only flag;

k - set/remove the channel key (password);
l - set/remove the user limit to channel;

b - set/remove ban mask to keep users out;
e - set/remove an exception mask to override a ban mask;
I - set/remove an invitation mask to automatically override
   the invite-only flag;

struct UserChannel {
	User*              	usr;
	long               	flags;
	struct UserChannel*	next;
}

typedef struct Channel {
    UserChannel*   	usrs;  	/* Lista de usuarios         	*/
    char*          	name;  	/* Nombre                    	*/
    char*          	topic; 	/* Tema                      	*/
    char*          	psw;   	/* Contraseña                	*/
    unsigned int   	flags; 	/* Flags de un canal         	*/
    Server*        	server;	/* Servidor al que pertenece 	*/
    struct Channel*	next;  	/* Puntero al siguiente canal	*/
} Channel;

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
typedef struct Channel**   ChannelList;

/* Convierte un monstruo en la cabeza de una lista para que
 * se puedan insertar y extraer elementos.
 */
#define channellist(chan)      (&(chan))


int        	channellist_insert(ChannelList list, Channel chan);
ChannelList	channellist_select(ChannelList list, int index);
Channel    	channellist_extract(ChannelList list);
ChannelList	channellist_findByName(ChannelList list, const char* name);
void       	channellist_deleteAll(ChannelList list);

#endif /* CHANNEL_H */

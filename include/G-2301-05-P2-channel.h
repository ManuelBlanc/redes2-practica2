#ifndef CHANNEL_H
#define CHANNEL_H

typedef struct Channel   Channel;
typedef struct Channel** ChannelList;

#include "G-2301-05-P2-util.h"
#include "G-2301-05-P2-user.h"
#include "G-2301-05-P2-server.h"

// Creacion y destruccion de las estructuras
Channel* channel_new(Server* server, char* name);
void channel_delete(Channel* chan);

// Borra un usuario de la lista enlazada del canal
long channel_remove_user(Channel* chan, User* usr);

// Unirse y abandonar (o kickear) del canal
long channel_join(Channel* chan, User* usr, char* key);
long channel_part(Channel* chan, User* usr, User* actor);
long channel_has_user(Channel* chan, User* usr);

// Cambiar las flags de un usuario (no es necesario que este ingresado)
long channel_has_flag_user(Channel* chan, User* usr, char flag);
long channel_set_flag_user(Channel* chan, User* usr, char flag, User* actor);
long channel_unset_flag_user(Channel* chan, User* usr, char flag, User* actor);

// Puede un usuario mandar mensajes?
long channel_can_send_message(Channel* chan, User* usr);

long channel_get_user_names(Channel* chan, char flag, char*** usr_array_ret);

// Mandar comandos
long channel_send_cmd(Channel* chan, char* str);
long channel_send_cmdf(Channel* chan, char* fmt, ...)
ATTRIBUTE((format(printf, 2, 3)));

// Numeros
long channel_get_user_count(Channel* chan);
long channel_get_max_users(Channel* chan);

// Manejo del tema
long channel_get_topic(Channel* chan, char** topic);
long channel_set_topic(Channel* chan, char*  topic, User* actor);

// Manejo del nombre
long channel_get_name(Channel* chan, char** name);

// Manejo de la contraseña
long channel_get_key(Channel* chan, char** key);
long channel_set_key(Channel* chan, char*  key, User* actor);

// Manejo de las flags del canal
long channel_has_flag(Channel* chan, char flag);
long channel_set_flag(Channel* chan, char flag, User* actor);
long channel_unset_flag(Channel* chan, char flag, User* actor);

/*
** ==============================================
**     FUNCIONES DE LISTA
** ==============================================
*/

Channel*    channellist_head(ChannelList list);
ChannelList channellist_tail(ChannelList list);
long        channellist_insert(ChannelList list, Channel* chan);
Channel*    channellist_extract(ChannelList list);
ChannelList channellist_findByName(ChannelList list, char* name);
void        channellist_deleteAll(ChannelList list);

#endif /* CHANNEL_H */

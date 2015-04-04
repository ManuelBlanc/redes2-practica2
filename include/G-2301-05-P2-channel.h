#ifndef CHANNEL_H
#define CHANNEL_H

typedef struct Channel   Channel;
typedef struct Channel** ChannelList;

#include "G-2301-05-P2-util.h"
#include "G-2301-05-P2-user.h"
#include "G-2301-05-P2-server.h"

// Creacion y destruccion de las estructuras
Channel* channel_new(Server* server, const char* name);
void channel_delete(Channel* chan);

// Unirse y abandonar (o kickear) del canal
int channel_join(Channel* chan, User* usr, const char* key);
int channel_part(Channel* chan, User* usr, User* actor);

// Cambiar las flags de un usuario (no es necesario que este ingresado)
int channel_get_flag_user(Channel* chan, User* usr, char flag);
int channel_set_flag_user(Channel* chan, User* usr, char flag, User* actor);
int channel_unset_flag_user(Channel* chan, User* usr, char flag, User* actor);

// Puede un usuario mandar mensajes?
int channel_can_send_message(Channel* chan, User* usr);

// Mandar comandos
int channel_send_cmd(Channel* chan, const char* str);
int channel_send_cmdf(Channel* chan, const char* fmt, ...)
ATTRIBUTE((format(printf, 2, 3)));


// Manejo del tema
int channel_get_topic(Channel* chan, const char** topic);
int channel_set_topic(Channel* chan, const char*  topic, User* actor);

// Manejo del nombre
int channel_get_name(Channel* chan, const char** name);

// Manejo de la contraseña
int channel_get_key(Channel* chan, const char** key);
int channel_set_key(Channel* chan, const char*  key);

// Manejo de las flags del canal
int channel_get_flag(Channel* chan, char flag);
int channel_set_flag(Channel* chan, char flag, User* actor);
int channel_unset_flag(Channel* chan, char flag, User* actor);

/*
** ==============================================
**     FUNCIONES DE LISTA
** ==============================================
*/

int         channellist_insert(ChannelList list, Channel* chan);
Channel*    channellist_extract(ChannelList list);
ChannelList channellist_findByName(ChannelList list, const char* name);
void        channellist_deleteAll(ChannelList list);

#endif /* CHANNEL_H */

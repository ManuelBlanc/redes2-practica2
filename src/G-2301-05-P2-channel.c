
/* std */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* redes2 */
#include <redes2/irc.h>
/* usr */
#include "G-2301-05-P2-config.h"
#include "G-2301-05-P2-util.h"
#include "G-2301-05-P2-channel.h"

// Mascara de flags de usuario
typedef enum ChannelUserFlags {
	UCF_CREATOR   	= (1<<0), // O - give "channel creator" status;
	UCF_OPERATOR  	= (1<<1), // o - give/take channel operator privilege;
	UCF_VOICE     	= (1<<2), // v - give/take the voice privilege;
	UCF_BAN       	= (1<<3), // b - set/remove ban mask to keep users out;
	UCF_EXCEPTION 	= (1<<4), // e - set/remove an exception mask to override a ban mask;
	UCF_INVITATION	= (1<<5), // I - set/remove an invitation mask to automatically override the invite-only flag;
} ChannelUserFlags;

// Mascara de flags de canal
typedef enum ChannelFlags {
	CF_ANONYMOUS	= (1<<0),  // a - toggle the anonymous channel flag;
	CF_INVONLY  	= (1<<1),  // i - toggle the invite-only channel flag;
	CF_MODERATED	= (1<<2),  // m - toggle the moderated channel;
	CF_NOMSGS   	= (1<<3),  // n - toggle the no messages to channel from clients on the outside;
	CF_QUIET    	= (1<<4),  // q - toggle the quiet channel flag;
	CF_PRIVATE  	= (1<<5),  // p - toggle the private channel flag;
	CF_SECRET   	= (1<<6),  // s - toggle the secret channel flag;
	CF_REOP     	= (1<<7),  // r - toggle the server reop channel flag;
	CF_TOPIC    	= (1<<8),  // t - toggle the topic settable by channel operator only flag;
	CF_CHANKEY  	= (1<<9),  // k - set/remove the channel key (password);
	CF_USERLIMIT	= (1<<10), // l - set/remove the user limit to channel;
} ChannelFlags;

// Estructura para la lista enlazada de usuarios interna al canal
typedef struct UserChannelData {
	int                    	inChannel;	/* Esta realmente en el canal?	*/
	User*                  	usr;      	/* La estructura usuario      	*/
	ChannelUserFlags       	flags;    	/* Flags asociados            	*/
	struct UserChannelData*	next;
} UserChannelData;

// Estructura de un canal
typedef struct Channel {
	char            	topic[CHANNEL_MAX_TOPIC_LEN+1];	/* Tema                      	*/
	char            	name[CHANNEL_MAX_NAME_LEN+1];  	/* Nombre                    	*/
	char            	key[CHANNEL_MAX_KEY_LEN+1];    	/* Contraseña                	*/
	unsigned int    	usr_max;                       	/* Maximo numero de usuarios 	*/
	unsigned int    	usr_cnt;                       	/* Numero de usuarios        	*/
	ChannelFlags    	flags;                         	/* Flags de un canal         	*/
	UserChannelData*	usrs;                          	/* Lista de usuarios         	*/
	Server*         	server;                        	/* Servidor al que pertenece 	*/
	struct Channel* 	next;                          	/* Puntero al siguiente canal	*/
} Channel;

// Reserva de memoria. Inicializa el nombre.
Channel* channel_new(Server* server, char* name) {
	Channel* chan;
	if (NULL == server || NULL == name) return NULL;

	chan = ecalloc(1, sizeof *chan);
	strncpy(chan->name, name, CHANNEL_MAX_NAME_LEN);
	chan->server = server;
	chan->usr_max = 10;
	return chan;
}

// Destruccion de un canal. Libera la lista interna de usuarios.
void channel_delete(Channel* chan) {
	if (NULL == chan) return;
	UserChannelData* usrData = chan->usrs;
	while (NULL != usrData) {
		UserChannelData* usrNext = usrData->next;
		free(usrData);
		usrData = usrNext;
	}
	free(chan);
}

// Añade un usuario a la lista interna de usuarios.
static long channelP_add_user(Channel* chan, User* usr, UserChannelData** ucdDst) {
	UserChannelData* ucd = emalloc(sizeof *ucd);
	ucd->usr       = usr;
	ucd->flags     = 0;
	ucd->inChannel = 0;
	ucd->next      = chan->usrs;
	chan->usrs     = ucd;

	*ucdDst = ucd;
	return OK;
}

// Busca y borra un usuario de la lista interna de usuarios.
long channel_remove_user(Channel* chan, User* usr) {
	UserChannelData** ucd = &chan->usrs;
	while (NULL != *ucd) {
		UserChannelData* usrNext = *ucd;
		if (usrNext->usr == usr) {
			*ucd = usrNext->next;
			return OK;
		}
		ucd = &usrNext->next;
	}
	return ERR;
}

// Busca y devuelve un usuario de la lista interna de usuarios.
static long channelP_find_user_data(Channel* chan, User* usr, UserChannelData** ucd) {
	UserChannelData* cud = chan->usrs;
	while (NULL != cud) {
		if (cud->usr == usr) {
			*ucd = cud;
			return 1;
		}
		cud = cud->next;
	}
	return 0;
}

// Busca o crea un usuario en la lista interna de usuarios.
static void channelP_find_or_create(Channel* chan, User* usr, UserChannelData** ucd) {
	if (!channelP_find_user_data(chan, usr, ucd)) {
		// Si no lo hemos encontrado lo creamos.
		channelP_add_user(chan, usr, ucd);
	}
}

// Comprueba si un usuario es operador o NULL.
static long channelP_user_op_or_null(Channel* chan, User* usr) {
	UserChannelData* ucd;
	if (NULL == usr) return 1;
	if (!channelP_find_user_data(chan, usr, &ucd)) return 0;
	return (UCF_OPERATOR & ucd->flags) != 0;
}

// Comprueba si un usuario tiene permiso para mandar un mensaje.
long channel_can_send_message(Channel* chan, User* usr) {
	UserChannelData* ucd;

	// Buscamos al que envia
	if (!channelP_find_user_data(chan, usr, &ucd)) {
		// Si no esta en la sala y se pueden enviar mensajes desde fuera
		if (CF_NOMSGS & chan->flags) return ERR_CANNOTSENDTOCHAN;

		// Solo si no esta moderado
		return (CF_MODERATED & chan->flags) ? OK : ERR_CANNOTSENDTOCHAN;
	}

	// Esta baneado?
	if (UCF_BAN & ucd->flags) {
		// Si no es una excepcion
		if (!(UCF_EXCEPTION & ucd->flags)) return ERR_CANNOTSENDTOCHAN;
	}

	// Si esta moderado, debe tener voz o ser operador
	if (CF_MODERATED & chan->flags) {
		if (!((UCF_OPERATOR | UCF_VOICE) & ucd->flags)) return ERR_CANNOTSENDTOCHAN;
	}

	return OK; // Podemos mandar mensajes!!
}

// Cambia de una letra de flag de usuario a su mascara correspondiente.
static ChannelUserFlags channelP_char_to_usrflag(char flag) {
	switch (flag) {
		case 'O': return UCF_CREATOR;
		case 'o': return UCF_OPERATOR;
		case 'v': return UCF_VOICE;
		case 'b': return UCF_BAN;
		case 'e': return UCF_EXCEPTION;
		case 'I': return UCF_INVITATION;
		default: return 0;
	}
}

long channel_get_user_names(Channel* chan, char flag, char*** usr_array_ret) {
	if (NULL == chan || NULL == usr_array_ret) return ERR;
	char** usr_array = *usr_array_ret = emalloc((sizeof *usr_array) * (chan->usr_cnt+1));

	ChannelUserFlags flag_mask = ~0;
	if (0 != flag) {
		flag_mask = channelP_char_to_usrflag(flag);
		if (0 == flag_mask) return ERR_UMODEUNKNOWNFLAG;
	}

	UserChannelData* cud = chan->usrs;
	while (NULL != cud) {
		if (flag_mask & cud->flags) {
			ASSERT(OK == user_get_nick(cud->usr, usr_array++), "Si fallase esto estariamos en serios problemas");
		}
		cud = cud->next;
	}
	*usr_array++ = NULL;
	return OK;
}


// Manda un comando a todos los usuarios del canal.
long channel_send_cmd(Channel* chan, char* str) {
	UserChannelData* ucd;
	if (NULL == chan) return ERR;
	ucd = chan->usrs;
	while (NULL != ucd) {
		if (ucd->inChannel) user_send_cmd(ucd->usr, str);
		ucd = ucd->next;
	}
	return OK;
}

// Manda un comando (con formateo) a todos los usuarios del canal.
long channel_send_cmdf(Channel* chan, char* fmt, ...) {
	if (NULL == chan) return ERR;
	char buffer[512];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buffer, sizeof buffer, fmt, ap);
	int ret = channel_send_cmd(chan, buffer);
	va_end(ap);
	return ret;
}

long channel_get_user_count(Channel* chan) {
	if (NULL == chan) return ERR;
	return chan->usr_cnt;
}
long channel_get_max_users(Channel* chan) {
	if (NULL == chan) return ERR;
	return chan->usr_max;
}

// Comprueba si un usuario tiene una flag.
long channel_has_flag_user(Channel* chan, User* usr, char flag) {
	UserChannelData* ucd;
	ChannelUserFlags flag_mask;
	if (NULL == chan || NULL == usr) return ERR_NEEDMOREPARAMS;

	// Convertimos la flag a una mascara
	flag_mask = channelP_char_to_usrflag(flag);
	if (0 == flag_mask) return ERR_UMODEUNKNOWNFLAG;

	// Buscamos al objetivo
	if (!channelP_find_user_data(chan, usr, &ucd)) return ERR_USERNOTINCHANNEL;

	return (flag_mask & ucd->flags) != 0;
}

// Pone una flag a un usuario.
long channel_set_flag_user(Channel* chan, User* usr, char flag, User* actor) {
	UserChannelData* ucd;
	ChannelUserFlags flag_mask;
	if (NULL == chan || NULL == usr) return ERR_NEEDMOREPARAMS;

	// Convertimos la flag a una mascara
	flag_mask = channelP_char_to_usrflag(flag);
	if (0 == flag_mask) return ERR_UMODEUNKNOWNFLAG;

	// Hay permisos?
	if (!channelP_user_op_or_null(chan, actor)) return ERR_CHANOPRIVSNEEDED;

	// Buscamos al objetivo
	if (!channelP_find_user_data(chan, usr, &ucd)) return ERR_USERNOTINCHANNEL;

	// Exito!
	ucd->flags |= flag_mask;
	return OK;
}

// Quita una flag de un usuario.
long channel_unset_flag_user(Channel* chan, User* usr, char flag, User* actor) {
	UserChannelData* ucd;
	ChannelUserFlags flag_mask;
	if (NULL == chan || NULL == usr) return ERR_NEEDMOREPARAMS;

	// Convertimos la flag a una mascara
	flag_mask = channelP_char_to_usrflag(flag);
	if (0 == flag_mask) return ERR_UMODEUNKNOWNFLAG;

	// Hay permisos?
	if (!channelP_user_op_or_null(chan, actor)) return ERR_CHANOPRIVSNEEDED;

	// Buscamos al objetivo
	if (!channelP_find_user_data(chan, usr, &ucd)) return ERR_USERNOTINCHANNEL;

	// Exito!
	ucd->flags &= ~flag_mask;
	return OK;
}

// Ingresa un usuario en el canal, con la clave proporcionada.
long channel_join(Channel* chan, User* usr, char* key) {
	UserChannelData* ucd;
	if (NULL == chan || NULL == usr) return ERR_NEEDMOREPARAMS;

	// Esta el canal lleno?
	if (CF_USERLIMIT & chan->flags) {
		if (chan->usr_cnt == chan->usr_max) return ERR_CHANNELISFULL;
	}

	// Buscamos o creamos al usuario
	channelP_find_or_create(chan, usr, &ucd);

	// Ya esta en el canal?
	if (ucd->inChannel) return OK;

	// Esta baneado el usuario?
	if (UCF_BAN & ucd->flags) return ERR_BANNEDFROMCHAN;

	// Hay clave? Se ha proporcionado la clave correcta?
	if (CF_CHANKEY & chan->flags) {
		if (NULL == key) return ERR_BADCHANNELKEY;
		if (strncmp(key, chan->key, CHANNEL_MAX_KEY_LEN)) return ERR_BADCHANNELKEY;
	}

	// Es de solo invitacion?
	if (CF_INVONLY & chan->flags) {
		if ((UCF_INVITATION & (0 == ucd->flags))) return ERR_INVITEONLYCHAN;
	}

	// Si hemos llegado hasta aqui, le apuntamos
	channelP_find_or_create(chan, usr, &ucd);
	ucd->inChannel = 1;
	chan->usr_cnt++;
	return OK;
}

// Abandona un canal. Si actor es distinto de NULL, es una expulsion forzosa.
long channel_part(Channel* chan, User* usr, User* actor) {
	UserChannelData* ucd;
	if (NULL == chan) return ERR_NOSUCHCHANNEL;
	if (NULL == usr) return ERR_NEEDMOREPARAMS;

	if (!channelP_find_user_data(chan, usr, &ucd) || !ucd->inChannel) {
		return ERR_NOTONCHANNEL;
	}

	// Solo los operadores pueden
	if (!channelP_user_op_or_null(chan, actor)) return ERR_CHANOPRIVSNEEDED;

	ucd->inChannel = 0;
	chan->usr_cnt--;
	return OK;
}

// Comprueba si un usuario esta dentro de la sala.
long channel_has_user(Channel* chan, User* usr) {
	UserChannelData* ucd;
	return channelP_find_user_data(chan, usr, &ucd);
}

// Devuelve el topic.
long channel_get_topic(Channel* chan, char** topic) {
	if (NULL == chan) return ERR;
	if ('\0' == chan->topic[0]) {
		*topic = NULL;
	}
	else {
		*topic = estrdup(chan->topic);
	}
	return OK;
}

// Cambia el topic del canal.
long channel_set_topic(Channel* chan, char* topic, User* actor) {
	if (NULL == chan) return ERR_NEEDMOREPARAMS;

	// Solo los operadores pueden cambiar el topic?
	if (CF_TOPIC & chan->flags) {
		// Hay permisos suficientes?
		if (!channelP_user_op_or_null(chan, actor)) return ERR_CHANOPRIVSNEEDED;
	}

	if (NULL == topic) {
		// Si es null el topic lo vaciamos
		chan->topic[0] = '\0';
	}
	else {
		// Sino, copiamos el nuevo
		strncpy(chan->topic, topic, CHANNEL_MAX_TOPIC_LEN);
	}

	return OK;
}

// Devuelve el nombre del canal.
long channel_get_name(Channel* chan, char** name) {
	if (NULL == chan) return ERR;
	*name = estrdup(chan->name);
	return OK;
}

// Devuelve la contraseña del canal.
long channel_get_key(Channel* chan, char** key) {
	if (NULL == chan) return ERR;
	*key = estrdup(chan->key);
	return OK;
}

// Cambia la contraseña del canal.
long channel_set_key(Channel* chan, char* key, User* actor) {
	if (NULL == chan) return ERR;

	// Hay permisos suficientes?
	if (!channelP_user_op_or_null(chan, actor)) return ERR_CHANOPRIVSNEEDED;

	strncpy(chan->key, key, CHANNEL_MAX_KEY_LEN);
	return OK;
}

// Cambia de una letra de flag de canal a su mascara correspondiente.
static ChannelFlags channelP_char_to_channelflag(char flag) {
	switch (flag) {
		case 'a': return CF_ANONYMOUS;
		case 'i': return CF_INVONLY;
		case 'm': return CF_MODERATED;
		case 'n': return CF_NOMSGS;
		case 'q': return CF_QUIET;
		case 'p': return CF_PRIVATE;
		case 's': return CF_SECRET;
		case 'r': return CF_REOP;
		case 't': return CF_TOPIC;
		case 'k': return CF_CHANKEY;
		case 'l': return CF_USERLIMIT;
		default:  return 0;
	}
}

//
long channel_has_flag(Channel* chan, char flag) {
	ChannelFlags flag_mask;
	if (NULL == chan) return 0;

	// Convertimos la flag a una mascara
	flag_mask = channelP_char_to_channelflag(flag);
	if (0 == flag_mask) return ERR_UMODEUNKNOWNFLAG;

	return (flag_mask & chan->flags) != 0;
}

// Pone una flag al canal.
long channel_set_flag(Channel* chan, char flag, User* actor) {
	ChannelFlags flag_mask;
	if (NULL == chan) return ERR;

	// Convertimos la flag a una mascara
	flag_mask = channelP_char_to_channelflag(flag);
	if (0 == flag_mask) return ERR_UMODEUNKNOWNFLAG;

	// Hay permisos suficientes?
	if (!channelP_user_op_or_null(chan, actor)) return ERR_CHANOPRIVSNEEDED;

	chan->flags |= flag_mask;
	return OK;
}

// Quita una flag del canal.
long channel_unset_flag(Channel* chan, char flag, User* actor) {
	ChannelFlags flag_mask;
	if (NULL == chan) return ERR;

	// Convertimos la flag a una mascara
	flag_mask = channelP_char_to_channelflag(flag);
	if (0 == flag_mask) return ERR_UMODEUNKNOWNFLAG;

	// Hay permisos suficientes?
	if (!channelP_user_op_or_null(chan, actor)) return ERR_CHANOPRIVSNEEDED;

	chan->flags &= ~flag_mask;
	return OK;
}

// ============================================================================
//   Funciones de listas
// ============================================================================

// Macros
#define channellistP_head(list)	(*(list))
#define channellistP_tail(list) (&channellistP_head(list)->next)

Channel* channellist_head(ChannelList list) {
	return (NULL != list) ? channellistP_head(list) : NULL;
}
ChannelList channellist_tail(ChannelList list) {
	return (NULL != list) ? channellistP_tail(list) : NULL;
}

// Inserta un elemento en la lista.
long channellist_insert(ChannelList list, Channel* chan) {
	if (NULL == list || NULL == chan) return ERR;

	// Comprobamos que NO este ya en una lista
	if (NULL != chan->next) return ERR;

	chan->next = channellistP_head(list);
	*list = chan;
	return OK;
}

// Extrae el primer elemento de una lista.
Channel* channellist_extract(ChannelList list) {
	Channel* chan;
	if (NULL == list) return NULL;

	chan = channellistP_head(list);
	*list = chan->next;
	chan->next = NULL;
	return chan;
}

// Busca un elemento por su nombre.
ChannelList channellist_findByName(ChannelList list, char* name) {
	if (NULL == list || NULL == name) return NULL;

	while (1) {
		Channel* chan = channellistP_head(list);
		if (NULL == chan) break;
		if (0 == strncmp(name, chan->name, CHANNEL_MAX_NAME_LEN)) break;
		list = channellistP_tail(list);
	}

	return list;
}

// Libera todos los elementos de la lista.
void channellist_deleteAll(ChannelList list) {
	Channel* chan;
	if (NULL == list) return;

	chan = channellistP_head(list);
	while (NULL != chan) {
		list = channellistP_tail(list);
		free(chan);
		chan = channellistP_head(list);
	}
}

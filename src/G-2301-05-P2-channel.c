
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
typedef enum UserFlags {
	USRFLAG_CREATOR   	= (1<<0), // O - give "channel creator" status;
	USRFLAG_OPERATOR  	= (1<<1), // o - give/take channel operator privilege;
	USRFLAG_VOICE     	= (1<<2), // v - give/take the voice privilege;
	USRFLAG_BAN       	= (1<<3), // b - set/remove ban mask to keep users out;
	USRFLAG_EXCEPTION 	= (1<<4), // e - set/remove an exception mask to override a ban mask;
	USRFLAG_INVITATION	= (1<<5), // I - set/remove an invitation mask to automatically override the invite-only flag;
} UserFlags;

// Mascara de flags de canal
typedef enum ChannelFlags {
	FLAG_ANONYMOUS	= (1<<0),  // a - toggle the anonymous channel flag;
	FLAG_INVONLY  	= (1<<1),  // i - toggle the invite-only channel flag;
	FLAG_MODERATED	= (1<<2),  // m - toggle the moderated channel;
	FLAG_NOMSGS   	= (1<<3),  // n - toggle the no messages to channel from clients on the outside;
	FLAG_QUIET    	= (1<<4),  // q - toggle the quiet channel flag;
	FLAG_PRIVATE  	= (1<<5),  // p - toggle the private channel flag;
	FLAG_SECRET   	= (1<<6),  // s - toggle the secret channel flag;
	FLAG_REOP     	= (1<<7),  // r - toggle the server reop channel flag;
	FLAG_TOPIC    	= (1<<8),  // t - toggle the topic settable by channel operator only flag;
	FLAG_CHANKEY  	= (1<<9),  // k - set/remove the channel key (password);
	FLAG_USERLIMIT	= (1<<10), // l - set/remove the user limit to channel;
} ChannelFlags;

// Estructura para la lista enlazada de usuarios interna al canal
typedef struct UserChannelData {
	int                    	inChannel;	/* Esta realmente en el canal?	*/
	User*                  	usr;      	/* La estructura usuario      	*/
	UserFlags              	flags;    	/* Flags asociados            	*/
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
//Apart from the
//requirement that the first character is either '&', '#', '+' or '!',
//the only restriction on a channel name is that it SHALL NOT contain
//any spaces (' '), a control G (^G or ASCII 7), a comma (',').  Space
//is used as parameter separator and command is used as a list item
//separator by the protocol).  A colon (':') can also be used as a
//delimiter for the channel mask.  Channel names are case insensitive.
Channel* channel_new(Server* server, const char* name) {
	Channel* chan;
	if (server == NULL || name == NULL) return NULL;

	chan = ecalloc(1, sizeof *chan);
	strncpy(chan->name, name, CHANNEL_MAX_NAME_LEN);
	chan->server = server;
	return chan;
}

// Destruccion de un canal. Libera la lista interna de usuarios.
void channel_delete(Channel* chan) {
	if (chan == NULL) return;
	UserChannelData* usrData = chan->usrs;
	while (usrData != NULL) {
		UserChannelData* usrNext = usrData->next;
		free(usrData);
		usrData = usrNext;
	}
	free(chan);
}

// Añade un usuario a la lista interna de usuarios.
static int channelP_add_user(Channel* chan, User* usr, UserChannelData** ucdDst) {
	UserChannelData* ucd = emalloc(sizeof *ucd);
	ucd->usr       = usr;
	ucd->flags     = 0;
	ucd->inChannel = 0;
	ucd->next      = chan->usrs;
	chan->usrs = ucd;

	*ucdDst = ucd;
	return OK;
}

// Busca y borra un usuario de la lista interna de usuarios.
/*
static int channelP_remove_user(Channel* chan, User* usr) {
	UserChannelData** ucd = &chan->usrs;
	while (*ucd != NULL) {
		UserChannelData* usrNext = *ucd;
		if (usrNext->usr == usr) {
			*ucd = usrNext->next;
			return OK;
		}
		ucd = &usrNext->next;
	}
	return ERR;
}
*/

// Busca y devuelve un usuario de la lista interna de usuarios.
static int channelP_find_user_data(Channel* chan, User* usr, UserChannelData** ucd) {
	UserChannelData* cud = chan->usrs;
	while (cud != NULL) {
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
static int channelP_user_op_or_null(Channel* chan, User* usr) {
	UserChannelData* ucd;
	if (usr == NULL) return 1;
	if (!channelP_find_user_data(chan, usr, &ucd)) return 0;
	return (USRFLAG_OPERATOR & ucd->flags) != 0;
}

// Comprueba si un usuario tiene permiso para mandar un mensaje.
int channel_can_send_message(Channel* chan, User* usr) {
	UserChannelData* ucd;

	// Buscamos al que envia
	if (!channelP_find_user_data(chan, usr, &ucd)) {
		// Si no esta en la sala y se pueden enviar mensajes desde fuera
		if (FLAG_NOMSGS & chan->flags) return ERR_CANNOTSENDTOCHAN;

		// Solo si no esta moderado
		return (FLAG_MODERATED & chan->flags) ? OK : ERR_CANNOTSENDTOCHAN;
	}

	// Esta baneado?
	if (USRFLAG_BAN & ucd->flags) {
		// Si no es una excepcion
		if (!(USRFLAG_EXCEPTION & ucd->flags)) return ERR_CANNOTSENDTOCHAN;
	}

	// Si esta moderado, debe tener voz o ser operador
	if (FLAG_MODERATED & chan->flags) {
		if (!((USRFLAG_OPERATOR | USRFLAG_VOICE) & ucd->flags)) return ERR_CANNOTSENDTOCHAN;
	}

	return OK; // Podemos mandar mensajes!!
}

// Manda un comando a todos los usuarios del canal.
int channel_send_cmd(Channel* chan, const char* str) {
	UserChannelData* ucd;
	if (chan == NULL) return ERR;
	ucd = chan->usrs;
	while (ucd != NULL) {
		user_send_cmd(ucd->usr, str);
		ucd = ucd->next;
	}
	return OK;
}

// Manda un comando (con formateo) a todos los usuarios del canal.
int channel_send_cmdf(Channel* chan, const char* fmt, ...) {
	if (chan == NULL) return ERR;
	char buffer[512];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buffer, sizeof buffer, fmt, ap);
	int ret = channel_send_cmd(chan, buffer);
	va_end(ap);
	return ret;
}

// Cambia de una letra de flag de usuario a su mascara correspondiente.
static UserFlags channelP_char_to_usrflag(char flag) {
	switch (flag) {
		case 'O': return USRFLAG_CREATOR;
		case 'o': return USRFLAG_OPERATOR;
		case 'v': return USRFLAG_VOICE;
		case 'b': return USRFLAG_BAN;
		case 'e': return USRFLAG_EXCEPTION;
		case 'I': return USRFLAG_INVITATION;
		default: return 0;
	}
}

// Comprueba si un usuario tiene una flag.
int channel_has_flag_user(Channel* chan, User* usr, char flag) {
	UserChannelData* ucd;
	UserFlags flag_mask;
	if (chan == NULL || usr == NULL) return ERR_NEEDMOREPARAMS;

	// Convertimos la flag a una mascara
	flag_mask = channelP_char_to_usrflag(flag);
	if (flag_mask == 0) return ERR_UMODEUNKNOWNFLAG;

	// Buscamos al objetivo
	if (!channelP_find_user_data(chan, usr, &ucd)) return ERR_USERNOTINCHANNEL;

	return (flag_mask & ucd->flags) != 0;
}

// Pone una flag a un usuario.
int channel_set_flag_user(Channel* chan, User* usr, char flag, User* actor) {
	UserChannelData* ucd;
	UserFlags flag_mask;
	if (chan == NULL || usr == NULL) return ERR_NEEDMOREPARAMS;

	// Convertimos la flag a una mascara
	flag_mask = channelP_char_to_usrflag(flag);
	if (flag_mask == 0) return ERR_UMODEUNKNOWNFLAG;

	// Hay permisos?
	if (!channelP_user_op_or_null(chan, actor)) return ERR_CHANOPRIVSNEEDED;

	// Buscamos al objetivo
	if (!channelP_find_user_data(chan, usr, &ucd)) return ERR_USERNOTINCHANNEL;

	// Exito!
	ucd->flags |= flag_mask;
	return OK;
}

// Quita una flag de un usuario.
int channel_unset_flag_user(Channel* chan, User* usr, char flag, User* actor) {
	UserChannelData* ucd;
	UserFlags flag_mask;
	if (chan == NULL || usr == NULL) return ERR_NEEDMOREPARAMS;

	// Convertimos la flag a una mascara
	flag_mask = channelP_char_to_usrflag(flag);
	if (flag_mask == 0) return ERR_UMODEUNKNOWNFLAG;

	// Hay permisos?
	if (!channelP_user_op_or_null(chan, actor)) return ERR_CHANOPRIVSNEEDED;

	// Buscamos al objetivo
	if (!channelP_find_user_data(chan, usr, &ucd)) return ERR_USERNOTINCHANNEL;

	// Exito!
	ucd->flags &= ~flag_mask;
	return OK;
}

// Ingresa un usuario en el canal, con la clave proporcionada.
int channel_join(Channel* chan, User* usr, const char* key) {
	UserChannelData* ucd;
	if (chan == NULL || usr == NULL) return ERR_NEEDMOREPARAMS;

	// Esta el canal lleno?
	if (chan->usr_cnt == chan->usr_max) return ERR_CHANNELISFULL;

	// Buscamos o creamos al usuario
	channelP_find_or_create(chan, usr, &ucd);

	// Ya esta en el canal?
	if (ucd->inChannel) return OK;

	// Esta baneado el usuario?
	if (USRFLAG_BAN & ucd->flags) return ERR_BANNEDFROMCHAN;

	// Hay clave? Se ha proporcionado la clave correcta?
	if (FLAG_CHANKEY & chan->flags) {
		if (key == NULL) return ERR_BADCHANNELKEY;
		if (strncmp(key, chan->key, CHANNEL_MAX_KEY_LEN)) return ERR_BADCHANNELKEY;
	}

	// Es de solo invitacion?
	if (FLAG_INVONLY & chan->flags) {
		if ((USRFLAG_INVITATION & ucd->flags) == 0) return ERR_INVITEONLYCHAN;
	}

	// Si hemos llegado hasta aqui, le apuntamos
	channelP_find_or_create(chan, usr, &ucd);
	ucd->inChannel = 1;
	return OK;
}

// Abandona un canal. Si actor es distinto de NULL, es una expulsion forzosa.
int channel_part(Channel* chan, User* usr, User* actor) {
	UserChannelData* ucd;

	if (!channelP_find_user_data(chan, usr, &ucd)) {
		return ERR_NOTONCHANNEL;
	}

	// Solo los operadores pueden
	if (!channelP_user_op_or_null(chan, actor)) return ERR_CHANOPRIVSNEEDED;

	ucd->inChannel = 0;
	return OK;
}

// Devuelve el topic.
int channel_get_topic(Channel* chan, char** topic) {
	if (chan == NULL) return ERR;
	*topic = chan->topic;
	return OK;
}

// Cambia el topic del canal.
int channel_set_topic(Channel* chan, const char* topic, User* actor) {
	if (chan == NULL) return ERR;

	// Solo los operadores pueden cambiar el topic?
	if (FLAG_TOPIC & chan->flags) {
		// Hay permisos suficientes?
		if (!channelP_user_op_or_null(chan, actor)) return ERR;
	}

	strncpy(chan->topic, topic, CHANNEL_MAX_TOPIC_LEN);
	return OK;
}

// Devuelve el nombre del canal.
int channel_get_name(Channel* chan, char** name) {
	if (chan == NULL) return ERR;
	*name = chan->name;
	return OK;
}

// Devuelve la contraseña del canal.
int channel_get_key(Channel* chan, char** key) {
	if (chan == NULL) return ERR;
	*key = chan->key;
	return OK;
}

// Cambia la contraseña del canal.
int channel_set_key(Channel* chan, const char* key, User* actor) {
	if (chan == NULL) return ERR;

	// Hay permisos suficientes?
	if (!channelP_user_op_or_null(chan, actor)) return ERR_CHANOPRIVSNEEDED;

	strncpy(chan->key, key, CHANNEL_MAX_KEY_LEN);
	return OK;
}

// Cambia de una letra de flag de canal a su mascara correspondiente.
static ChannelFlags channelP_char_to_channelflag(char flag) {
	switch (flag) {
		case 'a': return FLAG_ANONYMOUS;
		case 'i': return FLAG_INVONLY;
		case 'm': return FLAG_MODERATED;
		case 'n': return FLAG_NOMSGS;
		case 'q': return FLAG_QUIET;
		case 'p': return FLAG_PRIVATE;
		case 's': return FLAG_SECRET;
		case 'r': return FLAG_REOP;
		case 't': return FLAG_TOPIC;
		case 'k': return FLAG_CHANKEY;
		case 'l': return FLAG_USERLIMIT;
		default:  return 0;
	}
}

//
int channel_has_flag(Channel* chan, char flag) {
	ChannelFlags flag_mask;
	if (chan == NULL) return 0;

	// Convertimos la flag a una mascara
	flag_mask = channelP_char_to_channelflag(flag);
	if (flag_mask == 0) return ERR_UMODEUNKNOWNFLAG;

	return (flag_mask & chan->flags) != 0;
}

// Pone una flag al canal.
int channel_set_flag(Channel* chan, char flag, User* actor) {
	ChannelFlags flag_mask;
	if (chan == NULL) return ERR;

	// Convertimos la flag a una mascara
	flag_mask = channelP_char_to_channelflag(flag);
	if (flag_mask == 0) return ERR_UMODEUNKNOWNFLAG;

	// Hay permisos suficientes?
	if (!channelP_user_op_or_null(chan, actor)) return ERR_CHANOPRIVSNEEDED;

	chan->flags |= flag_mask;
	return OK;
}

// Quita una flag del canal.
int channel_unset_flags(Channel* chan, char flag, User* actor) {
	ChannelFlags flag_mask;
	if (chan == NULL) return ERR;

	// Convertimos la flag a una mascara
	flag_mask = channelP_char_to_channelflag(flag);
	if (flag_mask == 0) return ERR_UMODEUNKNOWNFLAG;

	// Hay permisos suficientes?
	if (!channelP_user_op_or_null(chan, actor)) return ERR_CHANOPRIVSNEEDED;

	chan->flags &= ~flag_mask;
	return OK;
}

// ============================================================================
//   Funciones de listas
// ============================================================================

// Macros
#define channellist_head(list)	(*(list))
#define channellist_tail(list)	(&channellist_head(list)->next)

// Inserta un elemento en la lista.
int channellist_insert(ChannelList list, Channel* chan) {
	if (list == NULL || chan == NULL) return ERR;

	// Comprobamos que NO este ya en una lista
	if (chan->next != NULL) return ERR;

	chan->next = channellist_head(list);
	*list = chan;
	return OK;
}

// Extrae el primer elemento de una lista.
Channel* channellist_extract(ChannelList list) {
	Channel* chan;
	if (list == NULL) return NULL;

	chan = channellist_head(list);
	*list = chan->next;
	chan->next = NULL;
	return chan;
}

// Busca un elemento por su nombre.
ChannelList channellist_findByName(ChannelList list, const char* name) {
	if (list == NULL || name == NULL) return NULL;

	while (1) {
		Channel* chan = channellist_head(list);
		if (chan == NULL) break;
		if (strncmp(name, chan->name, CHANNEL_MAX_NAME_LEN)) break;
		list = channellist_tail(list);
	}

	return list;
}

// Libera todos los elementos de la lista.
void channellist_deleteAll(ChannelList list) {
	Channel* chan;
	if (list == NULL) return;

	chan = channellist_head(list);
	while (chan != NULL) {
		list = channellist_tail(list);
		free(chan);
		chan = channellist_head(list);
	}
}

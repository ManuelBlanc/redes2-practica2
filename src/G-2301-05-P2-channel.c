
#include "G-2301-05-P2-channel.h"

typedef struct UserChannelData {
	int                    	inChannel;	/* Esta realmente en el canal?	*/
	User*                  	usr;      	/* La estructura usuario      	*/
	UserFlags              	flags;    	/* Flags asociados            	*/
	struct UserChannelData*	next;
} UserChannelData;

typedef struct Channel {
	unsigned int    	usr_max;                   	/* Maximo numero de usuarios 	*/
	unsigned int    	usr_cnt;                   	/* Numero de usuarios        	*/
	UserChannelData*	usrs;                      	/* Lista de usuarios         	*/
	char            	name[IRC_MAX_NAME_LEN+1];  	/* Nombre                    	*/
	char            	topic[IRC_MAX_TOPIC_LEN+1];	/* Tema                      	*/
	char            	passwd[IRC_MAX_PSW_LEN+1]; 	/* Contraseña                	*/
	unsigned int    	flags;                     	/* Flags de un canal         	*/
	Server*         	server;                    	/* Servidor al que pertenece 	*/
	struct Channel* 	next;                      	/* Puntero al siguiente canal	*/
} Channel;

// Reserva de memoria. Inicializa el nombre.
Channel* channel_new(const char* name) {
	Channel* chan = calloc(1, sizeof Channel);
	strncpy(chan->)
	return chan;
}

// Destruccion de un canal. Libera la lista enlazada de UserChannelData.
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

// Añade un usuario a la lista de usuarios.
static int channelP_add_user(Channel* chan, User* usr, UserChannelData** ucd) {
	UserChannelData* usrData = malloc(sizeof UserChannelData);
	usrData->usr       = usr;
	usrData->flags     = 0;
	usrData->inChannel = 0
	usrData->next      = chan->usrs;
	chan->usrs = usrData;

	*ucd = usrData;
	return OK;
}

// Busca y extrae un usuario de la lista de usuarios
static int channelP_remove_user(Channel* chan, User* usr) {
	UserChannelData** usrData = &chan->usrs;
	while (*usrData != NULL) {
		UserChannelData* usrNext = *usrData;
		if (next->usr == usr) {
			*usrData = usrNext->next;
			return OK;
		}
		*usrData = &usrNext->next;
	}
	return ERR;
}

//
static int channelP_find_user_data(Channel* chan, User* usr, UserChannelData** ucd) {
	UserChannelData* usrData = chan->usrs;
	while (usrData != NULL) {
		if (usrData->usr == usr) {
			*ucd = usrData;
			return 1;
		}
		usrData = usrData->next;
	}
	return 0;
}

//
static int channelP_find_or_create(Channel* chan, User* usr, UserChannelData** ucd) {
	UserChannelData* usrData;
	if (!channelP_find_user_data(chan, usr, &ucd)) {
		channelP_add_user(chan, usr, &ucd);
	}
	return ucd;
}

//
static int channelP_user_op_or_null(Channel* chan, User* usr) {
	UserChannelData* ucd;
	if (usr == NULL) return 1;
	if (!channelP_find_user_data(chan, actor, &ucd)) return 0;
	return (USRFLAG_OPERATOR & ucd->flags) != 0;
}

//
static int channelP_can_send_message(Channel* chan, User* usr) {
	UserChannelData* ucd;

	// Buscamos al que envia
	if (!channelP_find_user_data(chan, usr, &ucd)) {
		// Si no esta en la sala y se pueden enviar mensajes desde fuera
		if (FLAG_NOMSGS & chan->flags) return 0; // No hay permiso
	}

	// Esta baneado?
	if (USRFLAG_BAN & ucd->flags) {
		// Si no es una excepcion
		if (!(USRFLAG_EXCEPTION & ucd->flags)) return 0; // No hay permiso
	}

	// Si esta moderado, debe tener voz o ser operador
	if (FLAG_MODERATED & chan->flags) {
		if (!((USRFLAG_OPERATOR | USRFLAG_VOICE) & ucd->flags)) return 0; // No hay permiso
	}

	return 1; // Podemos mandar mensajes!!
}

//
int channel_get_flags_user(Channel* chan, User* usr, long* flags) {
	UserChannelData* ucd;
	if (!channelP_find_user_data(chan, usr, &ucd)) return ERR;
	*flags = ucd->flags;
	return OK;
}

//
int channel_set_flags_user(Channel* chan, User* usr, long flags, User* actor) {
	UserChannelData* ucd;

	// Hay permisos?
	if (!channelP_user_op_or_null(chan, actor)) return ERR;

	// Buscamos al objetivo
	if (!channelP_find_user_data(chan, usr, &ucd)) return ERR;

	// Exito!
	ucd->flags |= flags;
	return OK;
}

//
int channel_unset_flags_user(Channel* chan, User* usr, long flags, User* actor) {
	UserChannelData* ucd;

	// Hay permisos?
	if (!channelP_user_op_or_null(chan, actor)) return ERR;

	// Buscamos al objetivo
	if (!channelP_find_user_data(chan, usr, &ucd)) return ERR;

	// Exito!
	ucd->flags &= ~flags;
	return OK;
}

//
int channel_send_message(Channel* chan, User* usr, const char* msg) {
	UserChannelData* ucd;

	if (!channelP_can_send_message(chan, usr)) return ERR;

	usrData = chan->usrs;
	while (usrData != NULL) {
		user_send_message(usrData->usr, chan->name, msg);
		usrData = usrData->next;
	}
	return OK;
}

//
int channel_join(Channel* chan, User* usr) {

}
int channel_part(Channel* chan, User* usr, User* actor) {

}

//
int channel_get_topic(Channel* chan, const char** topic) {
	if (chan == NULL) return ERR;
	*topic = chan->topic;
	return OK;
}

int channel_set_topic(Channel* chan, const char* topic, User* usr) {
	if (chan == NULL) return ERR;

	// Solo los operadores pueden cambiar el topic?
	if (FLAG_TOPIC & chan->flags) {
		// Comprobamos si hay permiso
		if (!channelP_user_op_or_null(chan, actor)) return ERR;
	}

	strncpy(chan->topic, topic);
	return OK;
}

//
int channel_get_name(Channel* chan, const char** name) {
	if (chan == NULL) return NULL;
	*name = chan->name;
	return OK;
}

//
int channel_set_name(Channel* chan, const char* name) {
	if (chan == NULL) return ERR;
	strncpy(chan->name, name, sizeof chan->name);
	return OK;
}

//
int channel_get_passwd(Channel* chan, const char** passwd) {
	if (chan == NULL) return ERR;
	*passwd = chan->passwd;
	return OK;
}

//
int channel_set_passwd(Channel* chan, const char* passwd, User* actor) {
	if (chan == NULL) return ERR;

	// Solo los operadores pueden
	if (!channelP_user_op_or_null(chan, actor)) return ERR;

	*passwd = chan->passwd;
	return OK;
}

//
int channel_get_flags(Channel* chan, long* flags) {
	if (chan == NULL) return ERR;
	*flags = chan->flags;
	return OK;
}

//
int channel_set_flags(Channel* chan, long flags, User* actor) {
	if (chan == NULL) return ERR;

	// Comprobamos si hay permiso
	if (!channelP_user_op_or_null(chan, actor)) return ERR;

	chan->flags |= flags;
	return OK;
}

//
int channel_unset_flags(Channel* chan, long flags, User* actor) {
	if (chan == NULL) return ERR;

	// Comprobamos si hay permiso
	if (!channelP_user_op_or_null(chan, actor)) return ERR;

	chan->flags &= ~flags;
	return OK;
}



//
int channellist_insert(ChannelList list, Channel* chan) {
	if (list == NULL || chan == NULL) return ERR;

	// Chan no esta en una lista?
	if (chan->next != NULL) return ERR;
}

//
ChannelList channellist_select(ChannelList list, int index) {
	if (list == NULL) return NULL;
}

//
Channel channellist_extract(ChannelList list) {
	if (list == NULL) return NULL;
}

//
ChannelList channellist_findByName(ChannelList list, const char* name) {
	if (list == NULL) return NULL;
}

//
void channellist_deleteAll(ChannelList list) {
	if (list == NULL) return;
}


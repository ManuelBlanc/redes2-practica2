#ifndef CHANNEL_H
#define CHANNEL_H

#include "G-2301-05-P2-user.h"

#define IRC_MAX_NAME_LEN 	(9) // Decision arbitraria
#define IRC_MAX_TOPIC_LEN	(50) // Decision arbitraria
#define IRC_MAX_PSW_LEN  	(50) // Decision arbitraria

enum UserFlags {
	USRFLAG_CREATOR   	= (1<<0), // O - give "channel creator" status;
	USRFLAG_OPERATOR  	= (1<<1), // o - give/take channel operator privilege;
	USRFLAG_VOICE     	= (1<<2), // v - give/take the voice privilege;
	USRFLAG_BAN       	= (1<<3), // b - set/remove ban mask to keep users out;
	USRFLAG_EXCEPTION 	= (1<<4), // e - set/remove an exception mask to override a ban mask;
	USRFLAG_INVITATION	= (1<<5), // I - set/remove an invitation mask to automatically override the invite-only flag;
};

enum ChannelFlags {
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
};

typedef struct Channel Channel;

Channel* channel_new(void);
void channel_delete(Channel* chan);

int channel_add_user(Channel* chan, User* usr);
int channel_remove_user(Channel* chan, User* usr);
int channel_get_flags_user(Channel* chan, User* usr, UserFlags* flags);
int channel_set_flags_user(Channel* chan, User* usr, UserFlags flags, User* actor);
int channel_unset_flags_user(Channel* chan, User* usr, UserFlags flags, User* actor);

int channel_send_message(Channel* chan, User* usr, const char* msg);

int channel_join(Channel* chan, User* usr);
int channel_part(Channel* chan, User* usr, User* actor);

int channel_get_topic(Channel* chan, const char** topic);
int channel_set_topic(Channel* chan, const char*  topic, User* actor);

int channel_get_name(Channel* chan, const char** name);
int channel_set_name(Channel* chan, const char*  name);

int channel_get_passwd(Channel* chan, const char** passwd);
int channel_set_passwd(Channel* chan, const char*  passwd);

int channel_get_flags(Channel* chan, ChannelFlags* flags);
int channel_set_flags(Channel* chan, ChannelFlags flags, User* actor);
int channel_unset_flags(Channel* chan, ChannelFlags flags, User* actor);

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

int        	channellist_insert(ChannelList list, Channel chan);
ChannelList	channellist_select(ChannelList list, int index);
Channel    	channellist_extract(ChannelList list);
ChannelList	channellist_findByName(ChannelList list, const char* name);
void       	channellist_deleteAll(ChannelList list);

#endif /* CHANNEL_H */

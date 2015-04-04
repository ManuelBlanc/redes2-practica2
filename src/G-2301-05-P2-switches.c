/* redes2 */
#include <redes2/irc.h>

/* usr */
#include "G-2301-05-P2-config.h"
#include "G-2301-05-P2-server.h"
#include "G-2301-05-P2-user.h"
#include "G-2301-05-P2-channel.h"

int action_switch(User* usr, char* str) {
	switch (IRC_CommandQuery(str)) {
		default: break;
		case ADMIN   	: printf("%s\n", "Comando ADMIN   recibido!"); break;
		case AWAY    	: printf("%s\n", "Comando AWAY    recibido!"); break;
		case CNOTICE 	: printf("%s\n", "Comando CNOTICE recibido!"); break;
		case CONNECT 	: printf("%s\n", "Comando CONNECT recibido!"); break;
		case CPRIVMSG	: printf("%s\n", "Comando CPRIVMS recibido!"); break;
		case DIE     	: printf("%s\n", "Comando DIE     recibido!"); break;
		case ENCAP   	: printf("%s\n", "Comando ENCAP   recibido!"); break;
		case ERROR   	: printf("%s\n", "Comando ERROR   recibido!"); break;
		case HELP    	: printf("%s\n", "Comando HELP    recibido!"); break;
		case INFO    	: printf("%s\n", "Comando INFO    recibido!"); break;
		case INVITE  	: printf("%s\n", "Comando INVITE  recibido!"); break;
		case ISON    	: printf("%s\n", "Comando ISON    recibido!"); break;
		case JOIN    	: printf("%s\n", "Comando JOIN    recibido!"); break;
		case KICK    	: printf("%s\n", "Comando KICK    recibido!"); break;
		case KILL    	: printf("%s\n", "Comando KILL    recibido!"); break;
		case KNOCK   	: printf("%s\n", "Comando KNOCK   recibido!"); break;
		case LINKS   	: printf("%s\n", "Comando LINKS   recibido!"); break;
		case LIST    	: printf("%s\n", "Comando LIST    recibido!"); break;
		case LUSERS  	: printf("%s\n", "Comando LUSERS  recibido!"); break;
		case MODE    	: printf("%s\n", "Comando MODE    recibido!"); break;
		case MOTD    	: printf("%s\n", "Comando MOTD    recibido!"); break;
		case NAMES   	: printf("%s\n", "Comando NAMES   recibido!"); break;
		case NAMESX  	: printf("%s\n", "Comando NAMESX  recibido!"); break;
		case NICK    	: printf("%s\n", "Comando NICK    recibido!"); break;
		case NOTICE  	: printf("%s\n", "Comando NOTICE  recibido!"); break;
		case OPER    	: printf("%s\n", "Comando OPER    recibido!"); break;
		case PART    	: printf("%s\n", "Comando PART    recibido!"); break;
		case PASS    	: printf("%s\n", "Comando PASS    recibido!"); break;
		case PING    	: printf("%s\n", "Comando PING    recibido!"); break;
		case PONG    	: printf("%s\n", "Comando PONG    recibido!"); break;
		case PRIVMSG 	: printf("%s\n", "Comando PRIVMSG recibido!"); break;
		case QUIT    	: printf("%s\n", "Comando QUIT    recibido!"); break;
		case REHASH  	: printf("%s\n", "Comando REHASH  recibido!"); break;
		case RESTART 	: printf("%s\n", "Comando RESTART recibido!"); break;
		case RULES   	: printf("%s\n", "Comando RULES   recibido!"); break;
		case SERVER  	: printf("%s\n", "Comando SERVER  recibido!"); break;
		case SERVICE 	: printf("%s\n", "Comando SERVICE recibido!"); break;
		case SERVLIST	: printf("%s\n", "Comando SERVLIS recibido!"); break;
		case SETNAME 	: printf("%s\n", "Comando SETNAME recibido!"); break;
		case SILENCE 	: printf("%s\n", "Comando SILENCE recibido!"); break;
		case SQUERY  	: printf("%s\n", "Comando SQUERY  recibido!"); break;
		case SQUIT   	: printf("%s\n", "Comando SQUIT   recibido!"); break;
		case STATS   	: printf("%s\n", "Comando STATS   recibido!"); break;
		case SUMMON  	: printf("%s\n", "Comando SUMMON  recibido!"); break;
		case TIME    	: printf("%s\n", "Comando TIME    recibido!"); break;
		case TOPIC   	: printf("%s\n", "Comando TOPIC   recibido!"); break;
		case TRACE   	: printf("%s\n", "Comando TRACE   recibido!"); break;
		case UHNAMES 	: printf("%s\n", "Comando UHNAMES recibido!"); break;
		case USER    	: printf("%s\n", "Comando USER    recibido!"); break;
		case USERHOST	: printf("%s\n", "Comando USERHOS recibido!"); break;
		case USERIP  	: printf("%s\n", "Comando USERIP  recibido!"); break;
		case USERS   	: printf("%s\n", "Comando USERS   recibido!"); break;
		case VERSION 	: printf("%s\n", "Comando VERSION recibido!"); break;
		case WALLOPS 	: printf("%s\n", "Comando WALLOPS recibido!"); break;
		case WATCH   	: printf("%s\n", "Comando WATCH   recibido!"); break;
		case WHO     	: printf("%s\n", "Comando WHO     recibido!"); break;
		case WHOIS   	: printf("%s\n", "Comando WHOIS   recibido!"); break;
		case WHOWAS  	: printf("%s\n", "Comando WHOWAS  recibido!"); break;
	}
}

static char* namechannel_skip_colon(char* channel) {
	return *channel == ':' ? channel+1 : channel;
}

int serverrcv_privmsg(Server* serv, User* usr, char* str) {
	char* nick;
	char* prefix;
	char* target;
	char* msg;
	char buf[512];
	long opt;

	user_get_nick(usr, &nick);
	if(0 != IRCParse_Privmsg(str, &prefix, &target, &msg)){
		IRC_ErrUnKnownCommand(buf, prefix, nick, str);
		user_send_cmd(usr, buf);
		return ERR;
	}

    // Es un canal?
	namechannel_skip_colon(target);
	if (strchr("#!&+", target[0])) {
		// Lo buscamos en los canales
		ChannelList chan = channellist_findByName(&serv->chan, target);
		//Envia el mensaje o devuelve un codigo de error
		if (NULL != chan) opt = channel_send_message(*chan, usr, msg);
		else opt = ERR_CANNOTSENDTOCHAN;
		switch(opt) {
			default: break;
			case ERR_NOTEXTTOSEND:
				IRC_ErrNoTextToSend(buf, prefix, target);
				user_send_cmd(usr, buf);
				break;
			case ERR_CANNOTSENDTOCHAN:
				IRC_ErrCanNotSendToChan(buf, prefix, nick, target);
				user_send_cmd(usr, buf);
				break;
			case ERR_NOSUCHNICK:
				IRC_ErrNoSuchNick(buf, prefix, nick, target);
				user_send_cmd(usr, buf);
				break;
		}
		//case ERR_TOOMANYTARGETS: no admitimos multiples destinatarios
		//ERR_NORECIPIENT	ERR_NOTOPLEVEL ERR_WILDTOPLEVEL
	} else {
		char* awaymsg;
		UserList recv = userlist_findByName(&serv->usrs, target);
		if (NULL != recv) opt = user_send_message(*recv, nick, msg);
		else opt = ERR_NOSUCHNICK;
		switch(opt) {
			default: break;
			case ERR_NOTEXTTOSEND:
				IRC_ErrNoTextToSend(buf, prefix, target);
				user_send_cmd(usr, buf);
				break;
			case ERR_NOSUCHNICK:
				IRC_ErrNoSuchNick(buf, prefix, nick, target);
				user_send_cmd(usr, buf);
				break;
			case RPL_AWAY:
				user_get_away(*recv, &awaymsg);
				IRC_RplAway(buf, prefix, nick, target, awaymsg);
				user_send_cmd(usr, buf);
				break;
		}
	}
	return OK;
}

int serverrcv_mode(Server* serv, User* usr, char* str) {
	/*char nick[USER_MAX_NICK_LEN + 1];
	char* channel_name;
	char* user_target;
	char* prefix;
	char* mode;
	char buf[512];
	long opt;

	user_get_nick(usr, &nick);
	if(0 != IRCParse_Mode(str, &prefix, &channel_name, &mode, &user_target)){
		IRC_ErrUnKnownCommand(buf, prefix, nick, str);
		user_send_cmd(usr, buf);
		return ERR;
	}

	namechannel_skip_colon(channel_name);
	Channel* chan = channellist_findByName(serv->chan, channel_name);
	if (NULL != user_target) {
		if(mode == '+') {
			opt = channel_set_flag_user(chan, user_target, mode, usr);
		} else if(mode == '-') {
			opt = channel_unset_flag_user(chan, user_target, mode, usr);
		} else {
			//si no hacemos devolver banlist, invitedlist o exceptionlist, esto BIEN
			opt = RPL_UNIQOPIS;
		}
		switch(opt) {
			default: break;
			case ERR_NEEDMOREPARAMS:
				IRC_ErrNeedMoreParams(buf, prefix, nick, str);
				user_send_cmd(usr, buf);
				break;
			case ERR_USERSDONTMATCH://!!!!!!!!!!!
				IRC_ErrUsersDontMatch(buf, prefix, nick);
				user_send_cmd(usr, buf);
				break;
			case ERR_UMODEUNKNOWNFLAG:
				IRC_ErrUModeUnknownFlag(buf, prefix, nick);
				user_send_cmd(usr, buf);
				break;
			case RPL_UMODEIS:
				IRC_RplUModeIs(buf, prefix, nick, mode);
				user_send_cmd(usr, buf);
				break;
			case RPL_UNIQOPIS:
				IRC_RplUniqOpIs(buf, prefix, nick, channel_name, char *nickname);
				//como sacar el creador del canal????
				user_send_cmd(usr, buf);
				break;
		}
	} else {
		User* recv = userlist_findByName(serv->usrs, user_target);
		if (NULL != recv) opt = user_send_message(recv, nick, msg);
		else opt = ERR_NOSUCHNICK;
		switch(opt) {
			default: break;
			case ERR_NOTEXTTOSEND:
				IRC_ErrNoTextToSend(buf, prefix, user_target);
				user_send_cmd(usr, buf);
				break;
			case ERR_NOSUCHNICK:
				IRC_ErrNoSuchNick(buf, prefix, nick, user_target);
				user_send_cmd(usr, buf);
				break;
			case RPL_AWAY:
				char* awaymsg = user_get_away(recv, &awaymsg);
				IRC_RplAway(buf, prefix, nick, user_target, awaymsg);
				user_send_cmd(usr, buf);
				break;
		}

		//ERR_NEEDMOREPARAMS              ERR_KEYSET
        	//ERR_NOCHANMODES                 ERR_CHANOPRIVSNEEDED
        	//ERR_USERNOTINCHANNEL            ERR_UNKNOWNMODE
        	//RPL_CHANNELMODEIS
        	//RPL_BANLIST                     RPL_ENDOFBANLIST
        	//RPL_EXCEPTLIST                  RPL_ENDOFEXCEPTLIST
        	//RPL_INVITELIST                  RPL_ENDOFINVITELIST
        	//RPL_UNIQOPIS

	}*/
	return OK;
}

int serverrcv_user(Server* serv, User* usr, char* str) {
	/*char nick[USER_MAX_NICK_LEN + 1];
	char* prefix;
	char* user_name;
	char* realname;
	char* mode;
	char buf[512];
	// The <mode> parameter should be a numeric, and can be used to
   	//automatically set user modes when registering with the server.  This
   	//parameter is a bitmask, with only 2 bits having any signification: if
   	//the bit 2 is set, the user mode 'w' will be set and if the bit 3 is
   	//set, the user mode 'i' will be set.
	if(0 != IRCParse_User(str, &prefix, &user_name, &mode, &realname)){
		IRC_ErrNeedMoreParams(buf, prefix, user_name, str);
		user_send_cmd(usr, buf);
		return ERR;
	}
	UserList usr_using = userlist_findByName(&serv->usrs, user_name);
	if(NULL == usr_using){
		setters...
		server_add_user(serv, usr);
		return OK;
	}
	IRC_ErrAlreadyRegistred();*/
	return ERR;
}

int serverrcv_quit(Server* serv, User* usr, char* str) {
/*sacar los semaforos fuera delparser.c*/
	/* down semaforo*/
	//userlist_extract(&usr);/*mal argumento*/
	/* up semaforo*/
	//channel_sendf(chan, "MODE %s +%c", user_get_name(usr), mode);
	return OK;
}

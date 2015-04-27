/** G-2301-05-P3-clientcmd.c
 * @author AAlonso <ana.alonsoa@estudiante.uam.es>
 * @author MBlanc <manuel.blanc@estudiante.uam.es>
 */

/* std */
#include <ctype.h>
/* redes2 */
#include <redes2/chat.h>
#include <redes2/irc.h>
/* usr */
#include "G-2301-05-P3-chat.h"
#include "G-2301-05-P3-client.h"
#include "G-2301-05-P3-clientfile.h"
#include "G-2301-05-P3-util.h"

static int clientrplP_switch(Client *cli, char* str);

int clientrpl(Client *cli, char* str)
{
	char* cmd;
	int more_commands = 1;

	gdk_threads_enter();
	LOG("Cadena recibida: %s", str);
	gdk_threads_leave();

	while (more_commands) {
		switch (IRC_UnPipelineCommands(str, &cmd)) {
		case IRC_ENDPIPE:
			cli->buf[0] = '\0';
			more_commands = 0;
		case IRC_OK:
			gdk_threads_enter();
			clientrplP_switch(cli, cmd);
			gdk_threads_leave();
			str = NULL;
			break;

		case IRC_EOP:
			memset(cli->buf, sizeof(cli->buf), 0);
			strncpy(cli->buf, cmd, sizeof(cli->buf));
			return OK;
		}
	}
	// Vaciamos el buffer
	return OK;
}

static void command_reply_handler(Client* cli, char* str) {
	char buffer[IRC_MAX_CMD_LEN+1];
	char* chr;
	strncpy(buffer, str, IRC_MAX_CMD_LEN);
	buffer[IRC_MAX_CMD_LEN] = 0;
	chr = strrchr(buffer, '\r'); if (chr) *chr = '\0';
	chr = strrchr(buffer, '\n'); if (chr) *chr = '\0';
	LOG("%s", str);
	message_text(current_page(), buffer);
}

static void command_error_handler(Client* cli, char* str) {
	LOG("%s", str);
	error_text(current_page(), str);
}

static char* nick_from_prefix(char* nick, char* prefix)
{
	size_t len_nick = strcspn(prefix, "!");
	strncpy(nick, prefix, len_nick);
	nick[len_nick] = '\0';
	return nick;
}

static char* namechannel_skip_colon(char* channel) {
	return *channel == ':' ? channel+1 : channel;
}

static int clientrpl_invite(Client* cli, char* str) {
	char* prefix;
	char* channel;
	char* nick;
	char* ch_clean;
	IRCParse_Invite(str, &prefix, &nick, &channel);
	ch_clean = namechannel_skip_colon(channel);
	message_textf(0, "Has sido invitado a %s", ch_clean);
	return OK;
}

static int clientrpl_ison(Client* cli, char* str) {
	char* prefix;
	char* nicklist;
	IRCParse_Ison(str, &prefix, &nicklist);
	message_text(0, nicklist);
	return OK;
}

/* Mensaje de confirmacion cuando nos unimos
 * a una sala. Creamos la pestaña correspondiente. */
static int clientrpl_join(Client* cli, char* str) {
	char nick[IRC_MAX_NICK_LEN + 1];
	char* prefix;
	char* channel;
	char* key;
	char* msg;
	char* ch_clean;
	IRCParse_Join(str, &prefix, &channel, &key, &msg);
	ch_clean = namechannel_skip_colon(channel);
	{
		char* cur = channel;
		do *cur = tolower(*cur); while (*cur++ != '\0');
	}
	if (get_index_page(ch_clean) == -1) {
		set_current_page(add_new_page(ch_clean));
	}
	message_textf(get_index_page(ch_clean), "%s se ha unido", nick_from_prefix(nick, prefix));
	free(prefix);
	free(channel);
	free(key);
	return OK;
}

static int clientrpl_kick(Client* cli, char* str) {
	char* prefix;
	char* channel;
	char* user;
	char* comment;
	char* ch_clean;
	IRCParse_Kick(str, &prefix, &channel, &user, &comment);
	ch_clean = namechannel_skip_colon(channel);
	if (strcmp(cli->nick, user) == 0) {
		delete_page(get_index_page(ch_clean));
		message_text(0, "Has sido expulsado");
	} else {
		message_textf(get_index_page(ch_clean), "%s ha sido expulsado del canal", user);
	}
	free(prefix);
	free(channel);
	free(user);
	free(comment);
	return OK;
}

static int clientrpl_mode(Client* cli, char* str) {
	char* prefix;
	char* channeluser;
	char* mode;
	char* user;
	char* ch_clean;
	int page;
	char nick[IRC_MAX_NICK_LEN + 1];
	IRCParse_Mode(str, &prefix, &channeluser, &mode, &user);
	nick_from_prefix(nick, prefix);
	ch_clean = namechannel_skip_colon(channeluser);
	page = get_index_page(ch_clean);
	message_textf(page, "%s elige modo %s %s", nick, mode, user == NULL ? ch_clean : user);

	// Cambia los checkboxes de la inerfaz
	if (*ch_clean == '#') {
		char onoff, flag;
		sscanf(mode, "%c%c", &onoff, &flag);
		onoff = onoff == '+' ? TRUE : FALSE;
		switch (flag) {
			case 't': set_topic_protect	(page, onoff); break;
			case 'n': set_extern_msg   	(page, onoff); break;
			case 's': set_secret       	(page, onoff); break;
			case 'i': set_guests       	(page, onoff); break;
			case 'p': set_privated     	(page, onoff); break;
			case 'm': set_moderated    	(page, onoff); break;
		}
	}

	free(prefix);
	free(channeluser);
	free(user);
	free(mode);
	return OK;
}


static int clientrpl_nick(Client* cli, char* str) {
	char* prefix;
	char* nick;
	char nick_aux[IRC_MAX_NICK_LEN + 1];
	IRCParse_Nick(str, &prefix, &nick);
	if (strcmp(nick_from_prefix(nick_aux, prefix), nick+1)) {
		message_textf(current_page(), "%s es ahora %s", nick_aux, nick+1);
	}
	if (client_lookfor_ignore(cli, nick_aux) == OK) {
	    client_remove_ignore(cli, nick_aux);
	    client_add_ignore(cli, nick+1);
	}
	free(prefix);
	free(nick);
	return OK;
}

static int clientrpl_part(Client* cli, char* str) {
	char nick[IRC_MAX_NICK_LEN + 1];
	char* prefix;
	char* channel;
	char* msg;
	char* ch_clean;
	IRCParse_Part(str, &prefix, &channel, &msg);
	nick_from_prefix(nick, prefix);
	ch_clean = namechannel_skip_colon(channel);
	if (strcmp(cli->nick, nick) == 0) {
		delete_page(get_index_page(ch_clean));
	} else {
		message_textf(get_index_page(ch_clean), "%s ha abandonado el canal", nick);
	}
	free(prefix);
	free(channel);
	free(msg);
	return OK;
}

/* Respondemos a los PINGs del servidor
 * para que no nos echen por inactividad. */
static int clientrpl_ping(Client* cli, char* str) {
	char* prefix;
	char* server;
	char* server2;
	IRCParse_Ping(str, &prefix, &server, &server2);
	client_send_pong(cli, server, server2);
	free(prefix);
	free(server);
	free(server2);
	return OK;
}
static int clientrpl_privmsg(Client* cli, char* str) {
	char nick[IRC_MAX_NICK_LEN + 1];
	char* prefix;
	char* target;
	char* msg;
	int isPrivate = 0;
	IRCParse_Privmsg(str, &prefix, &target, &msg);
	if (target == NULL) {
		free(prefix);
		free(msg);
		return OK;
	}

	if (client_lookfor_ignore(cli, nick_from_prefix(nick, prefix))) {
		// Esta ignorado
		return OK;

	}

	// Es un privado?
	if (strcmp(target, cli->nick) == 0) {
		target = nick_from_prefix(nick, prefix);
		isPrivate = 1;
	}


	int page = get_or_create_page(target);
	ASSERT(page >= 0, "Pagina inexistente");


	if (strncmp(msg, "\01ACTION", 7) == 0) {
		size_t len = strlen(msg);
		if (len-1 == '\01') msg[len-1] = '\0';
		message_textf(page, "* %s%s", nick_from_prefix(nick, prefix), msg+7);
	} else if (isPrivate && strncmp(msg, "DCC SEND", 8) == 0) {

		if (clientfile_is_active()) {
			error_text(page, "Ya estas en proceso de recibir un fichero. Envio cancelado");
		}
		else {
			if (clientfile_set_connection_params(msg) == OK) {
				message_textf(page, "Quiero enviarte el fichero \"%s\" de %lu bytes. Aceptar/cancelar con /faccept y /fcancel", "patata", 0xFUL);
			}
			else {
				error_text(page, "Recibido mensaje DCC mal formado.");
			}
		}
	} else {
		(isPrivate ? private_text : public_text)(page, nick_from_prefix(nick, prefix), msg);
	}
	free(prefix);
	if (!isPrivate) free(target);
	free(msg);
	return OK;
}

static int clientrpl_topic(Client* cli, char* str) {
	char* prefix;
	char* channel;
	char* topic;
	char* ch_clean;
	IRCParse_Topic(str, &prefix, &channel, &topic);
	ch_clean = namechannel_skip_colon(channel);
	message_textf(get_index_page(ch_clean), "## %s", topic);
	free(prefix);
	free(channel);
	free(topic);
	return OK;
}

static int clientrplP_switch(Client* cli, char* str)
{
// Macros para los manejadores
// Como son macros, pueden acceder a las locales de la funcion
#define COMMAND_QUERY_HANDLER(code)	case code: LOG("Ignorando " #code " recibido en el cliente"); break;
#define COMMAND_REPLY_HANDLER(code)	case code: command_reply_handler(cli, str); break;
#define COMMAND_ERROR_HANDLER(code)	case code: command_error_handler(cli, str); break;

	switch (IRC_CommandQuery(str)) {
	/* Casos especiales */
	default: return ERR;

	/* Peticiones que si respondemos */
	case INVITE 	: clientrpl_invite(cli, str); 	break;
	case ISON   	: clientrpl_ison(cli, str);   	break;
	case JOIN   	: clientrpl_join(cli, str);   	break;
	case KICK   	: clientrpl_kick(cli, str);   	break;
	case MODE   	: clientrpl_mode(cli, str);   	break;
	case NICK   	: clientrpl_nick(cli, str);   	break;
	case PART   	: clientrpl_part(cli, str);   	break;
	case PING   	: clientrpl_ping(cli, str);   	break;
	case PRIVMSG	: clientrpl_privmsg(cli, str);	break;
	case TOPIC  	: clientrpl_topic(cli, str);  	break;

  	/* Peticiones */
  	COMMAND_QUERY_HANDLER( ADMIN   	);
  	COMMAND_QUERY_HANDLER( AWAY    	);
  	COMMAND_QUERY_HANDLER( CNOTICE 	);
  	COMMAND_QUERY_HANDLER( CONNECT 	);
  	COMMAND_QUERY_HANDLER( CPRIVMSG	);
  	COMMAND_QUERY_HANDLER( DIE     	);
  	COMMAND_QUERY_HANDLER( ENCAP   	);
  	COMMAND_QUERY_HANDLER( ERROR   	);
  	COMMAND_QUERY_HANDLER( HELP    	);
  	COMMAND_QUERY_HANDLER( INFO    	);
//	COMMAND_QUERY_HANDLER( INVITE  	);
//	COMMAND_QUERY_HANDLER( ISON    	);
//	COMMAND_QUERY_HANDLER( JOIN    	);
//	COMMAND_QUERY_HANDLER( KICK    	);
  	COMMAND_QUERY_HANDLER( KILL    	);
  	COMMAND_QUERY_HANDLER( KNOCK   	);
  	COMMAND_QUERY_HANDLER( LINKS   	);
  	COMMAND_QUERY_HANDLER( LIST    	);
  	COMMAND_QUERY_HANDLER( LUSERS  	);
//	COMMAND_QUERY_HANDLER( MODE    	);
  	COMMAND_QUERY_HANDLER( MOTD    	);
  	COMMAND_QUERY_HANDLER( NAMES   	);
  	COMMAND_QUERY_HANDLER( NAMESX  	);
//	COMMAND_QUERY_HANDLER( NICK    	);
  	COMMAND_QUERY_HANDLER( NOTICE  	);
  	COMMAND_QUERY_HANDLER( OPER    	);
//	COMMAND_QUERY_HANDLER( PART    	);
  	COMMAND_QUERY_HANDLER( PASS    	);
//	COMMAND_QUERY_HANDLER( PING    	);
  	COMMAND_QUERY_HANDLER( PONG    	);
//	COMMAND_QUERY_HANDLER( PRIVMSG 	);
  	COMMAND_QUERY_HANDLER( QUIT    	);
  	COMMAND_QUERY_HANDLER( REHASH  	);
  	COMMAND_QUERY_HANDLER( RESTART 	);
  	COMMAND_QUERY_HANDLER( RULES   	);
  	COMMAND_QUERY_HANDLER( SERVER  	);
  	COMMAND_QUERY_HANDLER( SERVICE 	);
  	COMMAND_QUERY_HANDLER( SERVLIST	);
  	COMMAND_QUERY_HANDLER( SETNAME 	);
  	COMMAND_QUERY_HANDLER( SILENCE 	);
  	COMMAND_QUERY_HANDLER( SQUERY  	);
  	COMMAND_QUERY_HANDLER( SQUIT   	);
  	COMMAND_QUERY_HANDLER( STATS   	);
  	COMMAND_QUERY_HANDLER( SUMMON  	);
  	COMMAND_QUERY_HANDLER( TIME    	);
//	COMMAND_QUERY_HANDLER( TOPIC   	);
  	COMMAND_QUERY_HANDLER( TRACE   	);
  	COMMAND_QUERY_HANDLER( UHNAMES 	);
  	COMMAND_QUERY_HANDLER( USER    	);
  	COMMAND_QUERY_HANDLER( USERHOST	);
  	COMMAND_QUERY_HANDLER( USERIP  	);
  	COMMAND_QUERY_HANDLER( USERS   	);
  	COMMAND_QUERY_HANDLER( VERSION 	);
  	COMMAND_QUERY_HANDLER( WALLOPS 	);
  	COMMAND_QUERY_HANDLER( WATCH   	);
  	COMMAND_QUERY_HANDLER( WHO     	);
  	COMMAND_QUERY_HANDLER( WHOIS   	);
  	COMMAND_QUERY_HANDLER( WHOWAS  	);

	/* Respuestas */
	COMMAND_REPLY_HANDLER( RPL_ADMINEMAIL     	);
	COMMAND_REPLY_HANDLER( RPL_ADMINLOC1      	);
	COMMAND_REPLY_HANDLER( RPL_ADMINLOC2      	);
	COMMAND_REPLY_HANDLER( RPL_ADMINME        	);
	COMMAND_REPLY_HANDLER( RPL_AWAY           	);
	COMMAND_REPLY_HANDLER( RPL_BANLIST        	);
	COMMAND_REPLY_HANDLER( RPL_BOUNCE         	);
	COMMAND_REPLY_HANDLER( RPL_CHANNELMODEIS  	);
	COMMAND_REPLY_HANDLER( RPL_CHANNELURL     	);
	COMMAND_REPLY_HANDLER( RPL_CREATED        	);
	COMMAND_REPLY_HANDLER( RPL_CREATIONTIME   	);
	COMMAND_REPLY_HANDLER( RPL_ENDOFBANLIST   	);
	COMMAND_REPLY_HANDLER( RPL_ENDOFEXCEPTLIST	);
	COMMAND_REPLY_HANDLER( RPL_ENDOFINFO      	);
	COMMAND_REPLY_HANDLER( RPL_ENDOFINVITELIST	);
	COMMAND_REPLY_HANDLER( RPL_ENDOFLINKS     	);
	COMMAND_REPLY_HANDLER( RPL_ENDOFMOTD      	);
	COMMAND_REPLY_HANDLER( RPL_ENDOFNAMES     	);
	COMMAND_REPLY_HANDLER( RPL_ENDOFSTATS     	);
	COMMAND_REPLY_HANDLER( RPL_ENDOFUSERS     	);
	COMMAND_REPLY_HANDLER( RPL_ENDOFWHO       	);
	COMMAND_REPLY_HANDLER( RPL_ENDOFWHOIS     	);
	COMMAND_REPLY_HANDLER( RPL_ENDOFWHOWAS    	);
	COMMAND_REPLY_HANDLER( RPL_EXCEPTLIST     	);
	COMMAND_REPLY_HANDLER( RPL_GLOBALUSERS    	);
	COMMAND_REPLY_HANDLER( RPL_INFO           	);
	COMMAND_REPLY_HANDLER( RPL_INVITELIST     	);
	COMMAND_REPLY_HANDLER( RPL_INVITING       	);
	COMMAND_REPLY_HANDLER( RPL_ISON           	);
	COMMAND_REPLY_HANDLER( RPL_LINKS          	);
	COMMAND_REPLY_HANDLER( RPL_LIST           	);
	COMMAND_REPLY_HANDLER( RPL_LISTEND        	);
	COMMAND_REPLY_HANDLER( RPL_LISTSTART      	);
	COMMAND_REPLY_HANDLER( RPL_LOCALUSERS     	);
	COMMAND_REPLY_HANDLER( RPL_LUSERCHANNELS  	);
	COMMAND_REPLY_HANDLER( RPL_LUSERCLIENT    	);
	COMMAND_REPLY_HANDLER( RPL_LUSERME        	);
	COMMAND_REPLY_HANDLER( RPL_LUSEROP        	);
	COMMAND_REPLY_HANDLER( RPL_LUSERUNKNOWN   	);
	COMMAND_REPLY_HANDLER( RPL_MOTD           	);
	COMMAND_REPLY_HANDLER( RPL_MOTDSTART      	);
	COMMAND_REPLY_HANDLER( RPL_MYINFO         	);
	COMMAND_REPLY_HANDLER( RPL_NAMREPLY       	);
	COMMAND_REPLY_HANDLER( RPL_NOTOPIC        	);
	COMMAND_REPLY_HANDLER( RPL_NOUSERS        	);
	COMMAND_REPLY_HANDLER( RPL_NOWAWAY        	);
	COMMAND_REPLY_HANDLER( RPL_REHASHING      	);
	COMMAND_REPLY_HANDLER( RPL_SERVLIST       	);
	COMMAND_REPLY_HANDLER( RPL_SERVLISTEND    	);
	COMMAND_REPLY_HANDLER( RPL_STATSCOMMANDS  	);
	COMMAND_REPLY_HANDLER( RPL_STATSLINKINFO  	);
	COMMAND_REPLY_HANDLER( RPL_STATSOLINE     	);
	COMMAND_REPLY_HANDLER( RPL_STATSUPTIME    	);
	COMMAND_REPLY_HANDLER( RPL_SUMMONING      	);
	COMMAND_REPLY_HANDLER( RPL_TIME           	);
	COMMAND_REPLY_HANDLER( RPL_TOPIC          	);
	COMMAND_REPLY_HANDLER( RPL_TOPICWHOTIME   	);
	COMMAND_REPLY_HANDLER( RPL_TRACECLASS     	);
	COMMAND_REPLY_HANDLER( RPL_TRACECONNECT   	);
	COMMAND_REPLY_HANDLER( RPL_TRACECONNECTING	);
	COMMAND_REPLY_HANDLER( RPL_TRACEEND       	);
	COMMAND_REPLY_HANDLER( RPL_TRACEHANDSHAKE 	);
	COMMAND_REPLY_HANDLER( RPL_TRACELINK      	);
	COMMAND_REPLY_HANDLER( RPL_TRACELOG       	);
	COMMAND_REPLY_HANDLER( RPL_TRACENEWTYPE   	);
	COMMAND_REPLY_HANDLER( RPL_TRACEOPERATOR  	);
	COMMAND_REPLY_HANDLER( RPL_TRACESERVER    	);
	COMMAND_REPLY_HANDLER( RPL_TRACESERVICE   	);
	COMMAND_REPLY_HANDLER( RPL_TRACEUNKNOWN   	);
	COMMAND_REPLY_HANDLER( RPL_TRACEUSER      	);
	COMMAND_REPLY_HANDLER( RPL_TRYAGAIN       	);
	COMMAND_REPLY_HANDLER( RPL_UMODEIS        	);
	COMMAND_REPLY_HANDLER( RPL_UNAWAY         	);
	COMMAND_REPLY_HANDLER( RPL_UNIQOPIS       	);
	COMMAND_REPLY_HANDLER( RPL_USERHOST       	);
	COMMAND_REPLY_HANDLER( RPL_USERS          	);
	COMMAND_REPLY_HANDLER( RPL_USERSSTART     	);
	COMMAND_REPLY_HANDLER( RPL_VERSION        	);
	COMMAND_REPLY_HANDLER( RPL_WELCOME        	);
	COMMAND_REPLY_HANDLER( RPL_WHOISCHANNELS  	);
	COMMAND_REPLY_HANDLER( RPL_WHOISIDLE      	);
	COMMAND_REPLY_HANDLER( RPL_WHOISOPERATOR  	);
	COMMAND_REPLY_HANDLER( RPL_WHOISSERVER    	);
	COMMAND_REPLY_HANDLER( RPL_WHOISUSER      	);
	COMMAND_REPLY_HANDLER( RPL_WHOREPLY       	);
	COMMAND_REPLY_HANDLER( RPL_WHOWASUSER     	);
	COMMAND_REPLY_HANDLER( RPL_YOUREOPER      	);
	COMMAND_REPLY_HANDLER( RPL_YOURESERVICE   	);
	COMMAND_REPLY_HANDLER( RPL_YOURHOST       	);
	COMMAND_REPLY_HANDLER( RPL_YOURID         	);

	/* Errores */
	COMMAND_ERROR_HANDLER( ERR_ALREADYREGISTRED 	);
	COMMAND_ERROR_HANDLER( ERR_BADCHANMASK      	);
	COMMAND_ERROR_HANDLER( ERR_BADCHANNELKEY    	);
	COMMAND_ERROR_HANDLER( ERR_BADMASK          	);
	COMMAND_ERROR_HANDLER( ERR_BANLISTFULL      	);
	COMMAND_ERROR_HANDLER( ERR_BANNEDFROMCHAN   	);
	COMMAND_ERROR_HANDLER( ERR_CANNOTSENDTOCHAN 	);
	COMMAND_ERROR_HANDLER( ERR_CANTKILLSERVER   	);
	COMMAND_ERROR_HANDLER( ERR_CHANNELISFULL    	);
	COMMAND_ERROR_HANDLER( ERR_CHANOPRIVSNEEDED 	);
	COMMAND_ERROR_HANDLER( ERR_ERRONEUSNICKNAME 	);
	COMMAND_ERROR_HANDLER( ERR_FILEERROR        	);
	COMMAND_ERROR_HANDLER( ERR_INVITEONLYCHAN   	);
	COMMAND_ERROR_HANDLER( ERR_KEYSET           	);
	COMMAND_ERROR_HANDLER( ERR_NEEDMOREPARAMS   	);
	COMMAND_ERROR_HANDLER( ERR_NICKCOLLISION    	);
	COMMAND_ERROR_HANDLER( ERR_NICKNAMEINUSE    	);
	COMMAND_ERROR_HANDLER( ERR_NOADMININFO      	);
	COMMAND_ERROR_HANDLER( ERR_NOCHANMODES      	);
	COMMAND_ERROR_HANDLER( ERR_NOLOGIN          	);
	COMMAND_ERROR_HANDLER( ERR_NOMOTD           	);
	COMMAND_ERROR_HANDLER( ERR_NONICKNAMEGIVEN  	);
	COMMAND_ERROR_HANDLER( ERR_NOOPERHOST       	);
	COMMAND_ERROR_HANDLER( ERR_NOORIGIN         	);
	COMMAND_ERROR_HANDLER( ERR_NOPERMFORHOST    	);
	COMMAND_ERROR_HANDLER( ERR_NOPRIVILEGES     	);
	COMMAND_ERROR_HANDLER( ERR_NORECIPIENT      	);
	COMMAND_ERROR_HANDLER( ERR_NOSUCHCHANNEL    	);
	COMMAND_ERROR_HANDLER( ERR_NOSUCHNICK       	);
	COMMAND_ERROR_HANDLER( ERR_NOSUCHSERVER     	);
	COMMAND_ERROR_HANDLER( ERR_NOSUCHSERVICE    	);
	COMMAND_ERROR_HANDLER( ERR_NOTEXTTOSEND     	);
	COMMAND_ERROR_HANDLER( ERR_NOTONCHANNEL     	);
	COMMAND_ERROR_HANDLER( ERR_NOTOPLEVEL       	);
	COMMAND_ERROR_HANDLER( ERR_NOTREGISTERED    	);
	COMMAND_ERROR_HANDLER( ERR_PASSWDMISMATCH   	);
	COMMAND_ERROR_HANDLER( ERR_RESTRICTED       	);
	COMMAND_ERROR_HANDLER( ERR_SUMMONDISABLED   	);
	COMMAND_ERROR_HANDLER( ERR_TOOMANYCHANNELS  	);
	COMMAND_ERROR_HANDLER( ERR_TOOMANYMATCHES   	);
	COMMAND_ERROR_HANDLER( ERR_TOOMANYTARGETS   	);
	COMMAND_ERROR_HANDLER( ERR_UMODEUNKNOWNFLAG 	);
	COMMAND_ERROR_HANDLER( ERR_UNAVAILRESOURCE  	);
	COMMAND_ERROR_HANDLER( ERR_UNIQOPPRIVSNEEDED	);
	COMMAND_ERROR_HANDLER( ERR_UNKNOWNCOMMAND   	);
	COMMAND_ERROR_HANDLER( ERR_UNKNOWNMODE      	);
	COMMAND_ERROR_HANDLER( ERR_USERNOTINCHANNEL 	);
	COMMAND_ERROR_HANDLER( ERR_USERONCHANNEL    	);
	COMMAND_ERROR_HANDLER( ERR_USERSDISABLED    	);
	COMMAND_ERROR_HANDLER( ERR_USERSDONTMATCH   	);
	COMMAND_ERROR_HANDLER( ERR_WASNOSUCHNICK    	);
	COMMAND_ERROR_HANDLER( ERR_WILDTOPLEVEL     	);
	COMMAND_ERROR_HANDLER( ERR_YOUREBANNEDCREEP 	);
	COMMAND_ERROR_HANDLER( ERR_YOUWILLBEBANNED  	);
	}

	// Si se ha llegado hasta aqui, todo ha funcionado (probablemente)
	return OK;

#undef COMMAND_QUERY_HANDLER
#undef COMMAND_REPLY_HANDLER
#undef COMMAND_ERROR_HANDLER
}

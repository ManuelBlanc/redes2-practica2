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
		ChannelList chan = channellist_findByName(server_get_channellist(serv), target);
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
		UserList recv = userlist_findByName(server_get_userlist(serv), target);
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
	char* nick;
	char* prefix;
	char* user_name;
	char* realname;
	char* mode;
	// The <mode> parameter should be a numeric, and can be used to
	//automatically set user modes when registering with the server.  This
	//parameter is a bitmask, with only 2 bits having any signification: if
	//the bit 2 is set, the user mode 'w' will be set and if the bit 3 is
	//set, the user mode 'i' will be set.
	if (0 != IRCParse_User(str, &prefix, &user_name, &mode, &realname)) {
		IRC_ErrNeedMoreParams();
		IRC_ErrNeedMoreParams(buf, prefix, nick, str);
		user_send_cmd(usr, buf);
		return ERR;
	}
	User* usr = userlist_findByName();
	if (NULL == usr){
		//setters...
		//server_add_user();
		return OK;
	}
	//IRC_ErrAlreadyRegistred();
	return ERR;
}

// ================================================================================================

/*
	The admin command is used to find information about the administrator
	of the given server, or current server if <target> parameter is
	omitted.  Each server MUST have the ability to forward ADMIN messages
	to other servers.
*/
int exec_cmd_admin(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	With the AWAY command, clients can set an automatic reply string for
	any PRIVMSG commands directed at them (not to a channel they are on).
	The server sends an automatic reply to the client sending the PRIVMSG
	command.  The only replying server is the one to which the sending
	client is connected to.

	The AWAY command is used either with one parameter, to set an AWAY
	message, or with no parameters, to remove the AWAY message.

	Because of its high cost (memory and bandwidth wise), the AWAY
	message SHOULD only be used for client-server communication.  A
	server MAY choose to silently ignore AWAY messages received from
	other servers.  To update the away status of a client across servers,
	the user mode 'a' SHOULD be used instead.  (See Section 3.1.5)
*/
int exec_cmd_away(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	Extension magica de Eloy
*/
int exec_cmd_cnotice(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	The CONNECT command can be used to request a server to try to
	establish a new connection to another server immediately.  CONNECT is
	a privileged command and SHOULD be available only to IRC Operators.
	If a <remote server> is given and its mask doesn't match name of the
	parsing server, the CONNECT attempt is sent to the first match of
	remote server. Otherwise the CONNECT attempt is made by the server
	processing the request.

	The server receiving a remote CONNECT command SHOULD generate a
	WALLOPS message describing the source and target of the request.
*/
int exec_cmd_connect(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	Extension magica de Eloy.
*/
int exec_cmd_cprivmsg(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	An operator can use the DIE command to shutdown the server.  This
	message is optional since it may be viewed as a risk to allow
	arbitrary people to connect to a server as an operator and execute
	this command.

	The DIE command MUST always be fully processed by the server to which
	the sending client is connected and MUST NOT be passed onto other
	connected servers.
*/
int exec_cmd_die(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	Extension magica de Eloy.
*/
int exec_cmd_encap(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	The ERROR command is for use by servers when reporting a serious or
	fatal error to its peers.  It may also be sent from one server to
	another but MUST NOT be accepted from any normal unknown clients.

	Only an ERROR message SHOULD be used for reporting errors which occur
	with a server-to-server link.  An ERROR message is sent to the server
	at the other end (which reports it to appropriate local users and
	logs) and to appropriate local users and logs.  It is not to be
	passed onto any other servers by a server if it is received from a
	server.

	The ERROR message is also used before terminating a client
	connection.

	When a server sends a received ERROR message to its operators, the
	message SHOULD be encapsulated inside a NOTICE message, indicating
	that the client was not responsible for the error.
*/
int exec_cmd_error(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	Extension magica de Eloy.
*/
int exec_cmd_help(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	The INFO command is REQUIRED to return information describing the
	server: its version, when it was compiled, the patchlevel, when it
	was started, and any other miscellaneous information which may be
	considered to be relevant.

	Wildcards are allowed in the <target> parameter.
*/
int exec_cmd_info(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	The INVITE command is used to invite a user to a channel.  The
	parameter <nickname> is the nickname of the person to be invited to
	the target channel <channel>.  There is no requirement that the
	channel the target user is being invited to must exist or be a valid
	channel.  However, if the channel exists, only members of the channel
	are allowed to invite other users.  When the channel has invite-only
	flag set, only channel operators may issue INVITE command.

	Only the user inviting and the user being invited will receive
	notification of the invitation.  Other channel members are not
	notified.  (This is unlike the MODE changes, and is occasionally the
	source of trouble for users.)
*/
int exec_cmd_invite(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	The ISON command was implemented to provide a quick and efficient
	means to get a response about whether a given nickname was currently
	on IRC. ISON only takes one (1) type of parameter: a space-separated
	list of nicks.  For each nickname in the list that is present, the
	server adds that to its reply string.  Thus the reply string may
	return empty (none of the given nicks are present), an exact copy of
	the parameter string (all of them present) or any other subset of the
	set of nicks given in the parameter.  The only limit on the number of
	nicks that may be checked is that the combined length MUST NOT be too
	large as to cause the server to chop it off so it fits in 512
	characters.

	ISON is only processed by the server local to the client sending the
	command and thus not passed onto other servers for further
	processing.
*/
int exec_cmd_ison(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	The JOIN command is used by a user to request to start listening to
	the specific channel.  Servers MUST be able to parse arguments in the
	form of a list of target, but SHOULD NOT use lists when sending JOIN
	messages to clients.

	Once a user has joined a channel, he receives information about
	all commands his server receives affecting the channel.  This
	includes JOIN, MODE, KICK, PART, QUIT and of course PRIVMSG/NOTICE.
	This allows channel members to keep track of the other channel
	members, as well as channel modes.

	If a JOIN is successful, the user receives a JOIN message as
	confirmation and is then sent the channel's topic (using RPL_TOPIC) and
	the list of users who are on the channel (using RPL_NAMREPLY), which
	MUST include the user joining.

	Note that this message accepts a special argument ("0"), which is
	a special request to leave all channels the user is currently a member
	of.  The server will process this message as if the user had sent
	a PART command (See Section 3.2.2) for each channel he is a member
	of.
*/
int exec_cmd_join(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	The KICK command can be used to request the forced removal of a user
	from a channel.  It causes the <user> to PART from the <channel> by
	force.  For the message to be syntactically correct, there MUST be
	either one channel parameter and multiple user parameter, or as many
	channel parameters as there are user parameters.  If a "comment" is
	given, this will be sent instead of the default message, the nickname
	of the user issuing the KICK.

	The server MUST NOT send KICK messages with multiple channels or
	users to clients.  This is necessarily to maintain backward
	compatibility with old client software.
*/
int exec_cmd_kick(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_kill(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_knock(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_links(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_list(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_lusers(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	The user MODE's are typically changes which affect either how the
	client is seen by others or what 'extra' messages the client is sent.

	A user MODE command MUST only be accepted if both the sender of the
	message and the nickname given as a parameter are both the same.  If
	no other parameter is given, then the server will return the current
	settings for the nick.

	The flag 'a' SHALL NOT be toggled by the user using the MODE command,
	instead use of the AWAY command is REQUIRED.

	If a user attempts to make themselves an operator using the "+o" or
	"+O" flag, the attempt SHOULD be ignored as users could bypass the
	authentication mechanisms of the OPER command.  There is no
	restriction, however, on anyone `deopping' themselves (using "-o" or
	"-O").

	On the other hand, if a user attempts to make themselves unrestricted
	using the "-r" flag, the attempt SHOULD be ignored.  There is no
	restriction, however, on anyone `deopping' themselves (using "+r").
	This flag is typically set by the server upon connection for
	administrative reasons.  While the restrictions imposed are left up
	to the implementation, it is typical that a restricted user not be
	allowed to change nicknames, nor make use of the channel operator
	status on channels.

	The flag 's' is obsolete but MAY still be used.
*/
int exec_cmd_mode(Server* serv, User* usr, const char* cmd) {
	char* nick;
	char* channel_name;
	char* user_target;
	char* prefix;
	char* mode;
	char buf[IRC_MAX_CMD_LEN+1];
	long opt;

	user_get_nick(usr, &nick);
	if (OK != IRCParse_Mode(str, &prefix, &channel_name, &mode, &user_target)){
		IRC_ErrUnKnownCommand(buf, prefix, nick, str);
		user_send_cmd(usr, buf);
		return ERR;
	}

	namechannel_skip_colon(channel_name);
	Channel* chan = channellist_findByName(serv->chan, channel_name);
	if (NULL != user_target) {
		if (mode == '+') {
			opt = channel_set_flag_user(chan, user_target, mode, usr);
		}
		else if (mode == '-') {
			opt = channel_unset_flag_user(chan, user_target, mode, usr);
		}
		else {
			//si no hacemos devolver banlist, invitedlist o exceptionlist, esto BIEN
			opt = RPL_UNIQOPIS;
		}
		switch (opt) {
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
	}
	else {
		User* recv = userlist_findByName(serv->usrs, user_target);
		if (NULL != recv) opt = user_send_message(recv, nick, msg);
		else opt = ERR_NOSUCHNICK;
		switch (opt) {
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

	}
	return OK;
}

// ================================================================================================

/*
	The MOTD command is used to get the "Message Of The Day" of the given
	server, or current server if <target> is omitted.

	Wildcards are allowed in the <target> parameter.

*/
int exec_cmd_motd(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	By using the NAMES command, a user can list all nicknames that are
	visible to him. For more details on what is visible and what is not,
	see "Internet Relay Chat: Channel Management" [IRC-CHAN].  The
	<channel> parameter specifies which channel(s) to return information
	about.  There is no error reply for bad channel names.

	If no <channel> parameter is given, a list of all channels and their
	occupants is returned.  At the end of this list, a list of users who
	are visible but either not on any channel or not on a visible channel
	are listed as being on `channel' "*".

	If the <target> parameter is specified, the request is forwarded to
	that server which will generate the reply.

	Wildcards are allowed in the <target> parameter.
*/
int exec_cmd_names(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_namesx(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_nick(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_notice(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_oper(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_part(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_pass(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_ping(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_pong(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_privmsg(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_quit(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_rehash(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_restart(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_rules(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_server(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_service(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_servlist(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_setname(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_silence(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_squery(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_squit(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_stats(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_summon(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_time(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*

*/
int exec_cmd_topic(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	TRACE commaallowed in the <target> parameter.
*/
int exec_cmd_trace(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	Extension magica de Eloy.
*/
int exec_cmd_uhnames(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	The USER command is used at the beginning of connection to specify
	the username, hostname and realname of a new user.

	The <mode> parameter should be a numeric, and can be used to
	automatically set user modes when registering with the server.  This
	parameter is a bitmask, with only 2 bits having any signification: if
	the bit 2 is set, the user mode 'w' will be set and if the bit 3 is
	set, the user mode 'i' will be set.  (See Section 3.1.5 "User
	Modes").

	The <realname> may contain space characters.
*/
int exec_cmd_user(Server* serv, User* usr, const char* cmd) {
	char nick[USER_MAX_NICK_LEN + 1];
	char* prefix;
	char* user_name;
	char* realname;
	char* mode;
	char buf[512];
	if (0 != IRCParse_User(str, &prefix, &user_name, &mode, &realname)){
		IRC_ErrNeedMoreParams(buf, prefix, user_name, str);
		user_send_cmd(usr, buf);
		return ERR;
	}
	UserList usr_using = userlist_findByName(&serv->usrs, user_name);
	if (NULL == usr_using){
		setters...
		server_add_user(serv, usr);
		return OK;
	}
	IRC_ErrAlreadyRegistred();
	return ERR;
}

// ================================================================================================

/*
	The USERHOST command takes a list of up to 5 nicknames, each
	separated by a space character and returns a list of information
	about each nickname that it found.  The returned list has each reply
	separated by a space.
*/
int exec_cmd_userhost(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	Extension magica de Eloy.
*/
int exec_cmd_userip(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	The USERS command returns a list of users logged into the server in a
	format similar to the UNIX commands who(1), rusers(1) and finger(1).
	If disabled, the correct numeric MUST be returned to indicate this.

	Because of the security implications of such a command, it SHOULD be
	disabled by default in server implementations.  Enabling it SHOULD
	require recompiling the server or some equivalent change rather than
	simply toggling an option and restarting the server.  The procedure
	to enable this command SHOULD also include suitable large comments.
*/
int exec_cmd_users(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	The VERSION command is used to query the version of the server
	program.  An optional parameter <target> is used to query the version
	of the server program which a client is not directly connected to.

	Wildcards are allowed in the <target> parameter.
*/
int exec_cmd_version(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	The WALLOPS command is used to send a message to all currently
	connected users who have set the 'w' user mode for themselves.  (See
	Section 3.1.5 "User modes").
	After implementing WALLOPS as a user command it was found that it was
	often and commonly abused as a means of sending a message to a lot of
	people.  Due to this, it is RECOMMENDED that the implementation of
	WALLOPS allows and recognizes only servers as the originators of
	WALLOPS.
*/
int exec_cmd_wallops(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	Extension magica de Eloy.
*/
int exec_cmd_watch(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	The WHO command is used by a client to generate a query which returns
	a list of information which 'matches' the <mask> parameter given by
	the client.  In the absence of the <mask> parameter, all visible
	(users who aren't invisible (user mode +i) and who don't have a
	common channel with the requesting client) are listed.  The same
	result can be achieved by using a <mask> of "0" or any wildcard which
	will end up matching every visible user.

	The <mask> passed to WHO is matched against users' host, server, real
	name and nickname if the channel <mask> cannot be found.
	If the "o" parameter is passed only operators are returned according
	to the <mask> supplied.
*/
int exec_cmd_who(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	This command is used to query information about particular user.
	The server will answer this command with several numeric messages
	indicating different statuses of each user which matches the mask (if
	you are entitled to see them).  If no wildcard is present in the
	<mask>, any information about that nick which you are allowed to see
	is presented.

	If the <target> parameter is specified, it sends the query to a
	specific server.  It is useful if you want to know how long the user
	in question has been idle as only local server (i.e., the server the
	user is directly connected to) knows that information, while
	everything else is globally known.

	Wildcards are allowed in the <target> parameter.
*/
int exec_cmd_whois(Server* serv, User* usr, const char* cmd) {

}

// ================================================================================================

/*
	Whowas asks for information about a nickname which no longer exists.
	This may either be due to a nickname change or the user leaving IRC.
	In response to this query, the server searches through its nickname
	history, looking for any nicks which are lexically the same (no wild
	card matching here).  The history is searched backward, returning the
	most recent entry first.  If there are multiple entries, up to
	<count> replies will be returned (or all of them if no <count>
	parameter is given).  If a non-positive number is passed as being
	<count>, then a full search is done.

	Wildcards are allowed in the <target> parameter.
*/
int exec_cmd_whowas(Server* serv, User* usr, const char* cmd) {

}

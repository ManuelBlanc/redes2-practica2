#include <time.h>
#include <string.h>
#include <stdlib.h>
/* redes2 */
#include <redes2/irc.h>

/* usr */
#include "G-2301-05-P2-config.h"
#include "G-2301-05-P2-util.h"
#include "G-2301-05-P2-channel.h"
#include "G-2301-05-P2-user.h"
#include "G-2301-05-P2-server.h"
#include "G-2301-05-P2-switches.h"

#define UNIMPLEMENTED_COMMAND(name, reason)                                                           	\
static int exec_cmd_##name(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {	\
    UNUSED(serv);                                                                                     	\
    UNUSED(usr);                                                                                      	\
    UNUSED(sprefix);                                                                                  	\
    UNUSED(nick);                                                                                     	\
    UNUSED(cmd);                                                                                      	\
    UNUSED(buf);                                                                                      	\
    LOG("Recibido un %s de %s, ignorandolo por la razon: %s", #name, nick, reason);                   	\
    return OK;                                                                                        	\
}                                                                                                     	//

#define PARSE_PROTECT(cmd_name, parse)                                     	\
if (0 > (parse)) {                                                         	\
    LOG("Error al parsear un comando %s recibido de %s", (cmd_name), nick);	\
    IRC_ErrNeedMoreParams(buf, sprefix, nick, (cmd_name));                 	\
    user_send_cmd(usr, buf);                                               	\
    return ERR;                                                            	\
}                                                                          	//

static int switchesP_server_is_target(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* target) {
	char* serv_name = NULL;

	if (NULL == target) return 1; // Por defecto somos el objetivo

	// Comprobamos que este destinado a nosotros
	server_get_name(serv, &serv_name);
	if (!strncasecmp(serv_name, target, SERVER_MAX_NAME_LEN)) {
		free(serv_name);
		return 1;
	}
	else {
		free(serv_name);
		IRC_ErrNoSuchServer(buf, sprefix, nick, target);
		user_send_cmd(usr, buf);
		return 0;
	}
}

static void switchesP_send_channel_nicknames(Server* serv, User* usr, char* buf, char* sprefix, char* nick, Channel* chan) {
	char* channel_name = NULL;
	char** namelist = NULL;
	char** namelist2 = NULL;

	UNUSED(serv);

	// Obtenemos la lista de usuarios
	channel_get_name(chan, &channel_name);
	channel_get_user_names(chan, 0, &namelist);
	namelist2 = namelist;

	// Buscamos el tipo del canal para la lista de usuarios
	char* channel_type;
	     if (channel_has_flag(chan, 'p')) channel_type = "@";
	else if (channel_has_flag(chan, 's')) channel_type = "*";
	else                                  channel_type = "=";


	while (NULL != *namelist2) {
		IRC_RplNamReply(buf, sprefix, nick, channel_type, channel_name, *namelist2);
		user_send_cmd(usr, buf);
		free(*namelist2);
		namelist2++;
	}

	IRC_RplEndOfNames(buf, sprefix, nick, channel_name);
	user_send_cmd(usr, buf);
}

// Se salta los dos puntos de una cadena (si estan ahi)
/*static char* switchesP_skip_colon(char* channel) {
	return ':' == *channel ? channel+1 : channel;
}*/

//Separa una cadena por espacios y/o comas
static int parse_lists(char* str, char*** elems) {
	int i = 0;

	char* str2 = str;
	for(i=0; str2[i]; (str2[i]==',' || str2[i]==' ') ? i++ : *str2++);
	*elems = malloc((i + 2) * sizeof(*elems));

	i = 0;
	str2 = str;
	while (1) {
		size_t tam = strcspn(str2, " ,");
		(*elems)[i] = strndup(str2, tam);
		str2 = str2 + tam;
		if(str2[0] == '\0') break;
		str2 = str2 + 1;
		i++;
	}
	i++;
	(*elems)[i] = NULL;
	return OK;
}

// ================================================================================================

/*
	The admin command is used to find information about the administrator
	of the given server, or current server if <target> parameter is
	omitted.  Each server MUST have the ability to forward ADMIN messages
	to other servers.
*/
static int exec_cmd_ADMIN(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	char* target = NULL;
	char* serv_name = NULL;
	char* prefix = NULL;
	ServerAdmin* serv_admin = NULL;

	PARSE_PROTECT("ADMIN", IRCParse_Admin(cmd, &prefix, &target));

	// Si no esta destinado a nosotros, abortamos con error
	if (!switchesP_server_is_target(serv, usr, buf, sprefix, nick, target)) goto cleanup;

	server_get_name(serv, &serv_name);


	// Obtenemos los datos de manera segura
	if (OK == server_get_admin(serv, &serv_admin)) {
		// Aqui necesitamos acceder a la estructura server
		// y obtener los campos:
		// admin_me   admin_loc1   admin_info   admin_mail
		IRC_RplAdminMe(buf, sprefix, nick, serv_name, "Administrative info.");
		user_send_cmd(usr, buf);
		IRC_RplAdminLoc1(buf, sprefix, nick, serv_admin->loc1);
		user_send_cmd(usr, buf);
		IRC_RplAdminLoc2(buf, sprefix, nick, serv_admin->loc2);
		user_send_cmd(usr, buf);
		IRC_RplAdmineMail(buf, sprefix, nick, serv_admin->email);
		user_send_cmd(usr, buf);
	}
	else {
		IRC_ErrNoAdminInfo(buf, sprefix, nick, serv_name);
		user_send_cmd(usr, buf);
	}

cleanup:
	free(target);
	free(serv_name);
	free(prefix);
	free(serv_admin);
	return OK;
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
static int exec_cmd_AWAY(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	UNUSED(serv);
	char* prefix = NULL;
	char* msg = NULL;

	PARSE_PROTECT("AWAY", IRCParse_Away(cmd, &prefix, &msg));

	// Ponemos o quitamos el away
	user_set_away(usr, msg);

	// Avisamos al cliente
	if (NULL != msg) IRC_RplNowAway(buf, sprefix, nick);
	else             IRC_RplUnaway(buf, sprefix, nick);
	user_send_cmd(usr, buf);

	free(prefix);
	free(msg);
	return OK;
}

// ================================================================================================

/*
	Extension del RFC (no implementado).
*/
UNIMPLEMENTED_COMMAND(CNOTICE, "Extension del RFC")

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
UNIMPLEMENTED_COMMAND(CONNECT, "Comando de interconexion de servidores")


// ================================================================================================

/*
	Extension del RFC (no implementado).
*/
UNIMPLEMENTED_COMMAND(CPRIVMSG, "Extension del RFC")

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
UNIMPLEMENTED_COMMAND(DIE, "Comando opcional con riesgos de seguridad graves")

// ================================================================================================

/*
	Extension del RFC (no implementado).
*/
UNIMPLEMENTED_COMMAND(ENCAP, "Extension del RFC")

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
UNIMPLEMENTED_COMMAND(ERROR, "Comando de interconexion de servidores")

// ================================================================================================

/*
	Extension del RFC (no implementado).
*/
UNIMPLEMENTED_COMMAND(HELP, "Extension del RFC")

// ================================================================================================

/*
	The INFO command is REQUIRED to return information describing the
	server: its version, when it was compiled, the patchlevel, when it
	was started, and any other miscellaneous information which may be
	considered to be relevant.

	Wildcards are allowed in the <target> parameter.
*/
static int exec_cmd_INFO(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	char* prefix = NULL;
	char* target = NULL;
	char* serv_name = NULL;

	PARSE_PROTECT("INFO", IRCParse_Info(cmd, &prefix, &target));

	// Si no esta destinado a nosotros, abortamos con error
	if (!switchesP_server_is_target(serv, usr, buf, sprefix, nick, target)) goto cleanup;

	server_get_name(serv, &serv_name);

	IRC_RplInfo(buf, sprefix, nick, PACKAGE_STRING);
	user_send_cmd(usr, buf);
	IRC_RplEndOfInfo(buf, sprefix, nick);
	user_send_cmd(usr, buf);

cleanup:
	free(serv_name);
	free(prefix);
	free(target);
	return OK;
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
static int exec_cmd_INVITE(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	char* invitee_nick = NULL;
	char* channel_name = NULL;
	char* prefix = NULL;

	PARSE_PROTECT("INVITE", IRCParse_Invite(cmd, &prefix, &invitee_nick, &channel_name));

	User* user_invited = userlist_head(userlist_findByNickname(server_get_userlist(serv), invitee_nick));
	if (user_invited == NULL) {
		IRC_ErrNoSuchNick(buf, sprefix, nick, invitee_nick);
		user_send_cmd(usr, buf);
		goto cleanup;
	}


	// Obtenemos el prefix de verdad
	free(prefix);
	user_get_prefix(usr, &prefix);

	// Invitamos al usuario
	IRC_Invite(buf, prefix, invitee_nick, channel_name);
	user_send_cmd(user_invited, buf);

	// Avisamos a la sala
	Channel* channel = channellist_head(channellist_findByName(server_get_channellist(serv), channel_name));
	if(NULL != channel) {
		channel_set_flag_user(channel, user_invited, 'I', usr);
		IRC_RplInviting(buf, sprefix, channel_name, nick, invitee_nick);
		channel_send_cmd(channel, buf);
	}

cleanup:
	free(invitee_nick);
	free(channel_name);
	return OK;
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
static int exec_cmd_ISON(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	char* prefix = NULL;
	char* nick_str = NULL;
	char** nick_list = NULL;
	int i = 0;
	UserList ulist = server_get_userlist(serv);

	PARSE_PROTECT("ISON", IRCParse_Ison(cmd, &prefix, &nick_str))

	parse_lists(nick_str, &nick_list);
	while (nick_list[i] != NULL) {
		// Si esta el usuario
		if (NULL != userlist_head(userlist_findByNickname(ulist, nick_list[i]))) {
			// Enviamos un mensaje avisando
			IRC_RplIson(buf, sprefix, nick, nick_list[i], NULL);
			user_send_cmd(usr, buf);
		}
		free(nick_list[i]);
		i++;
	}

	free(prefix);
	free(nick_str);
	free(nick_list);
	return OK;
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
static int exec_cmd_JOIN(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	char* prefix = NULL;
	char* channel_name = NULL;
	char* channel_key = NULL;
	char* topic = NULL;
	char* msg = NULL;

	PARSE_PROTECT("JOIN", IRCParse_Join(cmd, &prefix, &channel_name, &channel_key, &msg));

	// Obtenemos el prefix de verdad
	free(prefix);
	user_get_prefix(usr, &prefix);

	Channel* chan;
	// Creamos o obtenemos el canal que buscamos
	switch (server_add_or_create_channel(serv, channel_name, &chan)) {
		case ERR_UNAVAILRESOURCE:
			IRC_ErrUnavailResource(buf, sprefix, nick, channel_name);
			user_send_cmd(usr, buf);
			goto cleanup;
		case ERR_NOSUCHCHANNEL:
			IRC_ErrNoSuchChannel(buf, sprefix, nick, channel_name);
			user_send_cmd(usr, buf);
			goto cleanup;
		case OK:
			break;
	}

	// Si ya estamos en el canal no hay nada que hacer
	if (channel_has_user(chan, usr)) goto cleanup;
	// En caso contrario nos intentamos unir al canal
	switch (channel_join(chan, usr, channel_key)) {
		case ERR_NEEDMOREPARAMS:
			IRC_ErrNeedMoreParams(buf, sprefix, nick, "JOIN");
			user_send_cmd(usr, buf);
			goto cleanup;
		case ERR_CHANNELISFULL:
			IRC_ErrChannelIsFull(buf, sprefix, nick, channel_name);
			user_send_cmd(usr, buf);
			goto cleanup;
		case ERR_BANNEDFROMCHAN:
			IRC_ErrBannedFromChan(buf, sprefix, nick, channel_name);
			user_send_cmd(usr, buf);
			goto cleanup;
		case ERR_BADCHANNELKEY:
			IRC_ErrBadChannelKey(buf, sprefix, nick, channel_name);
			user_send_cmd(usr, buf);
			goto cleanup;
		case ERR_INVITEONLYCHAN:
			IRC_ErrInviteOnlyChan(buf, sprefix, nick, channel_name);
			user_send_cmd(usr, buf);
			goto cleanup;
		case OK:
			//Ponemos las flags de creador y oper
			if(channel_get_user_count(chan) == 1) {
				channel_set_flag_user(chan, usr, 'O', NULL);
				channel_set_flag_user(chan, usr, 'o', NULL);
			}
			break;
	}

	// Join con exito, avisamos a todos los usuarios del canal
	IRC_Join(buf, prefix, channel_name, channel_key, msg);
	channel_send_cmd(chan, buf);

	// Enviamos el topic (si existe)
	channel_get_topic(chan, &topic);
	if (NULL != topic) {
		IRC_RplTopic(buf, sprefix, nick, channel_name, topic);
		user_send_cmd(usr, buf);
	}

	switchesP_send_channel_nicknames(serv, usr, buf, sprefix, nick, chan);

cleanup:
	free(topic);
	free(prefix);
	free(channel_name);
	free(channel_key);
	return OK;
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
	ERR_NEEDMOREPARAMS  	ERR_NOSUCHCHANNEL
	ERR_BADCHANMASK     	ERR_CHANOPRIVSNEEDED
	ERR_USERNOTINCHANNEL	ERR_NOTONCHANNEL
*/
static int exec_cmd_KICK(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {

	char* prefix = NULL;
        char* prefix2 = NULL;
	char* channel_name = NULL;
	char* target_user = NULL;
	char* comment = NULL;

	PARSE_PROTECT("KICK", IRCParse_Kick(cmd, &prefix, &channel_name, &target_user, &comment));

	User* target = userlist_head(userlist_findByNickname(server_get_userlist(serv), target_user));
	Channel* channel = channellist_head(channellist_findByName(server_get_channellist(serv), channel_name));

        user_get_prefix(target, &prefix2);

	// Por defecto la razon es el el nombre de quien kickea
	if (NULL == comment) user_get_name(usr, &comment);

	switch (channel_part(channel, target, usr)) {
		case ERR_NEEDMOREPARAMS:
			IRC_ErrNeedMoreParams(buf, sprefix, nick, channel_name);
			user_send_cmd(usr, buf);
			break;
		case ERR_CHANOPRIVSNEEDED:
			IRC_ErrChanOPrivsNeeded(buf, sprefix, nick, channel_name);
			user_send_cmd(usr, buf);
			break;
		case ERR_NOSUCHCHANNEL:
			IRC_ErrNoSuchChannel(buf, sprefix, nick, channel_name);
			user_send_cmd(usr, buf);
			break;
		case ERR_NOTONCHANNEL:
			IRC_ErrNotOnChannel(buf, sprefix, nick, nick, channel_name);
			user_send_cmd(usr, buf);
			break;
		case OK:
			IRC_Part(buf, prefix2, channel_name, comment);
			channel_send_cmd(channel, buf);
                        IRC_Kick(buf, prefix, channel_name, target_user, comment);
                        user_send_cmd(target, buf);
                        break;
	}

	free(prefix);
        free(prefix2);
	free(channel_name);
	free(target_user);
	free(comment);
	return OK;
}

// ================================================================================================

/*
	The KILL command is used to cause a client-server connection to be
	closed by the server which has the actual connection.  Servers
	generate KILL messages on nickname collisions.  It MAY also be
	available available to users who have the operator status.

	Clients which have automatic reconnect algorithms effectively make
	this command useless since the disconnection is only brief.  It does
	however break the flow of data and can be used to stop large amounts
	of 'flooding' from abusive users or accidents.  Abusive users usually
	don't care as they will reconnect promptly and resume their abusive
	behaviour.  To prevent this command from being abused, any user may
	elect to receive KILL messages generated for others to keep an 'eye'
	on would be trouble spots.

	In an arena where nicknames are REQUIRED to be globally unique at all
	times, KILL messages are sent whenever 'duplicates' are detected
	(that is an attempt to register two users with the same nickname) in
	the hope that both of them will disappear and only 1 reappear.

	When a client is removed as the result of a KILL message, the server
	SHOULD add the nickname to the list of unavailable nicknames in an
	attempt to avoid clients to reuse this name immediately which is
	usually the pattern of abusive behaviour often leading to useless
	"KILL loops".  See the "IRC Server Protocol" document [IRC-SERVER]
	for more information on this procedure.

	The comment given MUST reflect the actual reason for the KILL.  For
	server-generated KILLs it usually is made up of details concerning
	the origins of the two conflicting nicknames.  For users it is left
	up to them to provide an adequate reason to satisfy others who see
	it.  To prevent/discourage fake KILLs from being generated to hide
	the identify of the KILLer, the comment also shows a 'kill-path'
	which is updated by each server it passes through, each prepending
	its name to the path.
*/
UNIMPLEMENTED_COMMAND(KILL, "No aceptamos kills de clientes")

// ================================================================================================

/*
	Extension del RFC (no implementado).
*/
UNIMPLEMENTED_COMMAND(KNOCK, "Extension del RFC")

// ================================================================================================

/*
	With LINKS, a user can list all servernames, which are known by the
	server answering the query.  The returned list of servers MUST match
	the mask, or if no mask is given, the full list is returned.

	If <remote server> is given in addition to <server mask>, the LINKS
	command is forwarded to the first server found that matches that name
	(if any), and that server is then required to answer the query.
*/
UNIMPLEMENTED_COMMAND(LINKS, "Comando de interconexion de servidores")

// ================================================================================================

/*
	The list command is used to list channels and their topics.  If the
	<channel> parameter is used, only the status of that channel is
	displayed.

	If the <target> parameter is specified, the request is forwarded to
	that server which will generate the reply.

	Wildcards are allowed in the <target> parameter.
*/

static void send_channel_status(Server* serv, User* usr, char* buf, char* sprefix, char* nick, Channel* chan);

static int exec_cmd_LIST(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	char* prefix = NULL;
	char* channel_name_list = NULL;
	char* target = NULL;
	char** channel_list = NULL;
	int i = 0;

	PARSE_PROTECT("LIST", IRCParse_List(cmd, &prefix, &channel_name_list, &target));

	// Si no esta destinado a nosotros, abortamos con error
	if (!switchesP_server_is_target(serv, usr, buf, sprefix, nick, target)) goto cleanup;

	ChannelList chanlist = server_get_channellist(serv);

	// Obsoleto segun el RFC2812, pero XChat es RFC1459
	IRC_RplListStart(buf, sprefix, nick);
	user_send_cmd(usr, buf);

	if (NULL != channel_name_list) {
		// Si se proporciona una lista de canales, informamos de esos canales
		parse_lists(channel_name_list, &channel_list);

		while (channel_list[i] != NULL) {
			char* channel_name = channel_list[i];
			Channel* chan = channellist_head(channellist_findByName(server_get_channellist(serv), channel_name));
			if(chan != NULL) send_channel_status(serv, usr, buf, sprefix, nick, chan);
			free(channel_name);
			i++;
		}
	}
	else {
		// Si no se proporciona, enviamos la informacion de todos
		while (1) {
			Channel* chan = channellist_head(chanlist);
			if (NULL == chan) break;
			send_channel_status(serv, usr, buf, sprefix, nick, chan);
			chanlist = channellist_tail(chanlist);

		}
	}

	// Fin de la lista, en cualquiera de los dos casos
	IRC_RplListEnd(buf, sprefix, nick);
	user_send_cmd(usr, buf);

cleanup:
	free(prefix);
	free(channel_name_list);
	free(target);
	free(channel_list);
	return OK;
}

static void send_channel_status(Server* serv, User* usr, char* buf, char* sprefix, char* nick, Channel* chan) {
	int isInvisible = channel_has_flag(chan, 'p') || channel_has_flag(chan, 's');
	char* name = NULL;
	char* topic = NULL;
	char visible[12];
	UNUSED(serv);

	if(isInvisible) return;

	channel_get_name(chan, &name);
	channel_get_topic(chan, &topic);
	sprintf(visible, "%ld", channel_get_user_count(chan));
	IRC_RplList(buf, sprefix, nick, name, visible, NULL==topic ? "" : topic);
	user_send_cmd(usr, buf);

	free(name);
	free(topic);
}

// ================================================================================================

/*
	The LUSERS command is used to get statistics about the size of the
	IRC network.  If no parameter is given, the reply will be about the
	whole net.  If a <mask> is specified, then the reply will only
	concern the part of the network formed by the servers matching the
	mask.  Finally, if the <target> parameter is specified, the request
	is forwarded to that server which will generate the reply.

	Wildcards are allowed in the <target> parameter.
*/
static int exec_cmd_LUSERS(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	char* prefix = NULL;
	char* mask = NULL;
	char* target = NULL;

	PARSE_PROTECT("LUSERS", IRCParse_Lusers(cmd, &prefix, &mask, &target));

	// Obtenemos el prefix de verdad
	free(prefix);
	user_get_prefix(usr, &prefix);

	// Si no esta destinado a nosotros, abortamos con error
	if (!switchesP_server_is_target(serv, usr, buf, sprefix, nick, target)) goto cleanup;

	int num_users = server_get_num_users(serv);

	IRC_RplLuserClient(buf, sprefix, nick, num_users, 0, 0); // users, invisibles, servers
	user_send_cmd(usr, buf);
	IRC_RplLuserOp(buf, sprefix, nick, 0); // ops
	user_send_cmd(usr, buf);
	IRC_RplLuserUnknown(buf, sprefix, nick, 0); // unknownconn
	user_send_cmd(usr, buf);
	IRC_RplLuserChannels(buf, sprefix, nick, server_get_num_channels(serv));
	user_send_cmd(usr, buf);
	IRC_RplLuserMe(buf, sprefix, nick, num_users, 0); // clients, servers
	user_send_cmd(usr, buf);

cleanup:
	free(prefix);
	free(mask);
	free(target);
	return OK;
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
static char* switchesP_channel_user_mode(Channel* chan, User* usr) {
	char* mode = ecalloc(1, 13); // Longitud de los modes + 2
	char* mstr = "OovbeI";
	mode[0] = '+';
	int i;
	for (i = 1; *mstr; mstr++, i++) {
		if (channel_has_flag_user(chan, usr, *mstr)) mode[i] = *mstr;
	}
	return mode;
}


static int exec_cmd_MODE(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	char* prefix = NULL;
	char* target = NULL;
	char* mode = NULL;
	char* user_target_str = NULL;
	long opt = 0; // Borrame

	PARSE_PROTECT("MODE", IRCParse_Mode(cmd, &prefix, &target, &mode, &user_target_str));

	free(prefix);
	user_get_prefix(usr, &prefix);

	// Este comando es un desproposito. No sabemos si el objetivo es un usuario o canal.

	// Primero probamos con canales
	Channel* chan = channellist_head(channellist_findByName(server_get_channellist(serv), target));
	char flag = (NULL != mode) ? mode[1] : 0;
	if (NULL != chan) {
		// El objetivo del mode es un canal!!
		if (NULL == mode) {
			opt = RPL_CHANNELMODEIS;
			mode = ecalloc(1, 20);
			char* mode_aux = mode;
			char* mode_aux2 = "aimnqpsrtkl";
			*mode_aux++ = '+';
			for (; *mode_aux2; mode_aux2++) {
				if (channel_has_flag(chan, *mode_aux2)) *mode_aux++ = *mode_aux2;
			}
		}
		// Vemos si es una flag de canal o usuario
		else if (strchr("OovbeI", flag)) {
			// Es una flag de usuario en canal
			User* user_target = userlist_head(userlist_findByNickname(server_get_userlist(serv), user_target_str));
			if (NULL == user_target) opt = ERR_USERSDONTMATCH;
			else {
				     if ('+' == mode[0]) opt = channel_set_flag_user(chan, user_target, flag, usr);
				else if ('-' == mode[0]) opt = channel_unset_flag_user(chan, user_target, flag, usr);
			}
		}
		else if (strchr("aimnqpsrtkl", flag)) {
			// Es una flag de canal
			     if ('+' == mode[0]) opt = channel_set_flag(chan, flag, usr);
			else if ('-' == mode[0]) opt = channel_unset_flag(chan, flag, usr);
		}
		else {
			// Si no hacemos devolver banlist, invitedlist exceptionlist o creador del canal, esto BIEN
			switch (mode[0]) {
				case 'b':
					opt = RPL_BANLIST;
					break;
				case 'e':
					opt = RPL_EXCEPTLIST;
					break;
				case 'I':
					opt = RPL_INVITELIST;
					break;
				case 'O':
					opt = RPL_UNIQOPIS;
					break;
				default:
					opt = ERR_UMODEUNKNOWNFLAG;
					break;
			}
		}
		//En mode madamos con el prefix de usuario no de server
		char** mask;
		char** aux;
		switch (opt) {
			case ERR_NEEDMOREPARAMS:
				IRC_ErrNeedMoreParams(buf, prefix, nick, "MODE");
				user_send_cmd(usr, buf);
				break;
			case ERR_KEYSET:
				IRC_ErrKeySet(buf, prefix, nick, target);
				user_send_cmd(usr, buf);
				break;
			case ERR_NOCHANMODES:
				IRC_ErrNoChanModes(buf, prefix, nick, target);
				user_send_cmd(usr, buf);
				break;
			case ERR_CHANOPRIVSNEEDED:
				IRC_ErrChanOPrivsNeeded(buf, prefix, nick, target);
				user_send_cmd(usr, buf);
				break;
			case ERR_USERNOTINCHANNEL:
				IRC_ErrUserNotInChannel(buf, prefix, nick, user_target_str, target);
				user_send_cmd(usr, buf);
				break;
			case ERR_UNKNOWNMODE:
				IRC_ErrUnknownMode(buf, prefix, nick, mode, target);
				user_send_cmd(usr, buf);
				break;
			case RPL_CHANNELMODEIS:
				IRC_RplChannelModeIs(buf, prefix, nick, target, mode);
				user_send_cmd(usr, buf);
				break;
			case RPL_UNIQOPIS:
				//funcion que devuelve el user con flag O en channel_creator
				channel_get_user_names(chan, 'O', &mask);
				if (*mask != NULL) {
					IRC_RplUniqOpIs(buf, prefix, nick, target, *mask);
					user_send_cmd(usr, buf);
					free(*mask);
				}
				free(mask);
				break;
			case RPL_BANLIST:
				channel_get_user_names(chan, 'b', &mask);
				aux = mask;
				while((*mask) != NULL) {
					IRC_RplBanList(buf, prefix, nick, target, *mask);
					user_send_cmd(usr, buf);
					free(*mask);
					mask++;
				}
				free(aux);
				IRC_RplEndOfBanList(buf, prefix, nick, target);
				user_send_cmd(usr, buf);
				break;
			case RPL_EXCEPTLIST:
				channel_get_user_names(chan, 'e', &mask);
				aux = mask;
				while((*mask) != NULL) {
					IRC_RplExceptList(buf, prefix, nick, target, *mask);
					user_send_cmd(usr, buf);
					free(*mask);
					mask++;
				}
				free(aux);
				IRC_RplEndOfExceptList(buf, prefix, nick, target);
				user_send_cmd(usr, buf);
				break;
			case RPL_INVITELIST:
				channel_get_user_names(chan, 'I', &mask);
				aux = mask;
				while((*mask) != NULL) {
					IRC_RplInviteList(buf, prefix, nick, target, *mask);
					user_send_cmd(usr, buf);
					free(*mask);
					mask++;
				}
				free(aux);
				IRC_RplEndOfInviteList(buf, prefix, nick, target);
				user_send_cmd(usr, buf);
				break;
			case OK:
				IRC_Mode(buf, prefix, target, mode, user_target_str);
				channel_send_cmd(chan, buf);
				break;
		}
	}
	else {
		if(strncasecmp(target, nick, USER_MAX_NICK_LEN)) {
			opt = ERR_USERSDONTMATCH;
		}
		else if(mode == NULL) {
			opt = RPL_UMODEIS;
			mode = ecalloc(1, 20);
			char* mode_aux = mode;
			char* mode_aux2 = "aiwroOs";
			*mode_aux++ = '+';
			for (; *mode_aux2; mode_aux2++) {
				if (user_has_flag(usr, *mode_aux2)) *mode_aux++ = *mode_aux2;
			}
		}
		else if (strchr("aiwroOs", flag)) {
			// Es una flag de usuario en canal

			     if ('+' == mode[0]) opt = user_set_flag(usr, flag, usr);
			else if ('-' == mode[0]) opt = user_unset_flag(usr, flag, usr);
			else opt = ERR_UMODEUNKNOWNFLAG;
		}
		else opt = ERR_UMODEUNKNOWNFLAG;

		switch (opt) {
			case ERR_NEEDMOREPARAMS:
				IRC_ErrNeedMoreParams(buf, prefix, nick, "MODE");
				user_send_cmd(usr, buf);
				break;
			case ERR_USERSDONTMATCH:
				IRC_ErrUsersDontMatch(buf, prefix, nick);
				user_send_cmd(usr, buf);
				break;
			case ERR_UMODEUNKNOWNFLAG:
				IRC_ErrUModeUnknownFlag(buf, prefix, nick);
				user_send_cmd(usr, buf);
				break;
			case OK:
				IRC_Mode(buf, prefix, nick, mode, "");
				user_send_cmd(usr, buf);
				break;
			case RPL_UMODEIS:
				IRC_RplUModeIs(buf, prefix, nick, mode);
				user_send_cmd(usr, buf);
				break;
		}
	}

	free(prefix);
	free(target);
	free(mode);
	free(user_target_str);
	return OK;
}

// ================================================================================================

/*
	The MOTD command is used to get the "Message Of The Day" of the given
	server, or current server if <target> is omitted.

	Wildcards are allowed in the <target> parameter.

*/
static int exec_cmd_MOTD(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	char* prefix = NULL;
	char* target = NULL;
	char* serv_name = NULL;
	char* motd_path = NULL;
	char motd_buffer[80 + 1];

	PARSE_PROTECT("MOTD", IRCParse_Motd(cmd, &prefix, &target));

	// Si no esta destinado a nosotros, abortamos con error
	if (!switchesP_server_is_target(serv, usr, buf, sprefix, nick, target)) goto cleanup;

	server_get_name(serv, &serv_name);
	server_get_motd(&motd_path);

	// Abrimos el fichero del MotD
	FILE* motd_file = fopen(motd_path, "r");
	if (NULL == motd_file) {
		IRC_ErrNoMotd(buf, sprefix, nick);
		user_send_cmd(usr, buf);
		goto cleanup;
	}

	// Inicio
	IRC_RplMotdStart(buf, sprefix, nick, serv_name);
	user_send_cmd(usr, buf);

	// Cuerpo
	while (NULL != fgets(motd_buffer, sizeof motd_buffer, motd_file)) {
		IRC_RplMotd(buf, sprefix, nick, motd_buffer);
		user_send_cmd(usr, buf);
	}

	// Fin
	IRC_RplEndOfMotd(buf, sprefix, nick);
	user_send_cmd(usr, buf);

	fclose(motd_file);

cleanup:
	free(prefix);
	free(target);
	free(serv_name);
	free(motd_path);
	return OK;
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
static int exec_cmd_NAMES(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	char* prefix = NULL;
	char* channel_str = NULL;
	char* target = NULL;
	char** channel_list = NULL;
	int i = 0;

	PARSE_PROTECT("NAMES", IRCParse_Names(cmd, &prefix, &channel_str, &target));

	// Si no esta destinado a nosotros, abortamos con error
	if (!switchesP_server_is_target(serv, usr, buf, sprefix, nick, target)) goto cleanup;

	if (NULL == channel_str) {
		// Si no se proporciona el channel, listamos todos los usuarios

		ChannelList clist = server_get_channellist(serv);
		while (1) {
			Channel* chan = channellist_head(clist);
			if (NULL == chan) break;
			switchesP_send_channel_nicknames(serv, usr, buf, sprefix, nick, chan);
			clist = channellist_tail(clist);
		}
	}
	else {

		ChannelList channels = server_get_channellist(serv);
		parse_lists(channel_str, &channel_list);

		while (1) {
			char* channel_name = channel_list[i++];
			if (NULL == channel_name) break;
			Channel* chan = channellist_head(channellist_findByName(channels, channel_name));
			if (NULL != chan) switchesP_send_channel_nicknames(serv, usr, buf, sprefix, nick, chan);
			free(channel_name);
		}
		free(channel_list);
	}

cleanup:
	free(prefix);
	free(channel_str);
	free(target);
	return OK;
}

// ================================================================================================

/*
	Extension del RFC (no implementado).
*/
UNIMPLEMENTED_COMMAND(NAMESX, "Extension del RFC");

// ================================================================================================

/*
	NICK command is used to give user a nickname or change the existing
	one.
	ERR_NONICKNAMEGIVEN     	ERR_ERRONEUSNICKNAME
	ERR_NICKNAMEINUSE       	ERR_NICKCOLLISION(no)
	ERR_UNAVAILRESOURCE (no)	ERR_RESTRICTED(no)
*/
int exec_cmd_NICK(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	char* prefix = NULL;
	char* nick_wanted = NULL;

	PARSE_PROTECT("NICK", IRCParse_Nick(cmd, &prefix, &nick_wanted));

	// Obtenemos el prefix de verdad
	free(prefix);
	user_get_prefix(usr, &prefix);

	if (NULL == nick_wanted || '\0' == *nick_wanted) {
		IRC_ErrNoNickNameGiven(buf, sprefix, nick);
		user_send_cmd(usr, buf);
		return OK;
	}

	UserList usrlist = server_get_userlist(serv);
	UserList usr_match = userlist_findByNickname(usrlist, nick_wanted);

	if (NULL != *usr_match) {
		IRC_ErrNickNameInUse(buf, sprefix, nick, nick_wanted);
		user_send_cmd(usr, buf);
		return OK;
	}

	if (OK != user_set_nick(usr, nick_wanted)) {
		IRC_ErrErroneusNickName(buf, sprefix, nick, nick_wanted);
		user_send_cmd(usr, buf);
		return OK;
	}

	IRC_Nick(buf, prefix, nick_wanted);
	user_send_cmd(usr, buf);

	ChannelList channels = server_get_channellist(serv);
	while (1) {
		Channel* chan = channellist_head(channels);
		if (NULL == chan) break;
		if (channel_has_user(chan, usr)) {
			channel_send_cmd(chan, buf);
		}
		channels = channellist_tail(channels);
	}

	free(prefix);
	free(nick_wanted);
	return OK;
}

// ================================================================================================

/*
	The NOTICE command is used similarly to PRIVMSG.  The difference
	between NOTICE and PRIVMSG is that automatic replies MUST NEVER be
	sent in response to a NOTICE message.  This rule applies to servers
	too - they MUST NOT send any error reply back to the client on
	receipt of a notice.  The object of this rule is to avoid loops
	between clients automatically sending something in response to
	something it received.

	This command is available to services as well as users.

	This is typically used by services, and automatons (clients with
	either an AI or other interactive program controlling their actions).

	See PRIVMSG for more details on replies and examples.
*/

static long checksend_message_usr(User* dst, User* src, char* msg);
static long checksend_message_chan(Channel* dst, User* src, char* msg);

static int exec_cmd_NOTICE(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
        char* prefix = NULL;
	char* target = NULL;
	char* msg = NULL;

	PARSE_PROTECT("NOTICE", IRCParse_Notice(cmd, &prefix, &target, &msg));

	// Obtenemos el prefix de verdad
	free(prefix);
	user_get_prefix(usr, &prefix);

	// Es un canal?
	if (strchr("#!&+", target[0])) {
		// Lo buscamos en los canales
		Channel* chan = channellist_head(channellist_findByName(server_get_channellist(serv), target));
		// Envia el mensaje si puede
		checksend_message_chan(chan, usr, msg);
	} else {
		// Estamos enviando a un usuario
		User* recv = userlist_head(userlist_findByNickname(server_get_userlist(serv), target));
		//Enviamos el mensaje si se puede
		checksend_message_usr(recv, usr, msg);
	}

	free(prefix);
	free(target);
	free(msg);
	return OK;
}

// ================================================================================================

/*
	A normal user uses the OPER command to obtain operator privileges.
	The combination of <name> and <password> are REQUIRED to gain
	Operator privileges.  Upon success, the user will receive a MODE
	message (see section 3.1.5) indicating the new user modes.
*/
static int exec_cmd_OPER(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	char* prefix = NULL;
	char* name = NULL;
	char* password = NULL;

	UNUSED(serv);

	PARSE_PROTECT("OPER", IRCParse_Oper(cmd, &prefix, &name, &password));

	IRC_ErrNoOperHost(buf, sprefix, nick);
	user_send_cmd(usr, buf);

	free(prefix);
	free(name);
	free(password);
	return OK;
}

// ================================================================================================

/*
	The PART command causes the user sending the message to be removed
	from the list of active members for all given channels listed in the
	parameter string.  If a "Part Message" is given, this will be sent
	instead of the default message, the nickname.  This request is always
	granted by the server.

	Servers MUST be able to parse arguments in the form of a list of
	target, but SHOULD NOT use lists when sending PART messages to
	clients.
*/
static int exec_cmd_PART(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	char* prefix = NULL;
	char* channel_str = NULL;
	char* msg = NULL;
	char** channel_list = NULL;
	int i = 0;

	PARSE_PROTECT("PART", IRCParse_Part(cmd, &prefix, &channel_str, &msg));
	parse_lists(channel_str, &channel_list);

	// Obtenemos el prefix de verdad
	free(prefix);
	user_get_prefix(usr, &prefix);

	while (channel_list[i] != NULL) {
		char* channel_name = channel_list[i];

		Channel* channel = channellist_head(channellist_findByName(server_get_channellist(serv), channel_name));
		switch (channel_part(channel, usr, NULL)) {
			case ERR_NOSUCHCHANNEL:
				IRC_ErrNoSuchChannel(buf, sprefix, nick, channel_name);
				user_send_cmd(usr, buf);
				break;
			case ERR_NOTONCHANNEL:
				IRC_ErrNotOnChannel(buf, sprefix, nick, nick, channel_name);
				user_send_cmd(usr, buf);
				break;
			case ERR_NEEDMOREPARAMS:
				IRC_ErrNeedMoreParams(buf, sprefix, nick, "PART");
				user_send_cmd(usr, buf);
				break;
			case OK:
				IRC_Part(buf, prefix, channel_name, msg);
				channel_send_cmd(channel, buf);
                                user_send_cmd(usr, buf);
                                // Borramos el canal si somos el ultimo en salir
                                if (0 == channel_get_user_count(channel)) {
					server_delete_channel(serv, channel_name);
                                }

				break;
		}
		free(channel_name);
		i++;
	}

	free(prefix);
	free(channel_str);
	free(msg);
	free(channel_list);
	return OK;
}

// ================================================================================================

/*
	The PASS command is used to set a 'connection password'.  The
	optional password can and MUST be set before any attempt to register
	the connection is made.  Currently this requires that user send a
	PASS command before sending the NICK/USER combination.
*/
int exec_cmd_PASS(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	UNUSED(buf);
	UNUSED(sprefix);
	UNUSED(nick);
	UNUSED(serv);
	UNUSED(usr);
	UNUSED(cmd);
	fprintf(stderr, "Funcion exec_cmd_PASS no implementada\n");
	return OK;
}

// ================================================================================================

/*
	The PING command is used to test the presence of an active client or
	server at the other end of the connection.  Servers send a PING
	message at regular intervals if no other activity detected coming
	from a connection.  If a connection fails to respond to a PING
	message within a set amount of time, that connection is closed.  A
	PING message MAY be sent even if the connection is active.

	When a PING message is received, the appropriate PONG message MUST be
	sent as reply to <server1> (server which sent the PING message out)
	as soon as possible.  If the <server2> parameter is specified, it
	represents the target of the ping, and the message gets forwarded
	there.
*/
static int exec_cmd_PING(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	char* prefix = NULL;
	char* origin = NULL;
	char* target = NULL;
	char* serv_name = NULL;

	PARSE_PROTECT("PING", IRCParse_Ping(cmd, &prefix, &origin, &target));

	// No hay origen definido
	if (NULL == origin) {
		IRC_ErrNoOrigin(cmd, sprefix, nick, "No origin specified");
		user_send_cmd(usr, buf);
		goto cleanup;
	}

	// Miramos si el objetivo somos nosotros
	server_get_name(serv, &serv_name);
	if (NULL == target || !strncasecmp(target, serv_name, SERVER_MAX_NAME_LEN)) {
		IRC_Pong(buf, sprefix, serv_name, origin);
		user_send_cmd(usr, buf);
		goto cleanup;
	}

	// Es caso contrario, (deberia) ser un usuario
	User* target_user = userlist_head(userlist_findByNickname(server_get_userlist(serv), target));
	if (NULL == target_user) {
		IRC_ErrNoSuchServer(buf, sprefix, nick, target);
		user_send_cmd(usr, buf);
	}
	else {
		// Le reenviamos al objetivo el ping tal cual
		user_send_cmd(target_user, cmd);
	}


cleanup:
	free(prefix);
	free(origin);
	free(target);
	free(serv_name);
	return OK;
}

// ================================================================================================

/*
	PONG message is a reply to ping message.  If parameter <server2> is
	given, this message MUST be forwarded to given target.  The <server>
	parameter is the name of the entity who has responded to PING message
	and generated this message.
*/
static int exec_cmd_PONG(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	char* prefix = NULL;
	char* origin = NULL;
	char* target = NULL;
	char* serv_name = NULL;

	PARSE_PROTECT("PONG", IRCParse_Pong(cmd, &prefix, &origin, &target));

	// No hay origen definido
	if (NULL == origin) {
		IRC_ErrNoOrigin(cmd, sprefix, nick, "No origin specified");
		user_send_cmd(usr, buf);
		goto cleanup;
	}

	// Miramos si el objetivo somos nosotros
	server_get_name(serv, &serv_name);
	if (NULL == target || !strncasecmp(target, serv_name, SERVER_MAX_NAME_LEN)) {
		user_pong(usr);
		goto cleanup;
	}

	// Es caso contrario, (deberia) ser un usuario
	User* target_user = userlist_head(userlist_findByNickname(server_get_userlist(serv), target));
	if (NULL == target_user) {
		IRC_ErrNoSuchServer(buf, sprefix, nick, target);
		user_send_cmd(usr, buf);
	}
	else {
		// Le reenviamos al objetivo el ping tal cual
		user_send_cmd(target_user, cmd);
	}


cleanup:
	free(prefix);
	free(origin);
	free(target);
	free(serv_name);
	return OK;
}

// ================================================================================================

/*
	PRIVMSG is used to send private messages between users, as well as to
	send messages to channels.  <msgtarget> is usually the nickname of
	the recipient of the message, or a channel name.

	The <msgtarget> parameter may also be a host mask (#<mask>) or server
	mask ($<mask>).  In both cases the server will only send the PRIVMSG
	to those who have a server or host matching the mask.  The mask MUST
	have at least 1 (one) "." in it and no wildcards following the last
	".".  This requirement exists to prevent people sending messages to
	"#*" or "$*", which would broadcast to all users.  Wildcards are the
	'*' and '?'  characters.  This extension to the PRIVMSG command is
	only available to operators.
*/

static int exec_cmd_PRIVMSG(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	char* prefix = NULL;
	char* target = NULL;
	char* msg = NULL;
	long opt;

	PARSE_PROTECT("PRIVMSG", IRCParse_Privmsg(cmd, &prefix, &target, &msg));

	// Obtenemos el prefix de verdad
	free(prefix);
	user_get_prefix(usr, &prefix);

	// Es un canal?
	if (strchr("#!&+", target[0])) {
		// Lo buscamos en los canales
		Channel* chan = channellist_head(channellist_findByName(server_get_channellist(serv), target));

		// Envia el mensaje o devuelve un codigo de error
		if (NULL != chan) opt = checksend_message_chan(chan, usr, msg);
		else opt = ERR_CANNOTSENDTOCHAN;

		switch (opt) {
			default: break;
			case ERR_NOTEXTTOSEND:
				IRC_ErrNoTextToSend(buf, sprefix, target);
				user_send_cmd(usr, buf);
				break;
			case ERR_CANNOTSENDTOCHAN:
				IRC_ErrCanNotSendToChan(buf, sprefix, nick, target);
				user_send_cmd(usr, buf);
				break;
			case ERR_NOSUCHNICK:
				IRC_ErrNoSuchNick(buf, sprefix, nick, target);
				user_send_cmd(usr, buf);
				break;
		}
		// case ERR_TOOMANYTARGETS: no admitimos multiples destinatarios
		// ERR_NORECIPIENT	ERR_NOTOPLEVEL ERR_WILDTOPLEVEL
	} else {
		// Estamos enviando a un usuario
		char* awaymsg = NULL;
		User* recv = userlist_head(userlist_findByNickname(server_get_userlist(serv), target));
		//user_send_cmd(User* usr, char* cmd)

		if (NULL != recv) opt = checksend_message_usr(recv, usr, msg);
		else opt = ERR_NOSUCHNICK;

		switch(opt) {
			default: break;
			case ERR_NOTEXTTOSEND:
				IRC_ErrNoTextToSend(buf, sprefix, target);
				user_send_cmd(usr, buf);
				break;
			case ERR_NOSUCHNICK:
				IRC_ErrNoSuchNick(buf, sprefix, nick, target);
				user_send_cmd(usr, buf);
				break;
			case RPL_AWAY:
				user_get_away(recv, &awaymsg);
				IRC_RplAway(buf, sprefix, nick, target, awaymsg);
				user_send_cmd(usr, buf);
				break;
		}
	}

	free(prefix);
	free(target);
	free(msg);
	return OK;
}

static long checksend_message_usr(User* dst, User* src, char* msg) {
	char buf[512];
	char* prefix;
	char* dst_nick;

	if (NULL == dst) return ERR;
	if (NULL == msg) return ERR_NOTEXTTOSEND;

	user_get_prefix(src, &prefix);
	user_get_nick(dst, &dst_nick);

	IRC_Privmsg(buf, prefix, dst_nick, msg);
	user_send_cmd(dst, buf);

	return user_has_flag(dst, 'a') ? RPL_AWAY : OK;
}

static long checksend_message_chan(Channel* dst, User* src, char* msg) {
	long opt;
	char buf[512];
	char* prefix;
	char* chan;

        if(dst == NULL) return ERR;

	opt = channel_can_send_message(dst, src);

	if (OK != opt) return opt;
	if (NULL == msg) return ERR_NOTEXTTOSEND;

	user_get_prefix(src, &prefix);
	channel_get_name(dst, &chan);

	IRC_Privmsg(buf, prefix, chan, msg);
	channel_send_cmd_except(dst, buf, src);

	return OK;
}

// ================================================================================================

/*
	A client session is terminated with a quit message.  The server
	acknowledges this by sending an ERROR message to the client.
*/
static int exec_cmd_QUIT(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	UNUSED(serv);
	char* prefix = NULL;
	char* msg = NULL;

	PARSE_PROTECT("QUIT", IRCParse_Quit(cmd, &prefix, &msg));

	IRC_Error(buf, prefix, msg);
	user_send_cmd(usr, buf);

	userE_die(usr);

	free(prefix);
	free(msg);
	return OK;
}

// ================================================================================================

/*
	The rehash command is an administrative command which can be used by
	an operator to force the server to re-read and process its
	configuration file.
*/
UNIMPLEMENTED_COMMAND(REHASH, "No usamos ficheros de configuracion, comando inutil")

// ================================================================================================

/*
	An operator can use the restart command to force the server to
	restart itself.  This message is optional since it may be viewed as a
	risk to allow arbitrary people to connect to a server as an operator
	and execute this command, causing (at least) a disruption to service.

	The RESTART command MUST always be fully processed by the server to
	which the sending client is connected and MUST NOT be passed onto
	other connected servers.
*/
UNIMPLEMENTED_COMMAND(RESTART, "Comando opcional con riesgos de seguridad graves")

// ================================================================================================

/*
	Extension del RFC (no implementado).
*/
UNIMPLEMENTED_COMMAND(RULES, "Extension del RFC")

// ================================================================================================

/*
	Extension del RFC (no implementado).
*/
UNIMPLEMENTED_COMMAND(SERVER, "Extension del RFC")

// ================================================================================================

/*
	An operator can use the restart command to force the server to
	restart itself.  This message is optional since it may be viewed as a
	risk to allow arbitrary people to connect to a server as an operator
	and execute this command, causing (at least) a disruption to service.

	The RESTART command MUST always be fully processed by the server to
	which the sending client is connected and MUST NOT be passed onto
	other connected servers.
	The <distribution> parameter is used to specify the visibility of a
	service.  The service may only be known to servers which have a name
	matching the distribution.  For a matching server to have knowledge
	of the service, the network path between that server and the server
	on which the service is connected MUST be composed of servers which
	names all match the mask.

	The <type> parameter is currently reserved for future usage.
*/
UNIMPLEMENTED_COMMAND(SERVICE, "Comando de interconexion entre servidores")

// ================================================================================================

/*
	The SERVLIST command is used to list services currently connected to
	the network and visible to the user issuing the command.  The
	optional parameters may be used to restrict the result of the query
	(to matching services names, and services type).
*/
UNIMPLEMENTED_COMMAND(SERVLIST, "Comando de interconexion entre servidores")


// ================================================================================================

/*
	Extension del RFC (no implementado).
*/
UNIMPLEMENTED_COMMAND(SETNAME, "Extension del RFC")

// ================================================================================================

/*
	Extension del RFC (no implementado).
*/
UNIMPLEMENTED_COMMAND(SILENCE, "Extension del RFC")


// ================================================================================================

/*
	The SQUERY command is used similarly to PRIVMSG.  The only difference
	is that the recipient MUST be a service.  This is the only way for a
	text message to be delivered to a service.

	See PRIVMSG for more details on replies and example.
*/
UNIMPLEMENTED_COMMAND(SQUERY, "Comando para la interconexion de servidores")


// ================================================================================================

/*
	The SQUIT command is available only to operators.  It is used to
	disconnect server links.  Also servers can generate SQUIT messages on
	error conditions.  A SQUIT message may also target a remote server
	connection.  In this case, the SQUIT message will simply be sent to
	the remote server without affecting the servers in between the
	operator and the remote server.

	The <comment> SHOULD be supplied by all operators who execute a SQUIT
	for a remote server.  The server ordered to disconnect its peer
	generates a WALLOPS message with <comment> included, so that other
	users may be aware of the reason of this action.
*/
UNIMPLEMENTED_COMMAND(SQUIT, "Comando para la interconexion de servidores")


// ================================================================================================

/*
	The stats command is used to query statistics of certain server.  If
	<query> parameter is omitted, only the end of stats reply is sent
	back.

	A query may be given for any single letter which is only checked by
	the destination server and is otherwise passed on by intermediate
	servers, ignored and unaltered.

	Wildcards are allowed in the <target> parameter.

	Except for the ones below, the list of valid queries is
	implementation dependent.  The standard queries below SHOULD be
	supported by the server:

	l - returns a list of the server's connections, showing how
	  long each connection has been established and the
	  traffic over that connection in Kbytes and messages for
	  each direction;
	m - returns the usage count for each of commands supported
	  by the server; commands for which the usage count is
	  zero MAY be omitted;
	o - returns a list of configured privileged users,
	  operators;
	u - returns a string showing how long the server has been
	  up.

	It is also RECOMMENDED that client and server access configuration be
	published this way.
*/
UNIMPLEMENTED_COMMAND(STATS, "Comando opcional del RFC")

// ================================================================================================

/*
	The SUMMON command can be used to give users who are on a host
	running an IRC server a message asking them to please join IRC.  This
	message is only sent if the target server (a) has SUMMON enabled, (b)
	the user is logged in and (c) the server process can write to the
	user's tty (or similar).

	If no <server> parameter is given it tries to summon <user> from the
	server the client is connected to is assumed as the target.

	If summon is not enabled in a server, it MUST return the
	ERR_SUMMONDISABLED numeric.
*/
static int exec_cmd_SUMMON(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	UNUSED(serv);
	UNUSED(cmd);

	IRC_ErrSummonDisabled(buf, sprefix, nick);
	user_send_cmd(usr, buf);

	return OK;
}

// ================================================================================================

/*
	The time command is used to query local time from the specified
	server. If the <target> parameter is not given, the server receiving
	the command must reply to the query.

	Wildcards are allowed in the <target> parameter.
*/
static int exec_cmd_TIME(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	UNUSED(serv);
	UNUSED(cmd);
	char* prefix;
	char time_buffer[100];
	char* target;

	PARSE_PROTECT("TIME", IRCParse_Time(cmd, &prefix, &target));

	// Si no esta destinado a nosotros, abortamos con error
	if (!switchesP_server_is_target(serv, usr, buf, sprefix, nick, target)) goto cleanup;


	// Obtenemos el tiempo
	{
		time_t     t  = time(NULL);
		struct tm* tm = localtime(&t);
		strftime(time_buffer, sizeof time_buffer, "%FT%TZ", tm); // ISO 8601
	}

	// Y se lo enviamos
	IRC_RplTime(buf, sprefix, nick, time_buffer);
	user_send_cmd(usr, buf);

cleanup:
	free(target);
	free(prefix);
	return OK;
}

// ================================================================================================

/*
	The TOPIC command is used to change or view the topic of a channel.
	The topic for channel <channel> is returned if there is no <topic>
	given.  If the <topic> parameter is present, the topic for that
	channel will be changed, if this action is allowed for the user
	requesting it.  If the <topic> parameter is an empty string, the
	topic for that channel will be removed.
*/
static int exec_cmd_TOPIC(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	char* channel_name = NULL;
	char* topic = NULL;
	char* prefix = NULL;

	PARSE_PROTECT("TOPIC", IRCParse_Topic(cmd, &prefix, &channel_name, &topic));

	Channel* channel = channellist_head(channellist_findByName(server_get_channellist(serv), channel_name));
	if (NULL == channel) {
		IRC_ErrNotOnChannel(buf, sprefix, nick, nick, channel_name);
		user_send_cmd(usr, buf);
		goto cleanup;
	}


	// Ponemos o leemos el topic, dependiendo si el user lo proporciono
	if (NULL != topic) {
		long opt = channel_set_topic(channel, topic, usr);
		switch(opt) {
			case ERR_CHANOPRIVSNEEDED:
				IRC_ErrChanOPrivsNeeded(buf, sprefix, nick, channel_name);
				user_send_cmd(usr, buf);
				return ERR;
			case ERR_NEEDMOREPARAMS:
				IRC_ErrNeedMoreParams(buf, sprefix, nick, "TOPIC");
				user_send_cmd(usr, buf);
				return ERR;
			case OK:
				break;
		}
	}
	else {
		channel_get_topic(channel, &topic);
	}


	// Enviamos la respuesta adecuada
	if (NULL != topic) IRC_RplTopic(buf, sprefix, nick, channel_name, topic);
	else               IRC_RplNoTopic(buf, sprefix, nick, channel_name, topic);

	channel_send_cmd(channel, buf);


cleanup:
	free(channel_name);
	free(topic);
	free(prefix);
	return OK;
}

// ================================================================================================

/*
	TRACE command is used to find the route to specific server and
	information about its peers.  Each server that processes this command
	MUST report to the sender about it.  The replies from pass-through
	links form a chain, which shows route to destination.  After sending
	this reply back, the query MUST be sent to the next server until
	given <target> server is reached.

	TRACE command is used to find the route to specific server.  Each
	server that processes this message MUST tell the sender about it by
	sending a reply indicating it is a pass-through link, forming a chain
	of replies.  After sending this reply back, it MUST then send the
	TRACE message to the next server until given server is reached.  If
	the <target> parameter is omitted, it is RECOMMENDED that TRACE
	command sends a message to the sender telling which servers the local
	server has direct connection to.

	If the destination given by <target> is an actual server, the
	destination server is REQUIRED to report all servers, services and
	operators which are connected to it; if the command was issued by an
	operator, the server MAY also report all users which are connected to
	it.  If the destination given by <target> is a nickname, then only a
	reply for that nickname is given.  If the <target> parameter is
	omitted, it is RECOMMENDED that the TRACE command is parsed as
	targeted to the processing server.

	Wildcards are allowed in the <target> parameter.
*/
UNIMPLEMENTED_COMMAND(TRACE, "Comando para la interconexion de servidores")

// ================================================================================================

/*
	Extension del RFC (no implementado).
*/
UNIMPLEMENTED_COMMAND(UHNAMES, "Extension del RFC")

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
int exec_cmd_USER(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	char* pre = NULL;
	char* user_name = NULL;
	char* mode = NULL;
	char* realname = NULL;
	char* hostname = NULL;
	char* servername = NULL;
	UNUSED(nick);

	// Primero probamos con RFC2812. Parameters: <user> <mode> <unused> <realname>
	if (0 > IRCParse_User(cmd, &pre, &user_name, &mode, &realname)) {
		// Si no funciona, probamos con el RFC1459. Parameters: <username> <hostname> <servername> <realname>
		PARSE_PROTECT("USER", IRCParse_User1459(cmd, &pre, &user_name, &hostname, &servername, &realname));
	}

	UserList usr_using = userlist_findByUsername(server_get_userlist(serv), user_name);
	if (NULL != *usr_using) {
		// Ya esta registrado? Enviamos error
		IRC_ErrAlreadyRegistred(buf, sprefix, user_name);
		user_send_cmd(usr, buf);
	}
	else {
		// En caso contrario lo registramos
		user_set_name(usr, user_name);
		user_set_rname(usr, realname);
		user_init_prefix(usr);
		server_add_user(serv, usr);

		// Le enviamos cositas que se envian al registrarse
		char* host;
		user_get_host(usr, &host);

		IRC_RplWelcome(buf, sprefix, nick, nick, user_name, host);
		user_send_cmd(usr, buf);
		//IRC_RplYourHost(char *command, char *prefix, char *nick, char *servername, char *versionname)
		//user_send_cmd(usr, buf);
		//IRCParse_RplCreated(char *strin, char **prefix, char **nick, char **timedate, char **msg)
		//user_send_cmd(usr, buf);
		//IRCParse_RplMyInfo(char *strin, char **prefix, char **nick, char **servername, char **version, char **availableusermodes, char **availablechannelmodes, char **added)
		//user_send_cmd(usr, buf);

		action_switch(serv, usr, "LUSERS");
		action_switch(serv, usr, "MOTD");
		action_switch(serv, usr, "INFO");
		action_switch(serv, usr, "VERSION");
		action_switch(serv, usr, "NAMES");
		action_switch(serv, usr, "ADMIN");
		action_switch(serv, usr, "LIST");
	}

	free(pre);
	free(user_name);
	free(mode);
	free(realname);
	free(hostname);
	free(servername);
	return OK;
}

// ================================================================================================

/*
	The USERHOST command takes a list of up to 5 nicknames, each
	separated by a space character and returns a list of information
	about each nickname that it found.  The returned list has each reply
	separated by a space.
*/
static int exec_cmd_USERHOST(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	char* prefix = NULL;
	char* nick0 = NULL;
	char* nick1 = NULL;
	char* nick2 = NULL;
	char* nick3 = NULL;
	char* nick4 = NULL;

	PARSE_PROTECT("USERHOST", IRCParse_UserHost(cmd, &prefix, &nick0, &nick1, &nick2, &nick3, &nick4));

	User* usr2 = userlist_head(userlist_findByNickname(server_get_userlist(serv), nick0));
	if (NULL != usr2) {
		char* host;
		char uh_buf[80] = "*=*";
		user_get_host(usr2, &host);
		snprintf(uh_buf, sizeof uh_buf, "%s=%s", nick0, host);
		free(host);

		IRC_RplUserHost(buf, sprefix, nick, uh_buf, NULL);
		user_send_cmd(usr, buf);
	} else {
		IRC_RplUserHost(buf, sprefix, nick, NULL);
		user_send_cmd(usr, buf);
	}

	free(prefix);
	free(nick0);
	free(nick1);
	free(nick2);
	free(nick3);
	free(nick4);
	return OK;
}

// ================================================================================================

/*
	Extension del RFC (no implementado).
*/
UNIMPLEMENTED_COMMAND(USERIP, "Extension del RFC")

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
static int exec_cmd_USERS(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	UNUSED(serv);
	UNUSED(cmd);

	// Tiene serias implicaciones de seguridad
	IRC_ErrUsersDisabled(buf, sprefix, nick);
	user_send_cmd(usr, buf);

	return OK;
}

// ================================================================================================

/*
	The VERSION command is used to query the version of the server
	program.  An optional parameter <target> is used to query the version
	of the server program which a client is not directly connected to.

	Wildcards are allowed in the <target> parameter.
*/
static int exec_cmd_VERSION(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	UNUSED(usr);
	char* prefix = NULL;
	char* target = NULL;
	char* serv_name = NULL;

	PARSE_PROTECT("VERSION", IRCParse_Version(cmd, &prefix, &target));

	// Si no esta destinado a nosotros, abortamos con error
	if (!switchesP_server_is_target(serv, usr, buf, sprefix, nick, target)) goto cleanup;

	server_get_name(serv, &serv_name);
	IRC_RplVersion(buf, sprefix, nick, 0, serv_name, PACKAGE_STRING); // config.h
	user_send_cmd(usr, buf);

cleanup:
	free(prefix);
	free(target);
	free(serv_name);
	return OK;
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
static int exec_cmd_WALLOPS(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	char* prefix = NULL;
	char* msg = NULL;

	PARSE_PROTECT("WALLOPS", IRCParse_Wallops(cmd, &prefix, &msg));

	// Obtenemos el prefix de verdad
	free(prefix);
	user_get_prefix(usr, &prefix);

	UserList ulist = server_get_userlist(serv);

	// Construimos el mensaje de wallops
	IRC_Wallops(buf, prefix, msg);

	// Iteramos la lista de todos los usuarios del servidor
	while (1) {
		User* target = userlist_head(ulist);
		if (NULL == target) break;
		// Solo enviamos el mensaje (tal cual) si tienen modo +w
		if (user_has_flag(target, 'w')) user_send_cmd(target, buf);
		ulist = userlist_tail(ulist);
	}

	free(prefix);
	free(msg);
	return OK;
}

// ================================================================================================

/*
	Extension del RFC (no implementado).
*/
UNIMPLEMENTED_COMMAND(WATCH, "Extension del RFC")

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
static int exec_cmd_WHO(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	char* prefix;
	char* mask;
	char* op;
	char** namelist = NULL;


	PARSE_PROTECT("WHO", IRCParse_Who(cmd, &prefix, &mask, &op));

	if (NULL == mask) goto cleanup;

	// Obtenemos la lista de usuarios
	Channel* chan = channellist_head(channellist_findByName(server_get_channellist(serv), mask));
	if (NULL == chan) goto cleanup;
	channel_get_user_names(chan, 0, &namelist);

	int i = 0;
	while (NULL != namelist[i]) {
		User* other = userlist_head(userlist_findByUsername(server_get_userlist(serv), namelist[i]));
		char* nickname = NULL;
		char* realname = NULL;
		char* hostname = NULL;
		char* mode = NULL;
		user_get_nick(other, &nickname);
		user_get_rname(other, &realname);
		user_get_host(other, &hostname);
		mode = switchesP_channel_user_mode(chan, other);

		IRC_RplWhoReply(buf, sprefix, nick, mask, namelist[i], hostname, sprefix, nickname, mode, 0, realname);
		user_send_cmd(usr, buf);

		free(nickname);
		free(realname);
		free(hostname);
		free(mode);
		free(namelist[i]);
		i++;
	}

	free(namelist);

	IRC_RplEndOfWho(buf, sprefix, nick, mask);
	user_send_cmd(usr, buf);

cleanup:
	free(prefix);
	free(mask);
	free(op);
	return OK;
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
static int exec_cmd_WHOIS(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	UNUSED(serv);
	char* prefix = NULL;
	char* target = NULL;
	char* maskarray = NULL;

	PARSE_PROTECT("WHOIS", IRCParse_Whois(cmd, &prefix, &target, &maskarray));


	free(prefix);
	free(target);
	free(maskarray);
	return OK;
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
static int exec_cmd_WHOWAS(Server* serv, User* usr, char* buf, char* sprefix, char* nick, char* cmd) {
	char* prefix = NULL;
	char* nicks_str = NULL;
	int max_count;
	char* target = NULL;
	char** nick_list = NULL;
	int i = 0;

	PARSE_PROTECT("WHOWAS", IRCParse_Whowas(cmd, &prefix, &nicks_str, &max_count, &target));

	parse_lists(nicks_str, &nick_list);

	UserList ulist = server_get_disconnectlist(serv);

	// Iteramos por todos los nicknames
	while (nick_list[i] != NULL) {
		char* dc_nick = nick_list[i];

		// Y por cada usuario, lo buscamos en la lista de desconectados
		User* dc_user;
		dc_user = userlist_head(userlist_findByNickname(ulist, dc_nick));

		// Si no esta, liberamos su dc_nick y continuamos
		if (NULL == dc_user) {
			free(dc_nick);
			i++;
			continue;
		}

		// Encontrado! Hallamos sus datos y enviamos un mensaje
		char* dc_name = NULL;
		char* dc_host = NULL;
		char* dc_rname = NULL;

		user_get_name(dc_user, &dc_name);
		user_get_host(dc_user, &dc_host);
		user_get_rname(dc_user, &dc_rname);

		// Enviamos los datos
		IRC_RplWhoWasUser(buf, sprefix, nick, dc_nick, dc_name, dc_host, dc_rname);
		user_send_cmd(usr, buf);

		// Y un fin de la lista
		IRC_RplEndOfWhoWas(buf, sprefix, nick, nick);
		user_send_cmd(usr, buf);

		// Liberacion
		free(dc_name);
		free(dc_host);
		free(dc_rname);
		free(dc_nick);
		i++;
	}

	free(prefix);
	free(nicks_str);
	free(target);
	free(nick_list);
	return OK;
}

// ================================================================================================
int action_switch(Server* serv, User* usr, char* cmd) {
	char buf[IRC_MAX_CMD_LEN + 1];
	char* sprefix;
	char* nick;
	server_get_name(serv, &sprefix);
	user_get_nick(usr, &nick);

// Definimos una macro para el case que imprima el mensaje
#define CMD_CASE(CMD)                                           	\
case CMD:                                                       	\
    LOG("[Usuario \%s] envio [Comando %s]", nick, cmd);         	\
    return exec_cmd_ ## CMD(serv, usr, buf, sprefix, nick, cmd);	//

	switch (IRC_CommandQuery(cmd)) {
		default	: return ERR; // Aqui habria que dar un error
		CMD_CASE(ADMIN   );
		CMD_CASE(AWAY    );
		CMD_CASE(CNOTICE );
		CMD_CASE(CONNECT );
		CMD_CASE(CPRIVMSG);
		CMD_CASE(DIE     );
		CMD_CASE(ENCAP   );
		CMD_CASE(ERROR   );
		CMD_CASE(HELP    );
		CMD_CASE(INFO    );
		CMD_CASE(INVITE  );
		CMD_CASE(ISON    );
		CMD_CASE(JOIN    );
		CMD_CASE(KICK    );
		CMD_CASE(KILL    );
		CMD_CASE(KNOCK   );
		CMD_CASE(LINKS   );
		CMD_CASE(LIST    );
		CMD_CASE(LUSERS  );
		CMD_CASE(MODE    );
		CMD_CASE(MOTD    );
		CMD_CASE(NAMES   );
		CMD_CASE(NAMESX  );
		CMD_CASE(NICK    );
		CMD_CASE(NOTICE  );
		CMD_CASE(OPER    );
		CMD_CASE(PART    );
		CMD_CASE(PASS    );
		CMD_CASE(PING    );
		CMD_CASE(PONG    );
		CMD_CASE(PRIVMSG );
		CMD_CASE(QUIT    );
		CMD_CASE(REHASH  );
		CMD_CASE(RESTART );
		CMD_CASE(RULES   );
		CMD_CASE(SERVER  );
		CMD_CASE(SERVICE );
		CMD_CASE(SERVLIST);
		CMD_CASE(SETNAME );
		CMD_CASE(SILENCE );
		CMD_CASE(SQUERY  );
		CMD_CASE(SQUIT   );
		CMD_CASE(STATS   );
		CMD_CASE(SUMMON  );
		CMD_CASE(TIME    );
		CMD_CASE(TOPIC   );
		CMD_CASE(TRACE   );
		CMD_CASE(UHNAMES );
		CMD_CASE(USER    );
		CMD_CASE(USERHOST);
		CMD_CASE(USERIP  );
		CMD_CASE(USERS   );
		CMD_CASE(VERSION );
		CMD_CASE(WALLOPS );
		CMD_CASE(WATCH   );
		CMD_CASE(WHO     );
		CMD_CASE(WHOIS   );
		CMD_CASE(WHOWAS  );
	}
	free(sprefix);
	free(nick);
	return OK;
}

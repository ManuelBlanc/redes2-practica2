

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


int action_switch(int what) {
	switch (what) {
		default: break;
		case RPL_ADMINEMAIL     	: printf("%s\n", "Reply RPL_ADMINEMAIL recibido!"     	); break;
		case RPL_ADMINLOC1      	: printf("%s\n", "Reply RPL_ADMINLOC1 recibido!"      	); break;
		case RPL_ADMINLOC2      	: printf("%s\n", "Reply RPL_ADMINLOC2 recibido!"      	); break;
		case RPL_ADMINME        	: printf("%s\n", "Reply RPL_ADMINME recibido!"        	); break;
		case RPL_AWAY           	: printf("%s\n", "Reply RPL_AWAY recibido!"           	); break;
		case RPL_BANLIST        	: printf("%s\n", "Reply RPL_BANLIST recibido!"        	); break;
		case RPL_BOUNCE         	: printf("%s\n", "Reply RPL_BOUNCE recibido!"         	); break;
		case RPL_CHANNELMODEIS  	: printf("%s\n", "Reply RPL_CHANNELMODEIS recibido!"  	); break;
		case RPL_CHANNELURL     	: printf("%s\n", "Reply RPL_CHANNELURL recibido!"     	); break;
		case RPL_CREATED        	: printf("%s\n", "Reply RPL_CREATED recibido!"        	); break;
		case RPL_CREATIONTIME   	: printf("%s\n", "Reply RPL_CREATIONTIME recibido!"   	); break;
		case RPL_ENDOFBANLIST   	: printf("%s\n", "Reply RPL_ENDOFBANLIST recibido!"   	); break;
		case RPL_ENDOFEXCEPTLIST	: printf("%s\n", "Reply RPL_ENDOFEXCEPTLIST recibido!"	); break;
		case RPL_ENDOFINFO      	: printf("%s\n", "Reply RPL_ENDOFINFO recibido!"      	); break;
		case RPL_ENDOFINVITELIST	: printf("%s\n", "Reply RPL_ENDOFINVITELIST recibido!"	); break;
		case RPL_ENDOFLINKS     	: printf("%s\n", "Reply RPL_ENDOFLINKS recibido!"     	); break;
		case RPL_ENDOFMOTD      	: printf("%s\n", "Reply RPL_ENDOFMOTD recibido!"      	); break;
		case RPL_ENDOFNAMES     	: printf("%s\n", "Reply RPL_ENDOFNAMES recibido!"     	); break;
		case RPL_ENDOFSTATS     	: printf("%s\n", "Reply RPL_ENDOFSTATS recibido!"     	); break;
		case RPL_ENDOFUSERS     	: printf("%s\n", "Reply RPL_ENDOFUSERS recibido!"     	); break;
		case RPL_ENDOFWHO       	: printf("%s\n", "Reply RPL_ENDOFWHO recibido!"       	); break;
		case RPL_ENDOFWHOIS     	: printf("%s\n", "Reply RPL_ENDOFWHOIS recibido!"     	); break;
		case RPL_ENDOFWHOWAS    	: printf("%s\n", "Reply RPL_ENDOFWHOWAS recibido!"    	); break;
		case RPL_EXCEPTLIST     	: printf("%s\n", "Reply RPL_EXCEPTLIST recibido!"     	); break;
		case RPL_GLOBALUSERS    	: printf("%s\n", "Reply RPL_GLOBALUSERS recibido!"    	); break;
		case RPL_INFO           	: printf("%s\n", "Reply RPL_INFO recibido!"           	); break;
		case RPL_INVITELIST     	: printf("%s\n", "Reply RPL_INVITELIST recibido!"     	); break;
		case RPL_INVITING       	: printf("%s\n", "Reply RPL_INVITING recibido!"       	); break;
		case RPL_ISON           	: printf("%s\n", "Reply RPL_ISON recibido!"           	); break;
		case RPL_LINKS          	: printf("%s\n", "Reply RPL_LINKS recibido!"          	); break;
		case RPL_LIST           	: printf("%s\n", "Reply RPL_LIST recibido!"           	); break;
		case RPL_LISTEND        	: printf("%s\n", "Reply RPL_LISTEND recibido!"        	); break;
		case RPL_LISTSTART      	: printf("%s\n", "Reply RPL_LISTSTART recibido!"      	); break;
		case RPL_LOCALUSERS     	: printf("%s\n", "Reply RPL_LOCALUSERS recibido!"     	); break;
		case RPL_LUSERCHANNELS  	: printf("%s\n", "Reply RPL_LUSERCHANNELS recibido!"  	); break;
		case RPL_LUSERCLIENT    	: printf("%s\n", "Reply RPL_LUSERCLIENT recibido!"    	); break;
		case RPL_LUSERME        	: printf("%s\n", "Reply RPL_LUSERME recibido!"        	); break;
		case RPL_LUSEROP        	: printf("%s\n", "Reply RPL_LUSEROP recibido!"        	); break;
		case RPL_LUSERUNKNOWN   	: printf("%s\n", "Reply RPL_LUSERUNKNOWN recibido!"   	); break;
		case RPL_MOTD           	: printf("%s\n", "Reply RPL_MOTD recibido!"           	); break;
		case RPL_MOTDSTART      	: printf("%s\n", "Reply RPL_MOTDSTART recibido!"      	); break;
		case RPL_MYINFO         	: printf("%s\n", "Reply RPL_MYINFO recibido!"         	); break;
		case RPL_NAMREPLY       	: printf("%s\n", "Reply RPL_NAMREPLY recibido!"       	); break;
		case RPL_NOTOPIC        	: printf("%s\n", "Reply RPL_NOTOPIC recibido!"        	); break;
		case RPL_NOUSERS        	: printf("%s\n", "Reply RPL_NOUSERS recibido!"        	); break;
		case RPL_NOWAWAY        	: printf("%s\n", "Reply RPL_NOWAWAY recibido!"        	); break;
		case RPL_REHASHING      	: printf("%s\n", "Reply RPL_REHASHING recibido!"      	); break;
		case RPL_SERVLIST       	: printf("%s\n", "Reply RPL_SERVLIST recibido!"       	); break;
		case RPL_SERVLISTEND    	: printf("%s\n", "Reply RPL_SERVLISTEND recibido!"    	); break;
		case RPL_STATSCOMMANDS  	: printf("%s\n", "Reply RPL_STATSCOMMANDS recibido!"  	); break;
		case RPL_STATSLINKINFO  	: printf("%s\n", "Reply RPL_STATSLINKINFO recibido!"  	); break;
		case RPL_STATSOLINE     	: printf("%s\n", "Reply RPL_STATSOLINE recibido!"     	); break;
		case RPL_STATSUPTIME    	: printf("%s\n", "Reply RPL_STATSUPTIME recibido!"    	); break;
		case RPL_SUMMONING      	: printf("%s\n", "Reply RPL_SUMMONING recibido!"      	); break;
		case RPL_TIME           	: printf("%s\n", "Reply RPL_TIME recibido!"           	); break;
		case RPL_TOPIC          	: printf("%s\n", "Reply RPL_TOPIC recibido!"          	); break;
		case RPL_TOPICWHOTIME   	: printf("%s\n", "Reply RPL_TOPICWHOTIME recibido!"   	); break;
		case RPL_TRACECLASS     	: printf("%s\n", "Reply RPL_TRACECLASS recibido!"     	); break;
		case RPL_TRACECONNECT   	: printf("%s\n", "Reply RPL_TRACECONNECT recibido!"   	); break;
		case RPL_TRACECONNECTING	: printf("%s\n", "Reply RPL_TRACECONNECTING recibido!"	); break;
		case RPL_TRACEEND       	: printf("%s\n", "Reply RPL_TRACEEND recibido!"       	); break;
		case RPL_TRACEHANDSHAKE 	: printf("%s\n", "Reply RPL_TRACEHANDSHAKE recibido!" 	); break;
		case RPL_TRACELINK      	: printf("%s\n", "Reply RPL_TRACELINK recibido!"      	); break;
		case RPL_TRACELOG       	: printf("%s\n", "Reply RPL_TRACELOG recibido!"       	); break;
		case RPL_TRACENEWTYPE   	: printf("%s\n", "Reply RPL_TRACENEWTYPE recibido!"   	); break;
		case RPL_TRACEOPERATOR  	: printf("%s\n", "Reply RPL_TRACEOPERATOR recibido!"  	); break;
		case RPL_TRACESERVER    	: printf("%s\n", "Reply RPL_TRACESERVER recibido!"    	); break;
		case RPL_TRACESERVICE   	: printf("%s\n", "Reply RPL_TRACESERVICE recibido!"   	); break;
		case RPL_TRACEUNKNOWN   	: printf("%s\n", "Reply RPL_TRACEUNKNOWN recibido!"   	); break;
		case RPL_TRACEUSER      	: printf("%s\n", "Reply RPL_TRACEUSER recibido!"      	); break;
		case RPL_TRYAGAIN       	: printf("%s\n", "Reply RPL_TRYAGAIN recibido!"       	); break;
		case RPL_UMODEIS        	: printf("%s\n", "Reply RPL_UMODEIS recibido!"        	); break;
		case RPL_UNAWAY         	: printf("%s\n", "Reply RPL_UNAWAY recibido!"         	); break;
		case RPL_UNIQOPIS       	: printf("%s\n", "Reply RPL_UNIQOPIS recibido!"       	); break;
		case RPL_USERHOST       	: printf("%s\n", "Reply RPL_USERHOST recibido!"       	); break;
		case RPL_USERS          	: printf("%s\n", "Reply RPL_USERS recibido!"          	); break;
		case RPL_USERSSTART     	: printf("%s\n", "Reply RPL_USERSSTART recibido!"     	); break;
		case RPL_VERSION        	: printf("%s\n", "Reply RPL_VERSION recibido!"        	); break;
		case RPL_WELCOME        	: printf("%s\n", "Reply RPL_WELCOME recibido!"        	); break;
		case RPL_WHOISCHANNELS  	: printf("%s\n", "Reply RPL_WHOISCHANNELS recibido!"  	); break;
		case RPL_WHOISIDLE      	: printf("%s\n", "Reply RPL_WHOISIDLE recibido!"      	); break;
		case RPL_WHOISOPERATOR  	: printf("%s\n", "Reply RPL_WHOISOPERATOR recibido!"  	); break;
		case RPL_WHOISSERVER    	: printf("%s\n", "Reply RPL_WHOISSERVER recibido!"    	); break;
		case RPL_WHOISUSER      	: printf("%s\n", "Reply RPL_WHOISUSER recibido!"      	); break;
		case RPL_WHOREPLY       	: printf("%s\n", "Reply RPL_WHOREPLY recibido!"       	); break;
		case RPL_WHOWASUSER     	: printf("%s\n", "Reply RPL_WHOWASUSER recibido!"     	); break;
		case RPL_YOUREOPER      	: printf("%s\n", "Reply RPL_YOUREOPER recibido!"      	); break;
		case RPL_YOURESERVICE   	: printf("%s\n", "Reply RPL_YOURESERVICE recibido!"   	); break;
		case RPL_YOURHOST       	: printf("%s\n", "Reply RPL_YOURHOST recibido!"       	); break;
		case RPL_YOURID         	: printf("%s\n", "Reply RPL_YOURID recibido!"         	); break;
	}
}


int action_switch(int what) {
	switch (what) {
		default: break;
		case ERR_ALREADYREGISTRED 	: printf("%s\n", "ERR_ALREADYREGISTRED" 	); break;
		case ERR_BADCHANMASK      	: printf("%s\n", "ERR_BADCHANMASK"      	); break;
		case ERR_BADCHANNELKEY    	: printf("%s\n", "ERR_BADCHANNELKEY"    	); break;
		case ERR_BADMASK          	: printf("%s\n", "ERR_BADMASK"          	); break;
		case ERR_BANLISTFULL      	: printf("%s\n", "ERR_BANLISTFULL"      	); break;
		case ERR_BANNEDFROMCHAN   	: printf("%s\n", "ERR_BANNEDFROMCHAN"   	); break;
		case ERR_CANNOTSENDTOCHAN 	: printf("%s\n", "ERR_CANNOTSENDTOCHAN" 	); break;
		case ERR_CANTKILLSERVER   	: printf("%s\n", "ERR_CANTKILLSERVER"   	); break;
		case ERR_CHANNELISFULL    	: printf("%s\n", "ERR_CHANNELISFULL"    	); break;
		case ERR_CHANOPRIVSNEEDED 	: printf("%s\n", "ERR_CHANOPRIVSNEEDED" 	); break;
		case ERR_ERRONEUSNICKNAME 	: printf("%s\n", "ERR_ERRONEUSNICKNAME" 	); break;
		case ERR_FILEERROR        	: printf("%s\n", "ERR_FILEERROR"        	); break;
		case ERR_INVITEONLYCHAN   	: printf("%s\n", "ERR_INVITEONLYCHAN"   	); break;
		case ERR_KEYSET           	: printf("%s\n", "ERR_KEYSET"           	); break;
		case ERR_NEEDMOREPARAMS   	: printf("%s\n", "ERR_NEEDMOREPARAMS"   	); break;
		case ERR_NICKCOLLISION    	: printf("%s\n", "ERR_NICKCOLLISION"    	); break;
		case ERR_NICKNAMEINUSE    	: printf("%s\n", "ERR_NICKNAMEINUSE"    	); break;
		case ERR_NOADMININFO      	: printf("%s\n", "ERR_NOADMININFO"      	); break;
		case ERR_NOCHANMODES      	: printf("%s\n", "ERR_NOCHANMODES"      	); break;
		case ERR_NOLOGIN          	: printf("%s\n", "ERR_NOLOGIN"          	); break;
		case ERR_NOMOTD           	: printf("%s\n", "ERR_NOMOTD"           	); break;
		case ERR_NONICKNAMEGIVEN  	: printf("%s\n", "ERR_NONICKNAMEGIVEN"  	); break;
		case ERR_NOOPERHOST       	: printf("%s\n", "ERR_NOOPERHOST"       	); break;
		case ERR_NOORIGIN         	: printf("%s\n", "ERR_NOORIGIN"         	); break;
		case ERR_NOPERMFORHOST    	: printf("%s\n", "ERR_NOPERMFORHOST"    	); break;
		case ERR_NOPRIVILEGES     	: printf("%s\n", "ERR_NOPRIVILEGES"     	); break;
		case ERR_NORECIPIENT      	: printf("%s\n", "ERR_NORECIPIENT"      	); break;
		case ERR_NOSUCHCHANNEL    	: printf("%s\n", "ERR_NOSUCHCHANNEL"    	); break;
		case ERR_NOSUCHNICK       	: printf("%s\n", "ERR_NOSUCHNICK"       	); break;
		case ERR_NOSUCHSERVER     	: printf("%s\n", "ERR_NOSUCHSERVER"     	); break;
		case ERR_NOSUCHSERVICE    	: printf("%s\n", "ERR_NOSUCHSERVICE"    	); break;
		case ERR_NOTEXTTOSEND     	: printf("%s\n", "ERR_NOTEXTTOSEND"     	); break;
		case ERR_NOTONCHANNEL     	: printf("%s\n", "ERR_NOTONCHANNEL"     	); break;
		case ERR_NOTOPLEVEL       	: printf("%s\n", "ERR_NOTOPLEVEL"       	); break;
		case ERR_NOTREGISTERED    	: printf("%s\n", "ERR_NOTREGISTERED"    	); break;
		case ERR_PASSWDMISMATCH   	: printf("%s\n", "ERR_PASSWDMISMATCH"   	); break;
		case ERR_RESTRICTED       	: printf("%s\n", "ERR_RESTRICTED"       	); break;
		case ERR_SUMMONDISABLED   	: printf("%s\n", "ERR_SUMMONDISABLED"   	); break;
		case ERR_TOOMANYCHANNELS  	: printf("%s\n", "ERR_TOOMANYCHANNELS"  	); break;
		case ERR_TOOMANYMATCHES   	: printf("%s\n", "ERR_TOOMANYMATCHES"   	); break;
		case ERR_TOOMANYTARGETS   	: printf("%s\n", "ERR_TOOMANYTARGETS"   	); break;
		case ERR_UMODEUNKNOWNFLAG 	: printf("%s\n", "ERR_UMODEUNKNOWNFLAG" 	); break;
		case ERR_UNAVAILRESOURCE  	: printf("%s\n", "ERR_UNAVAILRESOURCE"  	); break;
		case ERR_UNIQOPPRIVSNEEDED	: printf("%s\n", "ERR_UNIQOPPRIVSNEEDED"	); break;
		case ERR_UNKNOWNCOMMAND   	: printf("%s\n", "ERR_UNKNOWNCOMMAND"   	); break;
		case ERR_UNKNOWNMODE      	: printf("%s\n", "ERR_UNKNOWNMODE"      	); break;
		case ERR_USERNOTINCHANNEL 	: printf("%s\n", "ERR_USERNOTINCHANNEL" 	); break;
		case ERR_USERONCHANNEL    	: printf("%s\n", "ERR_USERONCHANNEL"    	); break;
		case ERR_USERSDISABLED    	: printf("%s\n", "ERR_USERSDISABLED"    	); break;
		case ERR_USERSDONTMATCH   	: printf("%s\n", "ERR_USERSDONTMATCH"   	); break;
		case ERR_WASNOSUCHNICK    	: printf("%s\n", "ERR_WASNOSUCHNICK"    	); break;
		case ERR_WILDTOPLEVEL     	: printf("%s\n", "ERR_WILDTOPLEVEL"     	); break;
		case ERR_YOUREBANNEDCREEP 	: printf("%s\n", "ERR_YOUREBANNEDCREEP" 	); break;
		case ERR_YOUWILLBEBANNED  	: printf("%s\n", "ERR_YOUWILLBEBANNED"  	); break;
	}
}

int serverrcv_cmd(User* usr, char* str){
	char* cmd;
	int more_commands = 1;

	while (more_commands) {
		switch (IRC_UnPipelineCommands(str, &cmd)) {
		case IRC_ENDPIPE:
			usr->buffer_recv[0] = '\0';
			more_commands = 0;
		case IRC_OK:
			action_switch(usr, cmd);
			str = NULL;
			break;

		case IRC_EOP:
			memset(usr->buffer_recv, sizeof(usr->buffer_recv), 0);
			strncpy(usr->buffer_recv, cmd, sizeof(usr->buffer_recv));
			return OK;
		}
	}
	// Vaciamos el buffer
	return OK;
}





static char* namechannel_skip_colon(char* channel) {
	return *channel == ':' ? channel+1 : channel;
}

int serverrcv_privmsg(Server* serv, User* usr, const char* str) {
	char nick[IRC_MAX_NICK_LEN + 1];
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
		Channel* chan = channellist_findByName(serv->chan, target);
		//Envia el mensaje o devuelve un codigo de error
		if (NULL != chan) opt = channel_send_message(chan, usr, msg);
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
		User* recv = userlist_findByName(serv->usrs, target);
		if (NULL != recv) opt = user_send_message(recv, nick, msg);
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
				char* awaymsg = user_get_away(recv, &awaymsg);
				IRC_RplAway(buf, prefix, nick, target, awaymsg);
				user_send_cmd(usr, buf);
				break;
		}
	}
	return OK;
}

int serverrcv_mode(Server* serv, User* usr, const char* str) {
	/*char nick[IRC_MAX_NICK_LEN + 1];
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

	}
	return OK;*/
}

int serverrcv_quit(Server* serv, User* usr, const char* str) {
/*sacar los semaforos fuera delparser.c*/
	/* down semaforo*/
	userlist_extract(&usr);/*mal argumento*/
	/* up semaforo*/
	channel_sendf(chan, "MODE %s +%c", user_get_name(usr), mode);

}

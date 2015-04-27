/** G-2301-05-P3-clientcmd.c
 * @author AAlonso <ana.alonsoa@estudiante.uam.es>
 * @author MBlanc <manuel.blanc@estudiante.uam.es>
 */

/* std */
#include <stdarg.h>
/* posix */
#include <strings.h>
/* redes2 */
#include <redes2/chat.h>
#include <redes2/irc.h>
/* usr */
#include "G-2301-05-P3-chat.h"
#include "G-2301-05-P3-client.h"
#include "G-2301-05-P3-clientcmd.h"
#include "G-2301-05-P3-clientfile.h"
#include "G-2301-05-P3-util.h"

#define USRPARS(status)	do if ((status) < 0) { LOG("Comando mal formado."       	); return ERR; } while(0);
#define MAKECMD(status)	do if ((status) < 0) { LOG("Error al formar el comando."	); return ERR; } while(0);

static int clientcmd_part(Client* cli, char* cmdstr); // Para que no de error en el leave


static int clientcmd_admin(Client* cli, char* cmdstr) {
	char* server;
	if (IRCUserParse_Admin(cmdstr, &server) < 0) {
		clientcmd(cli, "/help admin");
		return ERR;
	}
	MAKECMD(IRC_Admin(cli->cmd, cli->pre, server));
	client_send_cmd(cli);
	free(server);
	return OK;
}


static int clientcmd_away(Client* cli, char* cmdstr) {
	char* reason;
	if (IRCUserParse_Away(cmdstr, &reason) < 0) {
		clientcmd(cli, "/help away");
		return ERR;
	}
	MAKECMD(IRC_Away(cli->cmd, cli->pre, reason));
	client_send_cmd(cli);
	free(reason);
	return OK;
}


static int clientcmd_botmotd(Client* cli, char* cmdstr) {
	char* server;
	if (IRCUserParse_BotMotd(cmdstr, &server) < 0) {
		clientcmd(cli, "/help botmotd");
		return ERR;
	}
	error_text(0, "Comando no implementado");
	free(server);
	return OK;
}


static int clientcmd_chat(Client* cli, char* cmdstr) {
	char* msg;
	if (IRCUserParse_Chat(cmdstr, &msg) < 0) {
		clientcmd(cli, "/help chat");
		return ERR;
	}
	error_text(0, "Comando no implementado");
	free(msg);
	return OK;
}


static int clientcmd_ctcp(Client* cli, char* cmdstr) {
	char* msg;
	if (IRCUserParse_CTCP(cmdstr, &msg) < 0) {
		clientcmd(cli, "/help ctcp");
		return ERR;
	}
	error_text(0, "Comando no implementado");
	free(msg);
	return OK;
}


static int clientcmd_cycle(Client* cli, char* cmdstr) {
	char** channelarray;
	int i, numchannels;
	if (IRCUserParse_Cycle(cmdstr, &channelarray, &numchannels) < 0) {
		clientcmd(cli, "/help cycle");
		return ERR;
	}
	for (i=0; i < numchannels; i++) {
		// PART
		clientcmdf(cli, "/part %s", channelarray[i]);
		clientcmdf(cli, "/join %s", channelarray[i]);
		clientcmdf(cli, "/mode %s", channelarray[i]); // Pide el modo al servidor
		free(channelarray[i]);
	}
	free(channelarray);
	return OK;
}

static int clientcmd_dcc(Client* cli, char* cmdstr) {
	error_text(0, "Comando no implementado");
	return OK;
}

static int clientcmd_dns(Client* cli, char* cmdstr) {
	char* opt;
	if (IRCUserParse_Dns(cmdstr, &opt)) {
		clientcmd(cli, "/help dns");
		return ERR;
	}
	error_text(0, "Comando no implementado");
	free(opt);
	return OK;
}

typedef struct {
	char* command;
	char* syntax;
	char* descr;
} HelpMessage;

static const HelpMessage help_array[] = {
	{ "admin",   	"/ADMIN server"                           	, "Muestra quien es el admin del servidor."              	},
	{ "away",    	"/AWAY reason"                            	, "Te marca como ausente"                                	},
	{ "botmotd", 	"/BOTMOTD server"                         	, "No implementado."                                     	},
	{ "ctcp",    	"/CTCP"                                   	, "No implementado."                                     	},
	{ "cycle",   	"/CYCLE channel1 channel2, ..."           	, "Abandona y vuelve a unirse a los canales."            	},
	{ "dcc",     	"/DCC"                                    	, "No implementado."                                     	},
	{ "dns",     	"/DNS option"                             	, "No implementado."                                     	},
	{ "help",    	"/HELP command"                           	, "Muestra ayuda sobre un comando"                       	},
	{ "identify",	"/IDENTIFY password"                      	, "Manda tu contraseña al servidor de identificacion."   	},
	{ "ignore",  	"/IGNORE nickuserhost1 nickuserhost2, ..."	, "Ignora a un usuario."                                 	},
	{ "invite",  	"/INVITE nick channel"                    	, "Invita a un usuario a un canal."                      	},
	{ "ison",    	"/ISON nick1 nick2 nick3 ..."             	, "Comprueba si unos usuarios estan conectados"          	},
	{ "join",    	"/JOIN channel1 channel2, ..."            	, "Unirse a un canal"                                    	},
	{ "kick",    	"/KICK channel nick"                      	, "Expulsar a un usuario de un canal"                    	},
	{ "knock",   	"/KNOCK channel message"                  	, "No implementado."                                     	},
	{ "leave",   	"/LEAVE channel"                          	, "Abandona un canal"                                    	},
	{ "list",    	"/LIST search string"                     	, "Busca entre las salas"                                	},
	{ "lusers",  	"/LUSERS server"                          	, "Da informacion sobre los usuarios conectados."        	},
	{ "mode",    	"/MODE chan/nick mode chan/nick"          	, "Cambia el modo de un usuario o canal"                 	},
	{ "motd",    	"/MOTD server"                            	, "Muestra el mensaje del dia del servidor"              	},
	{ "msg",     	"/MSG nick/channel msg"                   	, "Manda un mensaje a un usuario o canal"                	},
	{ "nick",    	"/NICK newnickname"                       	, "Cambia tu nickname"                                   	},
	{ "notice",  	"/NOTICE nick/channel msg"                	, "Manda un mensaje que no acepta respuestas automaticas"	},
	{ "notify",  	"/NOTIFY nick1 nick2, ..."                	, "No implementado."                                     	},
	{ "part",    	"/PART channel"                           	, "Abandona un canal"                                    	},
	{ "ping",    	"/PING user"                              	, "Manda un ping a un usuario"                           	},
	{ "query",   	"/QUERY nick/channel msg"                 	, "No implementado."                                     	},
	{ "quit",    	"/QUIT reason"                            	, "Cierra la aplicacion"                                 	},
	{ "rules",   	"/RULES server"                           	, "Imprime las reglas"                                   	},
	{ "stats",   	"/STATS option"                           	, "Muestra estadisticas"                                 	},
	{ "time",    	"/TIME server"                            	, "No implementado."                                     	},
	{ "topic",   	"/TOPIC channel topic"                    	, "Cambia el tema del canal"                             	},
	{ "userhost",	"/USERHOST nick"                          	, "Muestra informacion sobre el host de un usuario"      	},
	{ "userip",  	"/USERIP nick"                            	, "No implementado."                                     	},
	{ "version", 	"/VERSION server"                         	, "Muestra la version del servidor"                      	},
	{ "vhost",   	"/VHOST login password"                   	, "No implementado."                                     	},
	{ "who",     	"/WHO mask"                               	, "Busca informacion sobre un usuario"                   	},
	{ "whois",   	"/WHOIS nick"                             	, "Busca informacion sobre un usuario"                   	},
	{ "whowas",  	"/WHOWAS nick maxreplies"                 	, "Busca informacion sobre un usuario"                   	},
	{ NULL, NULL, NULL }
};


static int clientcmd_help(Client* cli, char* cmdstr) {
	char* cmd;
	const HelpMessage* help = help_array;
	if (IRCUserParse_Help(cmdstr, &cmd) < 0 || cmd == NULL) {
		clientcmd(cli, "/help help");
		return ERR;
	}
	for (; help->command; help++) {
		if (0 == strcasecmp(cmd, help->command)) {
			message_textf(current_page(),
				"Comando: \n%s\n%s",
				help->syntax,
				help->descr
			);
			free(cmd);
			return OK;
		}
	}
	error_text(0, "No hay ayuda disponible para ese comando.");
	free(cmd);
	return OK;
}


static int clientcmd_identify(Client* cli, char* cmdstr) {
	char* password;
	if (IRCUserParse_Identify(cmdstr, &password) < 0) {
		clientcmd(cli, "/help identify");
		return ERR;
	}
	clientcmdf(cli, "/msg NickServ identify %s", password);
	free(password);
	return OK;
}


static int clientcmd_ignore(Client* cli, char* cmdstr) {
	char** nickuserhostarray;
	int i, numnickuserhosts;
	if (IRCUserParse_Ignore(cmdstr, &nickuserhostarray, &numnickuserhosts) < 0) {
		clientcmd(cli, "/help ignore");
		return ERR;
	}
	for (i = 0; i < numnickuserhosts; i++) {
		if (client_add_ignore(cli, nickuserhostarray[i]) == OK) {
			message_textf(current_page(), "Ignorando a %s", nickuserhostarray[i]);
		} else {
			message_text(current_page(), "Estas ignorando a demasiada gente, haz las paces con alguno.");
		}
		free(nickuserhostarray[i]);
	}
	free(nickuserhostarray);
	return OK;
}


static int clientcmd_invite(Client* cli, char* cmdstr) {
	char* nick;
	char* channel;
	if (IRCUserParse_Invite(cmdstr, &nick, &channel) < 0) {
		clientcmd(cli, "/help invite");
		return ERR;
	}
	MAKECMD(IRC_Invite(cli->cmd, cli->pre, nick, channel));
	client_send_cmd(cli);
	free(nick);
	free(channel);
	return OK;
}


static int clientcmd_ison(Client* cli, char* cmdstr) {
	char** nickarray;
	int i, numnicks;
	if (IRCUserParse_Ison(cmdstr, &nickarray, &numnicks) < 0) {
		clientcmd(cli, "/help ison");
		return ERR;
	}
	for (i=0; i < numnicks; i++) {
		MAKECMD(IRC_Ison(cli->cmd, cli->pre, nickarray[i], NULL));
		client_send_cmd(cli);
		free(nickarray[i]);
	}
	free(nickarray);
	return OK;
}


static int clientcmd_join(Client* cli, char* cmdstr) {
	char** channelarray;
	int i, numchannels;
	if (IRCUserParse_Join(cmdstr, &channelarray, &numchannels) < 0) {
		clientcmd(cli, "/help join");
		return ERR;
	}
	for (i=0; i < numchannels; i++) {
		MAKECMD(IRC_Join(cli->cmd, cli->pre, channelarray[i], NULL, NULL)); /* channel, key */
		client_send_cmd(cli);
		free(channelarray[i]);
	}
	free(channelarray);
	return OK;
}


static int clientcmd_kick(Client* cli, char* cmdstr) {
	char* nick;
	if (IRCUserParse_Kick(cmdstr, &nick) < 0) {
		clientcmd(cli, "/help kick");
		return ERR;
	}
	MAKECMD(IRC_Kick(cli->cmd, cli->pre, get_name_page(current_page()), nick, NULL)); // channel, user, comment
	client_send_cmd(cli);
	free(nick);
	return OK;
}


static int clientcmd_knock(Client* cli, char* cmdstr) {
	char* channel;
	char* channelmessage;
	if (IRCUserParse_Knock(cmdstr, &channel, &channelmessage) < 0) {
		clientcmd(cli, "/help e_knock");
		return ERR;
	}
	error_text(0, "Comando no implementado");
	free(channel);
	free(channelmessage);
	return OK;
}


static int clientcmd_leave(Client* cli, char* cmdstr) {
	return clientcmd_part(cli, cmdstr);
}


static int clientcmd_license(Client* cli, char* cmdstr) {
	message_text(0, "The MIT License (MIT)");
	message_text(0, "Copyright (c) 2015 A.Alonso, M.Blanc");
	message_text(0, "Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the \"Software\"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:");
	message_text(0, "The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.");
	message_text(0, "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.");
	return OK;
}


static int clientcmd_links(Client* cli, char* cmdstr) {
	return OK;
}


static int clientcmd_list(Client* cli, char* cmdstr) {
	char* searchstring;
	if (IRCUserParse_List(cmdstr, &searchstring) < 0) {
		clientcmd(cli, "/help list");
		return ERR;
	}
	MAKECMD(IRC_List(cli->cmd, cli->pre, NULL, searchstring)); // channel, target
	client_send_cmd(cli);
	return OK;
}


static int clientcmd_lusers(Client* cli, char* cmdstr) {
	char* server;
	if (IRCUserParse_Lusers(cmdstr, &server) < 0) {
		clientcmd(cli, "/help lusers");
		return ERR;
	}
	MAKECMD(IRC_Lusers(cli->cmd, cli->pre, NULL, server)); /* mask, target */
	client_send_cmd(cli);
	return OK;
}


static int clientcmd_map(Client* cli, char* cmdstr) {
	return OK;
}


static int clientcmd_me(Client* cli, char* cmdstr) {
	char* msg;
	if (IRCUserParse_Me(cmdstr, &msg) < 0) {
		clientcmd(cli, "/help me");
		return ERR;
	}
	clientcmdf(cli, "/msg %s \01ACTION %s\01", current_page_name(), msg);
	free(msg);
	return OK;
}



static int clientcmd_mode(Client* cli, char* cmdstr) {
	char* nickchannel;
	char* nickchannel2;
	char* mode;
	if (IRCUserParse_Mode(cmdstr, &nickchannel, &mode, &nickchannel2) < 0) {
		clientcmd(cli, "/help mode");
		return ERR;
	}
	MAKECMD(IRC_Mode(cli->cmd, cli->pre, nickchannel, mode, nickchannel2));
	client_send_cmd(cli);
	return OK;
}


static int clientcmd_module(Client* cli, char* cmdstr) {
	error_text(current_page(), "Comando no implementado");
	return OK;
}


static int clientcmd_motd(Client* cli, char* cmdstr) {
	char* server;
	if (IRCUserParse_Motd(cmdstr, &server) < 0) {
		clientcmd(cli, "/help motd");
		return ERR;
	}
	MAKECMD(IRC_Motd(cli->cmd, cli->pre, server));
	client_send_cmd(cli);
	free(server);
	return OK;
}


static int clientcmd_msg(Client* cli, char* cmdstr) {
	char* nickchannel;
	char* msg;
	if (IRCUserParse_Msg(cmdstr, &nickchannel, &msg) < 0) {
		clientcmd(cli, "/help msg");
		return ERR;
	}
	MAKECMD(IRC_Privmsg(cli->cmd, cli->pre, nickchannel, msg));
	(*nickchannel == '#' ? public_text : private_text)(get_or_create_page(nickchannel), cli->nick, msg);
	client_send_cmd(cli);
	free(nickchannel);
	free(msg);
	return OK;
}


static int clientcmd_names(Client* cli, char* cmdstr) {
	MAKECMD(IRC_Names(cli->cmd, cli->pre, NULL, NULL));
	client_send_cmd(cli);
	return OK;
}


static int clientcmd_nick(Client* cli, char* cmdstr) {
	char* newnickname;
	if (IRCUserParse_Nick(cmdstr, &newnickname) < 0) {
		clientcmd(cli, "/help nick");
		return ERR;
	}
	MAKECMD(IRC_Nick(cli->cmd, NULL, newnickname));
	client_send_cmd(cli);
	client_set_nick(cli, newnickname); /* lo metemos en la estructura si es valido */
	client_set_prefix(cli, cli->nick); /* con el prefix a juego */
	free(newnickname);
	return OK;
}


static int clientcmd_notice(Client* cli, char* cmdstr) {
	char* nickchannel;
	char* msg;
	if (IRCUserParse_Notice(cmdstr, &nickchannel, &msg) < 0) {
		clientcmd(cli, "/help notice");
		return ERR;
	}
	MAKECMD(IRC_Notice(cli->cmd, cli->pre, nickchannel, msg));
	client_send_cmd(cli);
	return OK;
}


static int clientcmd_notify(Client* cli, char* cmdstr) {
	char** nickarray;
	int numnicks;
	if (IRCUserParse_Notify(cmdstr, &nickarray, &numnicks) < 0) {
		clientcmd(cli, "/help notify");
		return ERR;
	}
	error_text(0, "Comando no implementado");
	while (numnicks --> 0) free(nickarray[numnicks]);
	free(nickarray);
	return OK;
}


static int clientcmd_part(Client* cli, char* cmdstr) {
	char* channel;
	char* msg;
	if (IRCUserParse_Part(cmdstr, &channel, &msg) < 0) {
		clientcmd(cli, "/help part");
		return ERR;
	}
	if (channel == NULL) channel = get_name_page(current_page());
	MAKECMD(IRC_Part(cli->cmd, cli->pre, channel, msg));
	client_send_cmd(cli);
	free(msg);
	return OK;
}


static int clientcmd_partall(Client* cli, char* cmdstr) {
	return OK;
}


static int clientcmd_ping(Client* cli, char* cmdstr) {
	char* user;
	if (IRCUserParse_Ping(cmdstr, &user) < 0) {
		clientcmd(cli, "/help ping");
		return ERR;
	}
	IRC_Ping(cli->cmd, cli->pre, user, user); /* server, server2 */
	client_send_cmd(cli);
	free(user);
	return OK;
}


static int clientcmd_query(Client* cli, char* cmdstr) {
	char* nickchannel;
	char* msg;
	if (IRCUserParse_Query(cmdstr, &nickchannel, &msg) < 0) {
		clientcmd(cli, "/help query");
		return ERR;
	}
	error_text(0, "Comando no implementado");
	return OK;
}


static int clientcmd_quit(Client* cli, char* cmdstr) {
	char* reason;
	if (IRCUserParse_Quit(cmdstr, &reason) < 0) {
		clientcmd(cli, "/help quit");
		return ERR;
	}
	MAKECMD(IRC_Quit(cli->cmd, cli->pre, reason));
	client_send_cmd(cli);
	free(reason);
	return OK;
}


static int clientcmd_rules(Client* cli, char* cmdstr) {
	char* server;
	if (IRCUserParse_Rules(cmdstr, &server) < 0) {
		clientcmd(cli, "/help rules");
		return ERR;
	}
	error_text(0, "Comando no implementado");
	free(server);
	return OK;
}


static int clientcmd_setname(Client* cli, char* cmdstr) {
	// No hay funcion de parse ...
	error_text(0, "Comando no implementado");
	return OK;
}


static int clientcmd_stats(Client* cli, char* cmdstr) {
	char* option;
	if (IRCUserParse_Stats(cmdstr, &option) < 0) {
		clientcmd(cli, "/help stats");
		return ERR;
	}
	MAKECMD(IRC_Stats(cli->cmd, cli->pre, option, NULL)); /* query, target */
	client_send_cmd(cli);
	free(option);
	return OK;
}


static int clientcmd_time(Client* cli, char* cmdstr) {
	char* server;
	if (IRCUserParse_Time(cmdstr, &server) < 0) {
		clientcmd(cli, "/help time");
		return ERR;
	}
	MAKECMD(IRC_Time(cli->cmd, cli->pre, server));
	client_send_cmd(cli);
	free(server);
	return OK;
}


static int clientcmd_topic(Client* cli, char* cmdstr) {
	char* channel_topic;
	if (IRCUserParse_Topic(cmdstr, &channel_topic) < 0) {
		clientcmd(cli, "/help topic");
		return ERR;
	}
	MAKECMD(IRC_Topic(cli->cmd, cli->pre, channel_topic, NULL));
	client_send_cmd(cli);
	free(channel_topic);
	return OK;
}



static int clientcmd_userhost(Client* cli, char* cmdstr) {
	char* nick;
	if (IRCUserParse_Userhost(cmdstr, &nick) < 0) {
		clientcmd(cli, "/help userhost");
		return ERR;
	}
	MAKECMD(IRC_Userhost(cli->cmd, cli->pre, nick, NULL, NULL, NULL, NULL));
	client_send_cmd(cli);
	free(nick);
	return OK;
}


static int clientcmd_userip(Client* cli, char* cmdstr) {
	char* nick;
	if (IRCUserParse_UserIp(cmdstr, &nick) < 0) {
		clientcmd(cli, "/help userip");
		return ERR;
	}
	error_text(0, "Comando no implementado");
	free(nick);
	return OK;
}


static int clientcmd_version(Client* cli, char* cmdstr) {
	char* server;
	if (IRCUserParse_Version(cmdstr, &server) < 0) {
		clientcmd(cli, "/help version");
		return ERR;
	}
	MAKECMD(IRC_Version(cli->cmd, cli->pre, server));
	client_send_cmd(cli);
	free(server);
	return OK;
}


static int clientcmd_vhost(Client* cli, char* cmdstr) {
	char* login;
	char* password;
	if (IRCUserParse_Vhost(cmdstr, &login, &password) < 0) {
		clientcmd(cli, "/help vhost");
		return ERR;
	}
	error_text(0, "Comando no implementado");
	free(login);
	free(password);
	return OK;
}


static int clientcmd_who(Client* cli, char* cmdstr) {
	char* mask;
	if (IRCUserParse_Who(cmdstr, &mask) < 0) {
		clientcmd(cli, "/help who");
		return ERR;
	}
	MAKECMD(IRC_Who(cli->cmd, cli->pre, mask, NULL)); /* mask, oppar */
	client_send_cmd(cli);
	free(mask);
	return OK;
}


static int clientcmd_whois(Client* cli, char* cmdstr) {
	char* nick;
	if (IRCUserParse_Whois(cmdstr, &nick) < 0) {
		clientcmd(cli, "/help whois");
		return ERR;
	}
	MAKECMD(IRC_Whois(cli->cmd, cli->pre, nick, NULL)); /* target, maskarray */
	client_send_cmd(cli);
	free(nick);
	return OK;
}


static int clientcmd_whowas(Client* cli, char* cmdstr) {
	char* nick;
	int maxreplies;
	if (IRCUserParse_WhoWas(cmdstr, &nick, &maxreplies) < 0) {
		clientcmd(cli, "/help whowas");
		return ERR;
	}
	MAKECMD(IRC_Whowas(cli->cmd, cli->pre, nick, maxreplies, NULL)); /* nick, count, target */
	client_send_cmd(cli);
	free(nick);
	return OK;
}



/* Para la funcion client_connect de client.c, manda el paquete USER */
int client_connect_user(Client* cli, char* mode) {
	MAKECMD(IRC_User(cli->cmd, cli->pre, cli->name, mode, cli->rname));
	client_send_cmd(cli);
	return OK;
}

static int clientcmd_fsend(Client* cli, char* cmdstr) {
	char nick[IRC_MAX_NICK_LEN+1];
	int file_offset;

	sscanf(cmdstr, "/fsend %9s %n", nick, &file_offset);
	clientfile_send(cli, nick, cmdstr+file_offset);

	return OK;
}

static int clientcmd_faccept(Client* cli, char* cmdstr) {
	if (clientfile_recv()) {
		error_text(current_page(), "Ha ocurrido un error, no hay nada pendiente de recibir");
	}
	return OK;
}

static int clientcmd_fcancel(Client* cli, char* cmdstr) {
	return OK;
}

int clientcmd(Client* cli, char* cmd) {
	int currpage = current_page();
	LOG("Ejecutando comando: %s", cmd);

	/* Comandos de envio de ficheros */
	if (strncasecmp(cmd, "/fsend",   6) == 0) return clientcmd_fsend(cli, cmd);
	if (strncasecmp(cmd, "/faccept", 8) == 0) return clientcmd_faccept(cli, cmd);
	if (strncasecmp(cmd, "/fcancel", 8) == 0) return clientcmd_fcancel(cli, cmd);

	switch (IRCUser_CommandQuery(cmd)) {
		/* Casos de error */
		default:
		case IRCERR_NOSTRING          	: error_text(currpage, "Cadena nula"              	); break;
		case IRCERR_NOUSERCOMMAND     	: error_text(currpage, "No hay comando de usuario"	); break;
		case IRCERR_NOVALIDUSERCOMMAND	: error_text(currpage, "Comando no valido"        	); break;
		/* Mensajes validos */
		case UADMIN   	: return clientcmd_admin   	(cli, cmd);
		case UAWAY    	: return clientcmd_away    	(cli, cmd);
		case UBOTMOTD 	: return clientcmd_botmotd 	(cli, cmd);
		case UCHAT    	: return clientcmd_chat    	(cli, cmd);
		case UCTCP    	: return clientcmd_ctcp    	(cli, cmd);
		case UCYCLE   	: return clientcmd_cycle   	(cli, cmd);
		case UDCC     	: return clientcmd_dcc     	(cli, cmd);
		case UDNS     	: return clientcmd_dns     	(cli, cmd);
		case UHELP    	: return clientcmd_help    	(cli, cmd);
		case UIDENTIFY	: return clientcmd_identify	(cli, cmd);
		case UIGNORE  	: return clientcmd_ignore  	(cli, cmd);
		case UINVITE  	: return clientcmd_invite  	(cli, cmd);
		case UISON    	: return clientcmd_ison    	(cli, cmd);
		case UJOIN    	: return clientcmd_join    	(cli, cmd);
		case UKICK    	: return clientcmd_kick    	(cli, cmd);
		case UKNOCK   	: return clientcmd_knock   	(cli, cmd);
		case ULEAVE   	: return clientcmd_leave   	(cli, cmd);
		case ULICENSE 	: return clientcmd_license 	(cli, cmd);
		case ULINKS   	: return clientcmd_links   	(cli, cmd);
		case ULIST    	: return clientcmd_list    	(cli, cmd);
		case ULUSERS  	: return clientcmd_lusers  	(cli, cmd);
		case UMAP     	: return clientcmd_map     	(cli, cmd);
		case UME      	: return clientcmd_me      	(cli, cmd);
		case UMODE    	: return clientcmd_mode    	(cli, cmd);
		case UMODULE  	: return clientcmd_module  	(cli, cmd);
		case UMOTD    	: return clientcmd_motd    	(cli, cmd);
		case UMSG     	: return clientcmd_msg     	(cli, cmd);
		case UNAMES   	: return clientcmd_names   	(cli, cmd);
		case UNICK    	: return clientcmd_nick    	(cli, cmd);
		case UNOTICE  	: return clientcmd_notice  	(cli, cmd);
		case UNOTIFY  	: return clientcmd_notify  	(cli, cmd);
		case UPART    	: return clientcmd_part    	(cli, cmd);
		case UPARTALL 	: return clientcmd_partall 	(cli, cmd);
		case UPING    	: return clientcmd_ping    	(cli, cmd);
		case UQUERY   	: return clientcmd_query   	(cli, cmd);
		case UQUIT    	: return clientcmd_quit    	(cli, cmd);
		case URULES   	: return clientcmd_rules   	(cli, cmd);
		case USETNAME 	: return clientcmd_setname 	(cli, cmd);
		case USTATS   	: return clientcmd_stats   	(cli, cmd);
		case UTIME    	: return clientcmd_time    	(cli, cmd);
		case UTOPIC   	: return clientcmd_topic   	(cli, cmd);
		case UUSERHOST	: return clientcmd_userhost	(cli, cmd);
		case UUSERIP  	: return clientcmd_userip  	(cli, cmd);
		case UVERSION 	: return clientcmd_version 	(cli, cmd);
		case UVHOST   	: return clientcmd_vhost   	(cli, cmd);
		case UWHO     	: return clientcmd_who     	(cli, cmd);
		case UWHOIS   	: return clientcmd_whois   	(cli, cmd);
		case UWHOWAS  	: return clientcmd_whowas  	(cli, cmd);
	}
	return ERR;
}

int clientcmdf(Client* cli, char* fmt, ...) {
	char cmd[1024];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(cmd, sizeof cmd, fmt, ap);
	va_end(ap);

	return clientcmd(cli, cmd);
}

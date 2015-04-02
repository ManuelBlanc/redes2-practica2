
int change_flags_usr(){

}

int change_flags_chan(){

}

int exe_msg(Server* serv, User* who, const char* msg) {

	switch (UserParse_Command(msg)) {

		case MODE: exe_mode(serv, who, msg);
	}
}

int exe_mode(Server* serv, User* who, const char* msg) {
	char* channel_name;
	char* user;
	char  mode;
	IRCParse_Mode(msg, NULL, &channel_name, &mode, &user);

	Channel* chan = server_find_channel_by_name(channel_name);
	if (user != NULL) {
		userlist_findByName(UserList list, const char* name);
		/* buscar user y llamar a cambiarflags de usuario*/
	} else {
		/* cambiar flags decanal*/
	}
	IRC_Mode(who->send, who->pre, channel_name, mode, char *user);
	channel_sendf(chan, "MODE %s +%c", user_get_name(who), mode);
}

int exe_quit(Server* serv, User* who, const char* msg) {
/*sacar los semaforos fuera delparser.c*/
	/* down semaforo*/
	userlist_extract(&who);/*mal argumento*/
	/* up semaforo*/
	channel_sendf(chan, "MODE %s +%c", user_get_name(who), mode);

}

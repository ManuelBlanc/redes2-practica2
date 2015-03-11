
int cambia_flags_usuario(){

}

int cambia_flags_canal(){

}

int ejecutar_mensaje(Server* serv, User* who, const char* msg) {

	switch (UserParse_Command(msg)) {

		case MODE: ejecutar_mode(serv, who, msg);
	}
}

int ejecutar_mode(Server* serv, User* who, const char* msg) {
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

int ejecutar_quit(Server* serv, User* who, const char* msg) {
/*sacar los semaforos fuera delparser.c*/
	/* down semaforo*/
	userlist_extract(&who);/*mal argumento*/
	/* up semaforo*/
	channel_sendf(chan, "MODE %s +%c", user_get_name(who), mode);

}

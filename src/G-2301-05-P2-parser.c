
int change_flags_usr(){

}

int change_flags_chan(){

}
int serverrcv_cmd(User* usr, char* str){
	char* cmd;
	int more_commands = 1;

	while (more_commands) {
		switch (IRC_UnPipelineCommands(str, &cmd)) {
		case IRC_ENDPIPE:
			cli->buf[0] = '\0';
			more_commands = 0;
		case IRC_OK:
			clientrplP_switch(cli, cmd);
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

int exe_msg(Server* serv, User* who, const char* msg) {

	switch (/*existe???*/UserParse_Command(msg)) {

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


Channel* channel_new(void) {
	Channel* chan = malloc(sizeof Channel);
	return chan;
}


void channel_delete(Channel* chan) {
	free(chan);
}

void channel_mutex_enter(Channel* chan) {
	if (chan == NULL) return;
	pthread_mutex_lock(chan->mutex);
}

void channel_mutex_leave(Channel* chan) {
	if (chan == NULL) return;
	pthread_mutex_unlock(chan->mutex);
}

int channel_add_user(Channel* chan, User* usr) {
	channel_mutex_enter(chan);
	UserChannelData* usrData = malloc(UserChannelData);
	usrData->usr   = usr;
	usrData->flags = 0;
	usrData->next  = chan->usrs;
	chan->usrs = usrData;
	return OK;
}

int channel_remove_user(Channel* chan, User* usr) {
	UserChannelData** usrData = &chan->usrs;
	while (*usrData != NULL) {
		UserChannelData* usrNext = *usrData;
		if (next->usr == usr) {
			*usrData = usrNext->next;
			return OK;
		}
		*usrData = &usrNext->next;
	}
	return ERR;
}

int channel_find_user_data(Channel* chan, User* usr, UserChannelData** ucd) {
	UserChannelData* usrData = chan->usrs;
	while (usrData != NULL) {
		if (usrData->usr == usr) {
			*ucd = usrData;
			return 1;
		}
		usrData = usrData->next;
	}
	return 0;
}

int channel_get_flags_user(Channel* chan, User* usr, long* flags) {
	UserChannelData* ucd;
	if (!channel_find_user_data(chan, usr, &ucd)) return ERR;
	*flags = ucd->flags;
	return OK;
}

int channel_set_flags_user(Channel* chan, User* usr, long flags, User* actor) {
	UserChannelData* ucd;

	// Si un usuario pidio esta accion
	if (actor != NULL) {
		// Buscamos al actor en la sala
		if (!channel_find_user_data(chan, actor, &ucd)) return ERR;
		// Comprobamos si tiene permisos
		if (USRFLAG_OPERATOR | ucd->flags == 0) return ERR;
	}

	// Buscamos al objetivo
	if (!channel_find_user_data(chan, usr, &ucd)) return ERR;

	// Exito!
	ucd->flags |= flags;
	return OK;
}

int channel_unset_flags_user(Channel* chan, User* usr, long flags, User* usr) {
	UserChannelData* ucd;

	// Si un usuario pidio esta accion
	if (actor != NULL) {
		// Buscamos al actor en la sala
		if (!channel_find_user_data(chan, actor, &ucd)) return ERR;
		// Comprobamos si tiene permisos
		if (USRFLAG_OPERATOR | ucd->flags == 0) return ERR;
	}

	// Buscamos al objetivo
	if (!channel_find_user_data(chan, usr, &ucd)) return ERR;

	// Exito!
	ucd->flags &= ~flags;
	return OK;
}

int channel_send_message(Channel* chan, User* usr, const char* msg) {
	UserChannelData* usrData = chan->usrs;
	while (usrData != NULL) {
		user_send_message(usrData->usr, chan->name, msg);
		usrData = usrData->next;
	}
	return OK;
}


int channel_get_topic(Channel* chan, const char** topic) {

}

int channel_set_topic(Channel* chan, const char*  topic, User* usr) {

}

int channel_get_name(Channel* chan, const char** name) {

}

int channel_set_name(Channel* chan, const char*  name) {

}

int channel_get_passwd(Channel* chan, const char** passwd) {

}

int channel_set_passwd(Channel* chan, const char*  passwd, User* usr) {

}

int channel_get_flags(Channel* chan, long* flags) {

}

int channel_set_flags(Channel* chan, long flags, User* usr) {

}

int channel_unset_flags(Channel* chan, long flags, User* usr) {

}

int channellist_insert(ChannelList list, Channel chan) {

}

ChannelList channellist_select(ChannelList list, int index) {

}

Channel channellist_extract(ChannelList list) {

}

ChannelList channellist_findByName(ChannelList list, const char* name) {

}

void channellist_deleteAll(ChannelList list) {

}

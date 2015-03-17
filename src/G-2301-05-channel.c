Channel* channel_new(voi) {

}
void channel_delete(Channel* chan) {

}
int channel_add_user(Channel* chan, User* usr) {

}
int channel_remove_user(Channel* chan, User* usr) {

}
int channel_get_flags_user(Channel* chan, User* usr, UserFlags* flags) {

}
int channel_set_flags_user(Channel* chan, User* usr, UserFlags flags, User* usr) {

}
int channel_unset_flags_user(Channel* chan, User* usr, UserFlags flags, User* usr) {

}
int channel_send_message(Channel* chan, User* usr, const char* msg) {

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
int channel_set_passwd(Channel* chan, const char*  passwd) {

}
int channel_get_flags(Channel* chan, ChannelFlags* flags) {

}
int channel_set_flags(Channel* chan, ChannelFlags flags, User* usr) {

}
int channel_unset_flags(Channel* chan, ChannelFlags flags, User* usr) {

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

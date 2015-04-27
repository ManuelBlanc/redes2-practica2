/* std */
#include <stdlib.h>
#include <string.h>
/* lib */
#include <redes2/chat.h>
/* usr */
#include "G-2301-05-P3-util.h"
#include "G-2301-05-P3-chat.h"
#include "G-2301-05-P3-client.h"
#include "G-2301-05-P3-clientcmd.h"


static Client* cli;


void connect_client(void)
{

	cli = client_new();

	client_set_server(cli, get_server());
	client_set_port(cli, get_port());
	client_set_name(cli, get_name());
	client_set_real_name(cli, get_real_name());
	client_set_nick(cli, get_nick());

	client_connect(cli);
	inputs_set_editable(FALSE);
}


void disconnect_client(void)
{
	client_disconnect(cli);
	inputs_set_editable(TRUE);
}


//========================================================================
static void funcion_checkbox(char bandera, gboolean state) {
	char* channel = current_page_name();
	if (current_page() == 0) return;
	clientcmdf(cli, "/mode %s %c%c", channel, state ? '+' : '-', bandera);
}
void topic_protect	(gboolean state) { funcion_checkbox('t', state); }
void extern_msg   	(gboolean state) { funcion_checkbox('n', state); }
void secret       	(gboolean state) { funcion_checkbox('s', state); }
void guests       	(gboolean state) { funcion_checkbox('i', state); }
void privated     	(gboolean state) { funcion_checkbox('p', state); }
void moderated    	(gboolean state) { funcion_checkbox('m', state); }
//========================================================================

void new_text (char *msg)
{
	int currpage = current_page();

	// Cadena no vacia
	if (msg == NULL || msg[0] == '\0') return;
	if (msg[0] == '/') {
		if (ERR == clientcmd(cli, msg)) {
			error_text(currpage, "Error ejecutando el comando.");
		}
		return;
	}

	if (currpage != 0) {
		clientcmdf(cli, "/msg %s %s", current_page_name(), msg);
	}
}

void exit_func(void)
{
	disconnect_client();
	LOG("Cerrando el programa");
}

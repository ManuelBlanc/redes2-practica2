/* std */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* posix */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
/* redes2 */
#include <redes2/chat.h>
#include <redes2/irc.h>
/* usr */
#include "G-2301-05-P3-util.h"
#include "G-2301-05-P3-client.h"
#include "G-2301-05-P3-clientcmd.h"
#include "G-2301-05-P3-clientrpl.h"
/* ssl */
#include "G-2301-05-P3-ssl_functions.h"
#include "G-2301-05-P3-ssock.h"

Client* client_new() {
	Client* cli = calloc(1, sizeof (*cli));
	return cli;
}

void client_delete(Client* cli) {
	if (!cli) return;
	free(cli->serv);
	free(cli->name);
	free(cli->rname);
	ssock_close(cli->sock);
	free(cli);
}

int client_add_ignore(Client* cli, char* nick) {
	int i;
	ASSERT(nick != NULL, "Parametro nick a ignorar nulo");
	for (i = 0; i < NUM_MAX_IGN; i++) {
		if (cli->ignored[i][0] == '\0') {
			strncpy(cli->ignored[i], nick, IRC_MAX_NICK_LEN);
			break;
		}
	}
	if (i == NUM_MAX_IGN) return ERR;
	return OK;
}

int client_remove_ignore(Client* cli, char* nick) {
	int i;
	ASSERT(nick != NULL, "Parametro nick a dejar de ignorar nulo");
	for (i = 0; i < NUM_MAX_IGN; i++) {
		if (strcmp(cli->ignored[i], nick) == 0) {
			memset(cli->ignored[i], 0, IRC_MAX_NICK_LEN); // ponemos a 0
			break;
		}
	}
	if (i == NUM_MAX_IGN) return ERR;
	return OK;
}

int client_lookfor_ignore(Client* cli, char* nick) {
	int i;
	ASSERT(nick != NULL, "Parametro nick a buscar nulo");
	for (i = 0; i < NUM_MAX_IGN; i++) {
		if (strncmp(cli->ignored[i], nick, IRC_MAX_NICK_LEN) == 0) {
			return TRUE;
		}
	}
	return FALSE;
}

int client_set_server(Client* cli, char* serv) {
	ASSERT(serv != NULL, "Parametro servidor nulo");
	free(cli->serv);
	cli->serv = malloc(strlen(serv) + 1);
	strcpy(cli->serv, serv);
	return OK;
}

int client_set_port(Client* cli, unsigned int port) {
	ASSERT(port > 0 && port < (1<<16), "Parametro puerto nulo");
	cli->port = htons(port);
	return OK;
}

int client_set_name(Client* cli, char* name) {
	ASSERT(name != NULL, "Parametro nombre nulo");
	free(cli->name);
	cli->name = malloc(strlen(name) + 1);
	strcpy(cli->name, name);
	return OK;
}

int client_set_real_name(Client* cli, char* rname) {
	ASSERT(rname != NULL, "Parametro nombre verdadero nulo");
	free(cli->rname);
	cli->rname = malloc(strlen(rname) + 1);
	strcpy(cli->rname, rname);
	return OK;
}

int client_set_nick(Client* cli, char* nick) {
	ASSERT(nick != NULL, "Parametro nick nulo");
	strncpy(cli->nick, nick, IRC_MAX_NICK_LEN);
	cli->nick[IRC_MAX_NICK_LEN] = '\0';
	return OK;
}

int client_send_cmd(Client* cli) {
	LOG("Enviando '%s'", cli->cmd);
	if(ERR == ssock_send(cli->sock, cli->cmd, strlen(cli->cmd))) return ERR;
	return OK;
}

int client_set_prefix(Client* cli, char* pre) {
	ASSERT(pre != NULL, "Parametro prefix nulo");
	strncpy(cli->pre, pre, IRC_MAX_PRE_LEN);
	cli->pre[IRC_MAX_PRE_LEN] = '\0';
	return OK;
}

int client_send_pong(Client* cli, char* server, char* server2) {
	/* Usamos un buffer aparte para no interferir con el thread de envios */
	char buffer[512];
	IRC_Pong(buffer, cli->pre, server, server2);
	if(ERR == ssock_send(cli->sock, buffer, strlen(buffer))) return ERR;
	return OK;
}

/** Ejecuta el hilo de recepcion y manda lo recibido al clientrpl
 * @param void* cli_ptr estructura con datos del cliente
 * @return
 */
static void* receive(void* cli_ptr) {
	ssize_t len;
	size_t len_buf;
	Client* cli = (Client*) cli_ptr;
	while (1) {
		len_buf = strlen(cli->buf);
		len = ssock_recv(cli->sock, cli->buf, sizeof(cli->buf));
		if (len <= 0) return NULL; // Se cierra la conexion
		cli->buf[len+len_buf] = '\0';
		clientrpl(cli, cli->buf);
	}
	return NULL;
}

extern Redes2_SSL_CTX* contexto;

int client_connect(Client* cli) {
	int desc;
	struct sockaddr_in addr;
	struct hostent* infoServer;
	struct in_addr* addrServer;
	char* dom;

	/* Creacion de socket */
	desc = socket(AF_INET, SOCK_STREAM, 0);
	ASSERT(desc != -1, "No se pudo crear un socket.");
	

	/* Se completa la estructura necesaria para la conexion */
	ASSERT((infoServer = gethostbyname(cli->serv)) != NULL, "No estas conectado a un DNS");
	addrServer = (struct in_addr *)infoServer->h_addr_list[0];
	ASSERT((infoServer != NULL), "Error al obtener la ip del servidor");
	addr.sin_family = AF_INET;
	addr.sin_port = cli->port;
	addr.sin_addr = *addrServer;
	memset(&addr.sin_zero, 0, 8);

	/* Se pide conexion */
	ASSERT(connect(desc, (struct sockaddr *)&addr, sizeof(addr)) != -1, "Error en connect");
	pthread_create(&cli->thr, NULL, &receive, cli);

	if (cli->port == SECURE_PORT) {
		cli->sock = ssock_secure_new(desc, conectar_canal_seguro_SSL(contexto, desc));
	} else {
		cli->sock = ssock_new(desc);
	}

	/* Se manda el commando NICK */
	dom = malloc(strlen(cli->nick)+6+1);
	sprintf(dom, "/nick %s", cli->nick);
	clientcmd(cli, dom);
	free(dom);

	/* Se manda el commando USER */
	client_connect_user(cli, "0"); //mode

	return OK;
}

int client_disconnect(Client* cli) {
	if (cli->sock == NULL) return ERR;
	clientcmd(cli, "/quit Bye");
	pthread_join(cli->thr, NULL);
	cli->sock = NULL;
	return OK;
}

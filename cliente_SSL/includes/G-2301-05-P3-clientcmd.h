
#ifndef CLIENTCMD_H
#define CLIENTCMD_H

#include "G-2301-05-P3-client.h"

int clientcmd(Client* cli, char* cmdstr);
int clientcmdf(Client* cli, char* fmt, ...) __attribute__((format (printf, 2, 3)));

int client_connect_user(Client* cli, char* mode);

#endif /* CLIENTCMD_H */


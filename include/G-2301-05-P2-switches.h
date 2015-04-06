#ifndef SWITCHES_H
#define SWITCHES_H

long checksend_message_chan(Channel* dst, User* src, char* msg);

long checksend_message_usr(User* dst, User* src, char* msg);

/**
 * Llama a la funcion correspondiente dependiendo del comando recibido
 * @param
 * @param
 * @param
 * @return
 */
int action_switch(Server* serv, User* usr, char* str);

//Funciones de comandos usadas en user.c

int exec_cmd_pass(Server* serv, User* usr, char* cmd);
int exec_cmd_nick(Server* serv, User* usr, char* cmd);
int exec_cmd_user(Server* serv, User* usr, char* cmd);

#endif /* SWITCHES_H */

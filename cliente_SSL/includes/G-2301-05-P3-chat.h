
#ifndef CHAT_H
#define CHAT_H

#include <glib.h>

/* Exponemos las funciones para cambiar los checkboxes.
 * Asi podemos desmarcar/marcar las cajas de acuerdo
 * con los cambios en el modo del canal.
 */

void set_topic_protect(int pagenum, gboolean state);
void set_extern_msg(int pagenum, gboolean state);
void set_secret(int pagenum, gboolean state);
void set_guests(int pagenum, gboolean state);
void set_privated(int pagenum, gboolean state);
void set_moderated(int pagenum, gboolean state);

void message_textf(int pagenum, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));;

/* Por comodidad */
int get_or_create_page(char* name);
char* current_page_name(void);

/* No permitir la edicion */
void inputs_set_editable(gboolean state);

#endif /* CHAT_H */

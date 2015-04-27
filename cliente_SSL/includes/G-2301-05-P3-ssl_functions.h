
#ifndef SSL_FUNCTIONS_H
#define SSL_FUNCTIONS_H

#include <sys/types.h>

typedef struct Redes2_SSL_CTX Redes2_SSL_CTX;
typedef struct Redes2_SSL Redes2_SSL;

typedef struct Redes2_SSL_CTX_config {
	char* ca_file;  // Autoridad de certificados
	char* ca_path;  // Ruta a hashes de autoridades de certificados
	char* key_file; // Clave privada
	char* pem_file; // Certificado completo
} Redes2_SSL_CTX_config;

/* Esta función se encargará de realizar todas las llamadas necesarias para que la aplicación
 * pueda usar la capa segura SSL.
 */
void inicializar_nivel_SSL(void);

/* Esta función se encargará de inicializar correctamente el contexto que será utilizado para
 * la creación de canales seguros mediante SSL. Deberá recibir información sobre las rutas a los certificados y
 * claves con los que vaya a trabajar la aplicación.
 * @param r2ssl estructura para los parametros de conexion segura
 */
Redes2_SSL_CTX* fijar_contexto_SSL(Redes2_SSL_CTX_config cnf);

/* Dado un contexto SSL y un descriptor de socket esta función se encargará de
 * obtener un canal seguro SSL inciando el proceso de handshake con el otro extremo.
 */
Redes2_SSL* conectar_canal_seguro_SSL(Redes2_SSL_CTX* r2ssl_ctx, int sock_fd);

/* Dado un contexto SSL y un descriptor de socket esta función se encargará de
 * bloquear la aplicación, que se quedará esperando hasta recibir un handshake por parte del cliente.
 */
Redes2_SSL* aceptar_canal_seguro_SSL(Redes2_SSL_CTX* r2ssl_ctx, int sock_fd);

/* Esta función comprobará una vez realizado el handshake que el canal de comunicación
 * se puede considerar seguro.
 * @param r2ssl estructura con los datos de la conexion segura
 * @return OK o ERR
 */
int evaluar_post_connectar_SSL(Redes2_SSL* r2ssl);

/* Esta función será el equivalente a la función de envío de mensajes que se realizó en la
 * práctica 1, pero será utilizada para enviar datos a través del canal seguro. Es importante que sea genérica y
 * pueda ser utilizada independientemente de los datos que se vayan a enviar.
 * @param r2ssl estructura con los datos de la conexion segura
 * @param buf buffer de envio
 * @param len tamaño del buffer
 * @return -1 si ha ocurrido un error o el numero de bytes enviados en caso contrario
 */
ssize_t enviar_datos_SSL(Redes2_SSL* r2ssl, void* buf, size_t len);

/* Esta función será el equivalente a la función de lectura de mensajes que se realizó en la
 * práctica 1, pero será utilizada para enviar datos a través del canal seguro. Es importante que sea genérica y
 * pueda ser utilizada independientemente de los datos que se vayan a recibir.
 * @param r2ssl estructura con los datos de la conexion segura
 * @param buf buffer de recepcion
 * @param len tamaño del buffer
 * @return -1 si ha ocurrido un error o el numero de bytes leidos en caso contrario
 */
ssize_t recibir_datos_SSL(Redes2_SSL* r2ssl, void* buf, size_t len);

/* Esta función liberará todos los recursos y cerrará el canal de comunicación seguro creado
 * previamente.
 * @param r2ssl estructura con los datos de la conexion segura
 */
int cerrar_canal_SSL(Redes2_SSL* r2ssl);

#endif /* SSL_FUNCTIONS_H */
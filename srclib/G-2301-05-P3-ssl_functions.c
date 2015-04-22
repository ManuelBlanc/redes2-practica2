#include <openssl/err.h>
#include <openssl/ssl.h>
#include "G-2301-05-P3-ssl_functions.h"
#include "G-2301-05-P3-util.h"

struct Redes2_SSL_CTX {
	const SSL_METHOD*  connection_method;
	SSL_CTX*     ctx;
};

struct Redes2_SSL {
	SSL* ssl;
};


void inicializar_nivel_SSL(void) {
	// Carga los errores para poder pintarlos
	SSL_load_error_strings();
	// Inicializa la libreria SSL y registra
	// los metodos de cifrado soportados
	SSL_library_init();
}

Redes2_SSL_CTX* fijar_contexto_SSL(void) {
	Redes2_SSL_CTX* r2ssl_ctx = emalloc(sizeof(*r2ssl_ctx));
	// Devuelve el metodo de conexion
	r2ssl_ctx->connection_method = SSLv23_method();

	// Crea un contexto usando un metodo de conexion
	r2ssl_ctx->ctx = SSL_CTX_new(r2ssl_ctx->connection_method);
	if (NULL == r2ssl_ctx->ctx) {
		LOG("Error al crear un contexto SSL");
		ERR_print_errors_fp(stdout);
		return NULL;
	}

	// Añade nuestra CA
	if(1 != SSL_CTX_load_verify_locations(r2ssl_ctx->ctx, "./cert/rootcert.pem", NULL)) {
		LOG("Error al comprobar la existencia de nuestro certificado");
		ERR_print_errors_fp(stdout);
		return NULL;
	}

	// Carpeta donde tiene que buscar las CA conocidas
	SSL_CTX_set_default_verify_paths(r2ssl_ctx->ctx);

	// Que certificado usara nuestra aplicacion
	if(1 != SSL_CTX_use_certificate_chain_file(r2ssl_ctx->ctx, "./cert/server.pem")) {
		LOG("Error al agregar nuestro certificado");
		ERR_print_errors_fp(stdout);
		return NULL;
	}

	// Clave privada de nuestra aplciacion
	if(1 != SSL_CTX_use_PrivateKey_file(r2ssl_ctx->ctx, "./cert/server_both.pem", SSL_FILETYPE_PEM)) {
		LOG("Error al agregar nuestra clave privada");
		ERR_print_errors_fp(stdout);
		return NULL;
	}


	SSL_CTX_set_verify(r2ssl_ctx->ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, 0);
	return r2ssl_ctx;
}

Redes2_SSL* conectar_canal_seguro_SSL(Redes2_SSL_CTX* r2ssl_ctx, int sock_fd) {
	//creamos la estructura donde se guardara el socket ssl
	Redes2_SSL* r2ssl = emalloc(sizeof(*r2ssl));
	r2ssl->ssl = SSL_new(r2ssl_ctx->ctx);
	if (r2ssl == NULL) {
		LOG("Error al crear un socket SSL");
		ERR_print_errors_fp(stdout);
		free(r2ssl);
		return NULL;
	}
	//le asociamos el descriptor del socket 
	if(1 != SSL_set_fd(r2ssl->ssl, sock_fd)) {
		LOG("Error al asociar el socket SSL");
		ERR_print_errors_fp(stdout);
		free(r2ssl);
		return NULL;
	}
	//se conecta de forma segura
	if(1 != SSL_connect(r2ssl->ssl)) {
		LOG("Error al conectarse de forma segura");
		ERR_print_errors_fp(stdout);
		free(r2ssl);
		return NULL;
	}
	return r2ssl;
}

static void print_error(unsigned long e) {
	char buffer[120];
	LOG("Error es: %s", ERR_error_string(e, buffer));
}

Redes2_SSL* aceptar_canal_seguro_SSL(Redes2_SSL_CTX* r2ssl_ctx, int sock_fd) {
	//creamos la estructura donde se guardara el socket ssl
	Redes2_SSL* r2ssl = emalloc(sizeof(*r2ssl));
	r2ssl->ssl = SSL_new(r2ssl_ctx->ctx);
	if (r2ssl == NULL) {
		LOG("Error al crear un socket SSL");
		ERR_print_errors_fp(stdout);
		free(r2ssl);
		return NULL;
	}
	//le asociamos el descriptor del socket 
	if(1 != SSL_set_fd(r2ssl->ssl, sock_fd)) {
		LOG("Error al asociar el socket SSL");
		ERR_print_errors_fp(stdout);
		free(r2ssl);
		return NULL;
	}
	//se conecta de forma segura
	unsigned long e = SSL_accept(r2ssl->ssl);
	if(1 != e) {
		LOG("Error al aceptar conexion de forma segura");
		print_error(e);
		free(r2ssl);
		return NULL;
	}
	return r2ssl;
}


/** Comprueba que el certificado del par es valida segun un CA */
int evaluar_post_connectar_SSL(Redes2_SSL* r2ssl) {
	if (NULL == r2ssl) return ERR;

	// Obtenemos el certificado del par
	// Queremos ver que no es NULL para asegurarnos de que
	// el par realmente proporciono un certificado.
	// El problema esta en que si el par no nos envia un
	// certificado, entonces no hay nada que validar y por
	// tanto no hay error de validacion!!!!
	X509* cert = SSL_get_peer_certificate(r2ssl->ssl);
	if (NULL == cert) return ERR;
	X509_free(cert);

	// Si ha ocurrido un error al verificar, abortamos tambien
	if (X509_V_OK != SSL_get_verify_result(r2ssl->ssl)) return ERR;

	// Si hemos llegado hasta aqui, exito!!
	return OK;
}

/** Envia datos por la conexion encapsulada, como send() */
ssize_t enviar_datos_SSL(Redes2_SSL* r2ssl, void* buf, size_t len) {
	if (NULL == r2ssl) return -1;
	return (ssize_t)SSL_write(r2ssl->ssl, buf, (int)len);
}


/** Envia datos por la conexion encapsulada, como recv() */
ssize_t recibir_datos_SSL(Redes2_SSL* r2ssl, void* buf, size_t len) {
	if (NULL == r2ssl) return -1;
	return (ssize_t)SSL_read(r2ssl->ssl, buf, (int)len);
}

/** Destruye la conexion */
int cerrar_canal_SSL(Redes2_SSL* r2ssl) {
	while(!SSL_shutdown(r2ssl->ssl));
	SSL_free(r2ssl->ssl);
	return OK;
}

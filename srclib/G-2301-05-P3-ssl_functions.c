#include <openssl/err.h>
#include <openssl/ssl.h>

struct Redes2_SSL_CTX {
	SSL_METHOD*  connection_method;
	SSL_CTX*     ctx;
};

struct SSL {
	SSL* ssl;
} Redes2_SSL;


void inicializar_nivel_SSL(void) {
	// Carga los errores para poder pintarlos
	SSL_load_error_strings();
	// Inicializa la libreria SSL y registra
	// los metodos de cifrado soportados
	SSL_library_init();
}

int fijar_contexto_SSL(Redes2_SSL* r2ssl) {
	// Devuelve el metodo de conexion
	r2ssl->connection_method = SSLv23_method();

	// Crea un contexto usando un metodo de conexion
	r2ssl->ctx = SSL_CTX_new(r2ssl->connection_method);
	if (NULL == r2ssl->ctx) {
		LOG("Error al crear un contexto SSL");
		ERR_print_errors_fp(stdout);
		return ERR;
	}

	// Añade nuestra CA
	if(1 != SSL_CTX_load_verify_locations(ctx, "ca_file.pem", "./cert")) {
		LOG("Error al comprobar la existencia de nuestro certificado");
		ERR_print_errors_fp(stdout);
		return ERR;
	}

	// Carpeta donde tiene que buscar las CA conocidas
	SSL_CTX_set_default_verify_paths(r2ssl->ctx);

	// Que certificado usara nuestra aplicacion
	if(1 != SSL_CTX_use_certificate_chain_file(r2ssl->ctx, "./cert/blah.pem")) {
		LOG("Error al agregar nuestro certificado");
		ERR_print_errors_fp(stdout);
		return ERR;
	}

	// Clave privada de nuestra aplciacion
	if(1 != SSL_CTX_use_PrivateKey_file(r2ssl->ctx, "path", SSL_FILETYPE_PEM)) {
		LOG("Error al agregar nuestra clave privada");
		ERR_print_errors_fp(stdout);
		return ERR;
	}


	SSL_CTX_set_verify(r2ssl->ctx, 0, 0);
	/*
	SSL_VERIFY_PEER
           Server mode: the server sends a client certificate request to the
           client.  The certificate returned (if any) is checked. If the
           verification process fails, the TLS/SSL handshake is immediately
           terminated with an alert message containing the reason for the
           verification failure.  The behaviour can be controlled by the
           additional SSL_VERIFY_FAIL_IF_NO_PEER_CERT and
           SSL_VERIFY_CLIENT_ONCE flags
	*/
}

int conectar_canal_seguro_SSL(Redes2_SSL* r2ssl, int sock_fd) {

}
int aceptar_canal_seguro_SSL(Redes2_SSL* r2ssl, int sock_fd) {

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
	SSL_shutdown(r2ssl->ssl);
	SSL_free(r2ssl->ssl);
}

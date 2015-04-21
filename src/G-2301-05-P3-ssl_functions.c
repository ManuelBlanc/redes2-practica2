#include <openssl/err.h>
#include <openssl/ssl.h>

void inicializar_nivel_SSL() {
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

int prueba(void) {


	Redes2_SSL* r2ssl = ecalloc(sizeof *r2ssl);


	// Carga los errores para poder pintarlos
	SSL_load_error_strings();
	// Inicializa la libreria SSL y registra
	// los metodos de cifrado soportados
	SSL_library_init();


	// Devuelve el metodo de conexion
	r2ssl->connection_method = SSLv23_method();


	// Crea un contexto usando un metodo de conexion
	r2ssl->ctx = SSL_CTX_new(r2ssl->connection_method);
	if (NULL == r2ssl->ctx) {
		LOG("Error al crear un contexto SSL");
		return NULL;
	}

	// Añade nuestra CA
	SSL_CTX_load_verify_locations(ctx, "ca_file.file", "./cert");

	// Carpeta donde tiene que buscar las CA conocidas
	SSL_CTX_set_default_verify_paths(r2ssl->ctx);

	// Que certificado usara nuestra aplicacion
	SSL_CTX_use_certificate_chain_file(r2ssl->ctx, "./cert/blah.pem");

	// Clave privada de nuestra aplciacion
	SSL_CTX_usePrivateKey_file(r2ssl->ctx, "path", SSL_FILETYPE_PEM);


	SSL_CTX_set_verify(r2ssl->ctx, 0, 0);
}
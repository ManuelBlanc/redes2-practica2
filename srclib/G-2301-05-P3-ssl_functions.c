#include <openssl/err.h>
#include <openssl/ssl.h>


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
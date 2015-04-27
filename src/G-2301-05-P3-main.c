
/* std */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

/* usr */
#include "G-2301-05-P3-server.h"
#include "G-2301-05-P3-ssl_functions.h"


static void usage(int code) {
	fprintf(stderr, "usage: %s [-hv]\n", "G-2301-05-P3-server");
	fprintf(stderr, "\t-%c,--%s, : %s\n", 'h', "help",                	"Muestra esta ayuda.");
	fprintf(stderr, "\t-%c,--%s, : %s\n", 'd', "demonio",             	"Arranca el programa como un demonio.");
	fprintf(stderr, "\t-%c,--%s=<puerto>, : %s\n", 'p', "port",       	"Especifica el puerto del servidor.");
	fprintf(stderr, "\t-%c,--%s=[puerto], : %s\n", 's', "secure-port",	"Especifica el puerto seguro.");
	exit(code);
}

// Struct anonimo con la configuracion
static struct {
	int      demonizar;
	int      usar_ssl;
	uint16_t puerto_normal;
	uint16_t puerto_seguro;
} configuracion;

static void procesar_opciones(int argc, char** argv) {
	static struct option longopts[] = {
		{ "port",       	required_argument,	NULL,	'p'	},
		{ "secure-port",	optional_argument,	NULL,	's'	},
		{ "help",       	no_argument,      	NULL,	'h'	},
		{ "demonio",    	no_argument,      	NULL,	'd'	},
		{NULL,0,NULL,0}
	};

	configuracion.demonizar     = 0;
	configuracion.usar_ssl      = 1;
	configuracion.puerto_normal = 6667;
	configuracion.puerto_seguro = 6697;

	while (1) {
		int opt = getopt_long(argc, argv, "dhp:s:", longopts, NULL);
		switch (opt) {
			/* Opciones */
			case 'h': usage(EXIT_SUCCESS);                                       	break;
			case 'd': configuracion.demonizar = 1;                               	break;
			case 'p': configuracion.puerto_normal = atoi(optarg);                	break;
			case 's': configuracion.puerto_seguro = optarg ? atoi(optarg) : 6697;	break;
			/* Especiales */
			case -1: return;   /* argument list exhausted */
			case 0:  continue; /* flag option set */
			/* Errores */
			default:
			case ':': /* missing option argument */
			case '?': /* unknown or ambiguous option */
				usage(EXIT_FAILURE);
		}
	}
}

static void demonizar(void) {
	// 1.– Crear un proceso hijo y terminar el proceso padre
	pid_t pid = fork();
	if (pid > 0) exit(EXIT_SUCCESS);

	// 2.– Crear una nueva sesion de tal forma que el proceso pase a ser el lider de sesion
	setsid();

	// 3.– Cambiar la mascara de modo de ficheros para que sean accesibles a cualquiera
	umask(0777);

	// 4.– Establecer el directorio raiz / como directorio de trabajo
	int dummy = chdir("/");
	UNUSED(dummy);

	// 5.– Cerrar todos los descriptores de fichero que pueda haber abiertos
	int i, fd_max = getdtablesize();
	for (i = 0; i < fd_max; ++i) close(i);

	// 6.– Redirigir stdin, stdout, stderr a /dev/null
	open("/dev/null", O_RDONLY); // 0 - stdin
	open("/dev/null", O_WRONLY); // 1 - stdout
	open("/dev/null", O_WRONLY); // 2 - stderr

	// 7.– Abrir el log del sistema para su uso posterior
	//openlog(const char *ident, int logopt, int facility);
	//setlogmask(int maskpri);
	//syslog(int priority, const char *message, ...);
	//closelog(void);
}

int main(int argc, char** argv)
{
	procesar_opciones(argc, argv);
	if (configuracion.demonizar) demonizar();

	inicializar_nivel_SSL();

	Server* serv = server_new();
	server_listen(serv, configuracion.puerto_seguro, 1);
	server_listen(serv, configuracion.puerto_normal, 0);
	pthread_exit(NULL);
	return 0;
}

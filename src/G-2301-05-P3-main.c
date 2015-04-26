
static void usage(int code) {
	fprintf(stderr, "usage: %s [-hv]\n", "G-2301-05-P3-server");
	fprintf(stderr, "\t%s : %s\n", "verbose",	"Pone el programa en un modo mas verboso.");
	fprintf(stderr, "\t%s : %s\n", "help",   	"Muestra esta ayuda.");
	fprintf(stderr, "\t%s : %s\n", "demonio",	"Arranca el programa como un demonio.");
	exit(code);
}

// Struct anonimo con la configuracion
static struct {
	int demonizar;
	uint16_t puerto_normal;
	uint16_t puerto_seguro;
} configuracion;

int demonio = 0;
static void procesar_opciones(int argc, char** argv) {
	static struct option longopts[] = {
		{ "verbose",	no_argument,	NULL,	'v'	},
		{ "help",   	no_argument,	NULL,	'h'	},
		{ "demonio",	no_argument,	NULL,	'd'	},
		{NULL,0,NULL,0}
	};
	while (1) {
		int opt = getopt_long(argc, argv, "vh", longopts, NULL);
		switch (opt) {
			/* Opciones */
			case 'v': /* verbosity++;        	*/ break;
			case 'h': /* usage(EXIT_SUCCESS);	*/ break;
			case 'd': demonio = 1; break;
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
	if (demonio) demonizar();

	inicializar_nivel_SSL();

	Server* serv = server_new();
	server_listen(serv, 6667, 1);
	server_listen(serv, 6697, 0);
	return 0;
}


/* std */
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
/* usr */
#include "G-2301-05-P1-chat.h"
#include "G-2301-05-P1-util.h"

#define BOLD(str)  	"\033[1m"  str "\033[0m"
#define RED(str)   	"\033[31m" str "\033[0m"
#define GREEN(str) 	"\033[32m" str "\033[0m"
#define YELLOW(str)	"\033[33m" str "\033[0m"

void _assert(int test, const char* test_str, const char* msg, const char* file, int line, const char* func) {
	if (test) return;
	fprintf(stderr, "===========================================================\n"	);
	fprintf(stderr, "  "RED("Ha fallado una asercion.")" Informacion adicional:\n" 	);
	fprintf(stderr, "  "BOLD("mensaje")" : %s\n", msg                              	);
	fprintf(stderr, "  "BOLD("prueba ")" : %s\n", test_str                         	);
	fprintf(stderr, "  "BOLD("errno  ")" : %s\n", strerror(errno)                  	);
	fprintf(stderr, "  "BOLD("lugar  ")" : %s() @ %s:%i\n", func, file, line       	);
	fprintf(stderr, " Por favor, pongase en contacto con el desarollador.\n"       	);
	fprintf(stderr, "===========================================================\n"	);
	exit(EXIT_FAILURE);
}

/** Sustituye todos los caracteres que representan espacio por ' ' (0x20)
 * @param str Cadena de caracteres
 */
static void normalize_whitespace(char* str) {
	while (*str) {
		if (isspace(*str)) *str = ' ';
		str++;
	}
}

static FILE* log_file;
void _log(const char* file, int line, const char* func, const char* fmt, ...)
{
	static char log_buffer[512];
	int len;
	va_list argptr;

	len = snprintf(log_buffer, sizeof log_buffer, GREEN("%s()")" @ "YELLOW("%s")":"RED("%i")": ", func, file, line);
	if (len < 0) return; // Ha ocurrido un error
	if (len > (int)(sizeof log_buffer)) len = sizeof log_buffer;

	va_start(argptr, fmt);
	vsnprintf(log_buffer + len, sizeof log_buffer - len, fmt, argptr);
	va_end(argptr);

	normalize_whitespace(log_buffer);

	// Imprimimos por consola
	fprintf(stderr, "%s\n", log_buffer);
	fflush(stderr);
	// Y a fichero
	if (log_file == NULL) log_file = fopen("chat.log", "w");
	fprintf(log_file, "%s\n", log_buffer);
	fflush(log_file);

	// Tambien por pantalla por comodidad
	//message_text(get_or_create_page("*DEBUG*"), log_buffer);
}


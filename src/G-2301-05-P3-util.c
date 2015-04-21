
#define _GNU_SOURCE /* vease: feature_test_macros(7) */

/* std */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
/* unistd */
#include <unistd.h>
#include <sys/syscall.h>
#include <syslog.h>
/* usr */
#include "G-2301-05-P2-util.h"

#define BOLD(str)  	"\033[1m"  str "\033[0m"
#define RED(str)   	"\033[31m" str "\033[0m"
#define GREEN(str) 	"\033[32m" str "\033[0m"
#define YELLOW(str)	"\033[33m" str "\033[0m"
#define PINK(str)  	"\033[1;35m" str "\033[0m"

void _assert(int test, char* test_str, char* msg, const char* file, int line, const char* func) {
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

//static FILE* log_file;
void _log(const char* file, int line, const char* func, char* fmt, ...) {
	static char log_buffer[512];
	int len;
	va_list argptr;

	len = snprintf(log_buffer, sizeof log_buffer, GREEN("%s()")"@"YELLOW("%s")":"RED("%i")": ["PINK("%ld")"]: ", func, file, line, syscall(SYS_gettid));
	if (len < 0) return; // Ha ocurrido un error
	if (len > (int)(sizeof log_buffer)) len = sizeof log_buffer;

	va_start(argptr, fmt);
	vsnprintf(log_buffer + len, sizeof log_buffer - len, fmt, argptr);
	va_end(argptr);

	normalize_whitespace(log_buffer);

	// Imprimimos por consola
	fprintf(stdout, "%s\n", log_buffer);
	fflush(stdout);

	// Y al syslog
	va_start(argptr, fmt);
	vsyslog(LOG_DAEMON, fmt, argptr);
	va_end(argptr);

}


void* emalloc(size_t size)
{
	void* ptr = malloc(size);
	ASSERT(ptr != NULL, "No se pudo reservar memoria con malloc()");
	return ptr;
}
void* ecalloc(size_t count, size_t size)
{
	void* ptr = calloc(count, size);
	ASSERT(ptr != NULL, "No se pudo reservar memoria con calloc()");
	return ptr;
}
char* estrdup(char* str) {
	str = strdup(str);
	ASSERT(str != NULL, "No se pudo reservar memoria con strdup()");
	return str;
}
char* estrndup(char* str, size_t size) {
	str = strndup(str,  size);
	ASSERT(str != NULL, "No se pudo reservar memoria con strndup()");
	return str;
}

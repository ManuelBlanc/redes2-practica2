#ifndef UTIL_H
#define UTIL_H

#define MY_MACRO(body)	do { body } while (0)

/* ========================================================
     DEFINES
   ======================================================== */
#define UNUSED(arg)	((void)(arg))
enum ErrCode {OK = 0, ERR = -1};

#define ATTRIBUTE(attr) __attribute__(attr)

/* ========================================================
     MALLOC
   ======================================================== */
#include <stddef.h>
void* emalloc(size_t size);
void* ecalloc(size_t count, size_t size);
char* estrdup(char* str);
char* estrndup(char* str, size_t size);

/* ========================================================
     LOGGER
   ======================================================== */
#define	LOG(...)	(_log(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__))
void _log(const char* file, int line, const char* func, char* fmt, ...)
ATTRIBUTE((format(printf, 4, 5)));

/* ========================================================
     ASSERT
   ======================================================== */
#define ASSERT(test, msg)  _assert((test), (#test), (msg), __FILE__, __LINE__, __FUNCTION__)
void _assert(int test, char* test_str, char* msg, const char* file, int line, const char* func);

#endif /* UTIL_H */

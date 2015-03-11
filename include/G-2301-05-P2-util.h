#ifndef UTIL_H
#define UTIL_H

#define MY_MACRO(body)	do { body } while (0)

/* ========================================================
     DEFINES
   ======================================================== */
#define UNUSED(arg)	((void)(arg))
enum ErrCode {OK = 0, ERR = -1};

/* ========================================================
     LOGGER
   ======================================================== */
#define	LOG(...)	(_log(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__))
void _log(const char* file, int line, const char* func, const char* fmt, ...);

/* ========================================================
     ASSERT
   ======================================================== */
#define ASSERT(test, msg)  _assert((test), (#test), (msg), __FILE__, __LINE__, __FUNCTION__)
void _assert(int test, const char* test_str, const char* msg, const char* file, int line, const char* func);

#endif /* UTIL_H */

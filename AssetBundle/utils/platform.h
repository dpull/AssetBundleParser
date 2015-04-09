#ifndef PLATFORM_H
#define PLATFORM_H
#ifdef _MSC_VER
#ifndef getcwd
#define getcwd      _getcwd
#endif
#define snprintf    _snprintf
#define snwprintf   _snwprintf
#define tzset       _tzset
#define open        _open
#define close       _close
#define chsize      _chsize
#define fileno      _fileno
#define stricmp     _stricmp
#ifndef strdup
#define strdup      _strdup
#endif

char *strndup(const char *s, size_t n);

#ifndef wcsdup
#define wcsdup      _wcsdup
#endif
#define getch       _getch
#define strtoll     _strtoi64
#define strtoull    _strtoui64
#define strtok_r    strtok_s
#define wfopen      _wfopen
#define getpid      _getpid
#define fseek64     _fseeki64
#define ftell64     _ftelli64

#include <io.h>
#define mktemp		_mktemp
#endif	  

#if defined(API_EXTERN) || defined(API_CALLBACK)
	#error API_EXTERN or API_CALLBACK already defined!
#endif

#if defined(_MSC_VER)
#define API_EXTERN __declspec(dllexport)
#define API_CALLBACK __stdcall
#else
#define API_EXTERN  
#define API_CALLBACK
#endif

#endif
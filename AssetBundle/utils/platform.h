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
#endif

#if defined(_MSC_VER)
#define EXTERN_API __declspec(dllexport)
#else
#define EXTERN_API 
#endif

#endif
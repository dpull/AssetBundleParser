#ifndef TRAVERSEDIR_H
#define TRAVERSEDIR_H

typedef bool (traversedir_callback)(const char* fullpath, const char* filename, void* userdata);
void traversedir(const char dir[], traversedir_callback* callback, void* userdata, bool ignore_hidefile);
#endif

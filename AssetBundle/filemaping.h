#ifndef filemaping_h
#define filemaping_h

struct filemaping* filemaping_create_readonly(const char* file);
struct filemaping* filemaping_create_readwrite(const char* file, size_t length);
void filemaping_destory(struct filemaping* filemaping);

unsigned char* filemaping_getdata(struct filemaping* filemaping);
size_t filemaping_getlength(struct filemaping* filemaping);

#endif


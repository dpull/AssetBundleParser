#ifndef filemapping_H
#define filemapping_H

struct filemapping* filemapping_create_readonly(const char* file);
struct filemapping* filemapping_create_readwrite(const char* file, size_t length);
void filemapping_destory(struct filemapping* filemapping);

unsigned char* filemapping_getdata(struct filemapping* filemapping);
size_t filemapping_getlength(struct filemapping* filemapping);
unsigned char* filemapping_getdata(struct filemapping* filemapping);

bool filemapping_truncate(const char* file, size_t length);
#endif


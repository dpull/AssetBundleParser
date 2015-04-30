#ifndef filemapping_H
#define filemapping_H

struct filemapping* filemapping_create_readonly(const char* file);
struct filemapping* filemapping_create_readwrite(const char* file, size_t length);
void filemapping_destroy(struct filemapping* filemapping);

unsigned char* filemapping_getdata(struct filemapping* filemapping);
size_t filemapping_getlength(struct filemapping* filemapping);
unsigned char* filemapping_getdata(struct filemapping* filemapping);

API_EXTERN bool filemapping_truncate(const char* file, size_t length);
#endif

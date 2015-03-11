
#ifndef OBJECT_CLASS_H
#define OBJECT_CLASS_H

char* objectinfo_getname(unsigned char* data, size_t start, size_t size); // need free
struct object_class_audioclip* object_class_audioclip_load(unsigned char* data, size_t start, size_t size);
void object_class_audioclip_destory(struct object_class_audioclip* object_class);

#endif
